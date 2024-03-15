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

#define UDP_SERVER_PORT 8080                 // Replace with your server port
#define MESSAGE "Hello from ESP32 S3!"

static const char* TAG = "ESP32_SOCKET";

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
        if(update_ready == 1){
            ESP_LOGE(TAG,"found update\t");
            update_ready = 0;
            // Send the string data
            char data_string[128];
              // Convert each member to string individually using appropriate formatting:
            int bytes_written = snprintf(data_string, sizeof(data_string),
                               "update"
                               );
            ESP_LOGE(TAG,"Sending data: %s",data_string);
            int sent_bytes = sendto(sockfd,
                         data_string, 
                         bytes_written, 
                         0, 
                         (struct sockaddr*)&remote_addr, 
                         addr_len);


        }
        // Receive data (waits for a packet)
        int recv_bytes = recvfrom(sockfd, NULL, 0, MSG_WAITALL, 
                                 (struct sockaddr*)&remote_addr, &addr_len);

        if (recv_bytes < 0) {
            ESP_LOGE(TAG, "Failed to receive data");
            continue;
        }

        // Send data if a connection is established (remote address is valid)
        if (remote_addr.sin_addr.s_addr != INADDR_ANY) {
            //int sent_bytes = sendto(sockfd, MESSAGE, strlen(MESSAGE), 0,
                                   //(struct sockaddr*)&remote_addr, addr_len);
            //espnow_message_mpg *data_to_send = (espnow_message_mpg *)malloc(sizeof(espnow_message_mpg));
            /*int sent_bytes = sendto(sockfd, 
                data_to_send, 
                sizeof(espnow_message_mpg), 
                0,
                (struct sockaddr*)&remote_addr, 
                addr_len);
                */
            char data_string[128];
              // Convert each member to string individually using appropriate formatting:
            int bytes_written = snprintf(data_string, sizeof(data_string),
                               "MSG for you!  %s, %d, %.2f, mpg1: %d",
                               mpgData.a, mpgData.b, mpgData.c, mpgData.mpg1);
            // Send the string data
            ESP_LOGE(TAG,"Sending data: %s",data_string);
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

