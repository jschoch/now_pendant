#pragma once
#include "led_strip.h"
#include "esp_now.h"

#include "freertos/FreeRTOS.h"
void example_wifi_init(void);
void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
void print_espnow_peer_addr(const esp_now_peer_info_t* addr);
esp_err_t example_espnow_init(led_strip_handle_t led_strip_init);



typedef struct ESP_NOW_test_data
{
   uint8_t id;
   uint16_t test_value;
} ESP_NOW_test_data_t;

typedef struct {
  esp_now_peer_info_t peer;
  // Add other relevant peer information here (e.g., name, status)
} peer_info_t;