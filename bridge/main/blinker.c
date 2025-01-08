#include "blinker.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_heap_caps.h" // 

#define TAG "BLINKER"

led_strip_handle_t led_strip;
QueueHandle_t blinkQueue;


const struct BlinkSequence blinkSequences[] = {
    // Note, the last 3 items are r,g,b
    {BLINK_NONE, 0, 0, 0, 0, 0, 0, 0}, // SOLID GREEN
    {BLINK_OK, 1, 1000, 0, 1000, 0, 155, 0}, // SOLID GREEN
    {BLINK_BOOTING, 3, 200, 200, 3000, 0, 255, 0}, // Green, 3 blinks
    {BLINK_WIFI_CONNECTING, 1, 500, 500, 5000, 255, 255, 0}, // Yellow, 1 blink
    {BLINK_WIFI_CONNECTED, 1, 1000, 0, 500, 0, 55, 0}, // DIM Solid Green
    {BLINK_WIFI_FAILED, 5, 100, 100, 3000, 255, 0, 0}, // Red, 5 quick blinks
    {BLINK_UDP_ERROR, 2, 300, 300, 3000, 0, 0, 255}, // Blue, 2 blinks
    {BLINK_USB_ERROR, 2, 300, 300, 3000, 155, 0, 0}, // RED, 2 blinks}
    {BLINK_ESPNOW_ERROR, 1, 1000, 0, 1000, 0, 0, 255}, // Solid Blue
    {BLINK_SERVER_FAILED, 3, 250, 250, 3000, 255, 0, 0},   // Red, 3 blinks
    {BLINK_ERROR_GENERIC, 4, 150, 150, 3000, 255, 165, 0}, // Orange, 4 quick blinks
    {BLINK_ERROR_SENSOR, 6, 100, 100, 3000, 255, 0, 255},  // Magenta, 6 quick blinks
    {BLINK_PEER_ERROR,6, 100, 100, 3000, 0, 0, 255},  // blue, 6 quick blinks
};

void configure_led(void){
    ESP_LOGI(TAG, "SETTING UP LED!");
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
    //led_strip_clear(led_strip);
}



void triggerBlink(BlinkCode code) {
    xQueueSend(blinkQueue, &code, 0);
}



// Task to reset the led from err msgs
void blink_task_old(void *arg) {
    while (1) {

        vTaskDelay(pdMS_TO_TICKS(1000));

        // Turn off the LED
        led_strip_set_pixel(led_strip, 0, 0,10,0 );
        led_strip_refresh(led_strip);

    }
}


void blink_task(void *pvParameters) {
    BlinkCode currentCode = BLINK_OK;
    BlinkCode lastCode = BLINK_NONE;
    TickType_t blinkStartTime = 0;
    uint8_t blinkCount = 0;

    while (1) {
        BlinkCode receivedCode;
        if (xQueueReceive(blinkQueue, &receivedCode, 0) == pdTRUE) {
            // nothing changed 
            /*
            if(currentCode == receivedCode){
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }
            */

            // reset for new code
            currentCode = receivedCode;
            blinkStartTime = xTaskGetTickCount();
            blinkCount = 0;
            
        }

        if (currentCode == BLINK_NONE){
            led_strip_clear(led_strip);
            // I think this causes a stack overflow
            //led_strip_refresh(led_strip);
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        else if (currentCode == BLINK_OK){
            led_strip_set_pixel(led_strip, 0, 0, 100,0);
            led_strip_refresh(led_strip);
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (currentCode != BLINK_NONE && currentCode != BLINK_OK) {
            for (size_t i = 0; i < sizeof(blinkSequences) / sizeof(blinkSequences[0]); i++) {
                const struct BlinkSequence *seq = &blinkSequences[i];
                if (seq->code == currentCode) {
                    if (xTaskGetTickCount() - blinkStartTime < pdMS_TO_TICKS(seq->displayDurationMs)) {
                        if (blinkCount < seq->numBlinks * 2) {
                            if (blinkCount % 2 == 0) {
                                led_strip_set_pixel(led_strip, 0, seq->r, seq->g, seq->b);
                                led_strip_refresh(led_strip);
                                vTaskDelay(pdMS_TO_TICKS(seq->onDurationMs));
                            } else {
                                led_strip_clear(led_strip);
                                vTaskDelay(pdMS_TO_TICKS(seq->offDurationMs));
                            }
                            blinkCount++;
                        } else {
                            currentCode = BLINK_OK; // Reset after complete sequence
                            break; // Exit the for loop
                        }
                    } else {
                        currentCode = BLINK_OK;
                        break; // Exit the for loop
                    }
                }
            }
        } 
        else {
            if(currentCode != lastCode){
                led_strip_set_pixel(led_strip, 0, 0, 100,0);
                led_strip_refresh(led_strip);
                
            }
            
        }
        lastCode = currentCode;
        vTaskDelay(pdMS_TO_TICKS(10));
        } // end while(1)
}