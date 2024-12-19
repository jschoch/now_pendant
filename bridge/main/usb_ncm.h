/**
 * @file usb_ncm.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

/**
 * 
 *   The linuxcnc network should have an IP of 192.168.11.3, the UDP server should be on the same interface?
 */
#define STATIC_IP_ADDR "192.168.11.4"
#define SERVER_IP_ADDR "192.168.11.3"
#define SERVER_PORT 8080
#define DEFAULT_GATEWAY "192.168.11.3"
#define DEFAULT_NETMASK "255.255.255.0"

/**
 * @brief Initialize USB NCM and start interface
 *
 * @return esp_err_t
 */
esp_err_t usb_ncm_init(void);
