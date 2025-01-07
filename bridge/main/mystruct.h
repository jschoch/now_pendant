#ifndef _MYSTRUCT_H
#define _MYSTRUCT_H 

typedef struct espnow_message_mpg {
  char a[32];
  int b;
  float c;
  bool d;
  int mpg1;
} espnow_message_mpg;

typedef struct espnow_message_ping{
  int cmd;
} espnow_message_ping;

extern espnow_message_mpg mpgData;
extern volatile bool update_ready;
extern QueueHandle_t blinkQueue;


//uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
#endif
