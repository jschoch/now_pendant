#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "mystruct.h"
#include <lwip/sockets.h>
#include "led_strip.h"

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

static const char *TAG = "espnow_example";

typedef struct ESP_NOW_test_data
{
   uint8_t id;
   uint16_t test_value;
} ESP_NOW_test_data_t;

ESP_NOW_test_data_t ESP_NOW_data;




espnow_message_mpg mpgData;

/* WiFi should start before using ESPNOW */
void example_wifi_init(void)
{

    // NOTE:  this netif init and loop creation is already done in the NCM init, so doing it here causes a boot loop
    //ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

    // enable esp now long range
    //ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );

}

// TODO: move this somewhere sensible 
//const char* target_ip = "192.168.1.210";  // Replace with target IP address
const char* target_ip = "192.168.1.160";
const int target_port = 8080;  

void sendShit(const uint8_t *data,int len){
    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
      ESP_LOGE(__FILE__, "Failed to create socket: %d", errno);
    }

    // Set up destination address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(target_ip);
    server_addr.sin_port = htons(target_port);

    // Prepare message to send
    //const char* message = "Hello from ESP32!";

    // Send the message
    int sent = sendto(sock, data, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
      ESP_LOGE(__FILE__, "Failed to send UDP message: %d", errno);
    } else {
      ESP_LOGI(__FILE__, "UDP message sent: ");
    }

    // Close the socket
    close(sock);
}

//void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    memcpy(&ESP_NOW_data, data, sizeof(ESP_NOW_data));
    //ESP_LOGI(TAG, "\nESP id: %d\nData recieved: %d\n\n", ESP_NOW_data.id, ESP_NOW_data.test_value);
    //int mlen = sizeof(mpgData);
    //memcpy(&mpgData, data, mlen);
    //ESP_LOGI(TAG, "\nESP id: %s\nData recieved: %d len: %d mlen %d\n\n", mpgData.a, mpgData.mpg1,len,mlen);
    ESP_LOGI(TAG,"Got data %d",len);
    update_ready = 1;
    sendShit(data,len);



}

esp_err_t example_espnow_init(void)
{
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );
    return ESP_OK;
}
