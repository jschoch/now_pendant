/* USB Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mystruct.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "sdkconfig.h"
#include "esp_mac.h"

#include "tinyusb.h"
#include "udp_server.h"
#include "nvs_flash.h"
#include "enow.h"
#include "usb_ncm.h"
#include "led_strip.h"


// for remote: MAC: { 0x84, 0xf7, 0x3, 0xc0, 0x24, 0x38 } 


volatile bool update_ready = 0;

static const char* TAG = "USB example";

#define BLINK_GPIO 48
static led_strip_handle_t led_strip;

static void configure_led(void){
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}


/*------------------------------------------*
 *               USB Callback
 *------------------------------------------*/
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB mounted.");
}




/*------------------------------------------*
 *                 APP MAIN
 *------------------------------------------*/
void app_main(void)
{
    ESP_LOGI(TAG, "------------ app_main -----------");

     
    // do usb ncm stuff

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    uint8_t mac[6] = {0};
    esp_efuse_mac_get_default(mac);

    printf("MAC: { 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x } \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    configure_led();

    led_strip_set_pixel(led_strip, 0, 16, 0, 5);
    led_strip_refresh(led_strip);

    const tinyusb_config_t tusb_cfg = {
        .external_phy = false,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));


    usb_ncm_init();
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)1234, 4, NULL);

    // Setup espnow
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    ESP_LOGI(TAG, "first setup done");

    example_wifi_init();

    led_strip_set_pixel(led_strip, 0, 0, 16, 5);
    led_strip_refresh(led_strip);
    ESP_LOGI(TAG, "wifi init done");

    example_espnow_init();
    led_strip_set_pixel(led_strip, 0, 0, 0, 16);
    led_strip_refresh(led_strip);
    ESP_LOGI(TAG, "espnow init done");
}
