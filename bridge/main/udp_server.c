#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/sockets.h>
#include "mystruct.h"
#include "usb_ncm.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "udp_server.h"
#include "blinker.h"

#define UDP_SERVER_PORT 8080                 // Replace with your server port
#define MESSAGE "Hello from ESP32 S3!"

//uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

static const char* TAG = "ESP32_SOCKET";

char rx_buffer[250];

unsigned long ip_to_ulong(const char *ip_str) {
    unsigned long ip = inet_addr(ip_str);
    if (ip == INADDR_NONE) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        //exit(EXIT_FAILURE);
    }
    return ip;
}

void print_sockaddr_in(const char* msg,const struct sockaddr_in* addr) {
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr->sin_addr), ip_str, sizeof(ip_str));

  ESP_LOGI(TAG, "%s SOCKADDR_IN %s %d ",msg, ip_str, ntohs(addr->sin_port));
}

void handle_espnow_send_error(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            ESP_LOGI(TAG, "Operation succeeded");
            break;
        case ESP_ERR_ESPNOW_NOT_INIT:
            ESP_LOGE(TAG, "ESPNOW is not initialized");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_INVALID_ARG:
            ESP_LOGE(TAG, "Invalid argument provided");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_ESPNOW_INTERNAL:
            ESP_LOGE(TAG, "Internal error in ESPNOW");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(TAG, "Out of memory. Consider delaying a while before sending the next data.");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(TAG, "Peer is not found");

            break;
        case ESP_ERR_ESPNOW_IF:
            ESP_LOGE(TAG, "Current Wi-Fi interface doesn't match that of peer");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_ESPNOW_CHAN:
            ESP_LOGE(TAG, "Current Wi-Fi channel doesn't match that of peer");
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;
        case ESP_ERR_ESPNOW_NOT_FOUND:
           ESP_LOGE(TAG, "Peer is not found");
           triggerBlink(BLINK_PEER_ERROR);
            break;
        default:
            ESP_LOGE(TAG, "Unknown error: %u", err);
            triggerBlink(BLINK_ESPNOW_ERROR);
            break;


    }
}

void udp_server_task(void *pvParameters)
{
    int sockfd;
    struct sockaddr_in server_addr, remote_addr;
    socklen_t addr_len = sizeof(remote_addr);

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_SERVER_PORT);

    // Bind the socket to any available port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket");
        close(sockfd);
        vTaskDelete(NULL);
        return;
    }

    while (1) {


        /*  no idea what is actually does
        // We got this msg from esp_now, update_ready flag set in enow.c recieve callback
        if(update_ready == 1){
            ESP_LOGI(TAG,"found update\t");
            update_ready = 0;
            // Send the string data
            char data_string[128];
            //  what is this?
            int bytes_written = snprintf(data_string, sizeof(data_string),
                               "update"
                               );
            print_sockaddr_in("update Remote ",&remote_addr);
            ESP_LOGI(TAG,"data sent: %s",data_string);
            int sent_bytes = sendto(sockfd,
                         data_string, 
                         bytes_written, 
                         0, 
                         (struct sockaddr*)&remote_addr, 
                         addr_len);
        }
        */

        // Receive data (waits for a packet)
        int recv_bytes = recvfrom(sockfd, rx_buffer, sizeof(rx_buffer)-1, MSG_WAITALL, 
                                 (struct sockaddr*)&remote_addr, &addr_len);

        if (recv_bytes < 0) {
            ESP_LOGE(TAG, "Failed to receive data");
            // go to next loop iteration
            continue;
        }


        // if the packet > 0 bytes

        // convert for comparison
        const unsigned long server_ip = ip_to_ulong(SERVER_IP_ADDR);

        // only forward to esp_now if sent from  the server
        if(remote_addr.sin_addr.s_addr == server_ip){
            ESP_LOGI(TAG,"Received from server\t");
            ESP_LOGI(TAG, "Received %d bytes \n", recv_bytes);

            // esp_now_send limited to 250 bytes
            for (int i = 0; i < num_peers; i++) {
                ESP_LOGI(TAG, "trying to send vi ESP_NOW Peer: %i",i);
                print_espnow_peer_addr(&peers[i].peer);
                esp_err_t res = esp_now_send((uint8_t *)&peers[i].peer, (uint8_t *)&rx_buffer, recv_bytes);
                handle_espnow_send_error(res);
            }
            // TODO: should we send an ack or something?

        }

        // Send a debug msg, mostly just to check if it is running
        else if (remote_addr.sin_addr.s_addr != INADDR_ANY) {

            char data_string[128];
            // Convert each member to string individually using appropriate formatting:
            int bytes_written = snprintf(data_string, sizeof(data_string),
                            "MSG for you!  %s, %d, %.2f, mpg1: %d",
                            mpgData.a, mpgData.b, mpgData.c, mpgData.mpg1);
            // Send the string data to UDP
            ESP_LOGI(TAG,"Sending some debug data: %s",data_string);
            int sent_bytes = sendto(sockfd,
                        data_string, 
                        bytes_written, 
                        0, 
                        (struct sockaddr*)&remote_addr, 
                        addr_len);

            if (sent_bytes < 0) {
                ESP_LOGE(TAG, "Failed to send data");
                continue;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    close(sockfd);
    vTaskDelete(NULL);
}

