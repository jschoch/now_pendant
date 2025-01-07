#ifndef BLINKER_H
#define BLINKER_H

#include "led_strip.h"
#include <stdint.h> 

#define BLINK_GPIO 48

void configure_led(void);
void blink_task(void *arg);



typedef enum { // Use typedef to create a type alias
    BLINK_NONE = 0,
    BLINK_OK,
    BLINK_BOOTING,
    BLINK_WIFI_CONNECTING,
    BLINK_WIFI_CONNECTED,
    BLINK_WIFI_FAILED,
    BLINK_UDP_ERROR,
    BLINK_ESPNOW_ERROR,
    BLINK_SERVER_FAILED,
    BLINK_ERROR_GENERIC,
    BLINK_ERROR_SENSOR,
    BLINK_PEER_ERROR,
    BLINK_USB_ERROR,
    // ... more blink codes
} BlinkCode; // The type alias is now BlinkCode

void triggerBlink(BlinkCode code);

struct BlinkSequence {
  BlinkCode code;
  uint8_t numBlinks;      // Number of blinks
  uint16_t onDurationMs;   // Duration LED is ON (milliseconds)
  uint16_t offDurationMs;  // Duration LED is OFF (milliseconds)
  uint32_t displayDurationMs; // total time to show the blink code.
  uint8_t r, g, b;         // RGB color for the blink
};





#endif