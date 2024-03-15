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



// for remote: MAC: { 0x84, 0xf7, 0x3, 0xc0, 0x24, 0x38 } 


volatile bool update_ready = 0;

static const char* TAG = "USB example";

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

    const tinyusb_config_t tusb_cfg = {
        .external_phy = false,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));


#if CONFIG_TINYUSB_NET_MODE_NCM
    usb_ncm_init();
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)1234, 4, NULL);
#endif

    // Setup espnow
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    example_wifi_init();
    example_espnow_init();
}
