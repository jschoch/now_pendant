#pragma once
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
void example_wifi_init(void);
void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len);
esp_err_t example_espnow_init(led_strip_handle_t led_strip_init);
