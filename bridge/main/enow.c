#include "usb_ncm.h"
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
#include "enow.h"
#include "blinker.h"

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#define MAX_PEERS 15

static const char *TAG = "BRIDGE";

void print_espnow_peer_addr(const esp_now_peer_info_t* addr) {
  ESP_LOGI(TAG, "Peer Address:");
  char mac_str[18] = {0}; // Buffer to hold MAC address in string format
  sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", 
          addr->peer_addr[0], addr->peer_addr[1], addr->peer_addr[2], 
          addr->peer_addr[3], addr->peer_addr[4], addr->peer_addr[5]);
  ESP_LOGI(TAG, "  - MAC Address: %s", mac_str);
}

ESP_NOW_test_data_t ESP_NOW_data;


peer_info_t peers[MAX_PEERS]; // Define MAX_PEERS based on your needs
int num_peers = 0;

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
    ESP_ERROR_CHECK( esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE));

    // enable esp now long range
    //ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );

}


void sendShit(const uint8_t *data,int len){
    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // set timeout
    struct timeval timeout;
    timeout.tv_sec = 0;  // 1-second timeout
    timeout.tv_usec = 5000;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    if (sock < 0) {
      ESP_LOGE(__FILE__, "Failed to create socket: %d", errno);
      triggerBlink(BLINK_SERVER_FAILED);
    }

    // Set up destination address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server_addr.sin_port = htons(SERVER_PORT);

    // Prepare message to send
    //const char* message = "Hello from ESP32!";

    // Send the message
    int sent = sendto(sock, data, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
      ESP_LOGE(__FILE__, "Failed to send UDP message: %d", errno);
      triggerBlink(BLINK_UDP_ERROR);

    } else {
      ESP_LOGI(__FILE__, "UDP message sent: ");
    }

    // Close the socket
    close(sock);
}

//void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{

  // register the peer if we've not seen it
  if (esp_now_is_peer_exist(recv_info->src_addr) == false) {
    if (num_peers < MAX_PEERS) {
      esp_now_peer_info_t peer;
      peer.channel = 11;
      peer.ifidx = ESPNOW_WIFI_IF;
      peer.encrypt = false;
      memcpy(peer.peer_addr, recv_info->src_addr, ESP_NOW_ETH_ALEN);
      esp_err_t e = esp_now_add_peer(&peer);
      ESP_ERROR_CHECK( e );
      ESP_LOGI(TAG, "New peer added: ");
      // Print the MAC address of the new peer
      for (int i = 0; i < ESP_NOW_ETH_ALEN; i++) {
        ESP_LOGI(TAG,"%02x:", recv_info->src_addr[i]);
      }
      peer_info_t pi;
      pi.peer = peer;
      peers[num_peers] = pi;
      num_peers++;

      // Add logic to handle new peer addition (e.g., send a welcome message)
    } else {
      // Handle case where peer list is full
        // TODO display this error or something
        ESP_LOGE(TAG, "Peer list is full. Cannot add new peer.");
        triggerBlink(BLINK_PEER_ERROR);
    }
  }
  memcpy(&ESP_NOW_data, data, sizeof(ESP_NOW_data));
  ESP_LOGI(TAG,"Got data %d",len);
  update_ready = 1;
  sendShit(data,len);



}

esp_err_t example_espnow_init()
{
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );
    return ESP_OK;
}
