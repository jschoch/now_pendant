#ifndef _MYSTRUCT_H
#define _MYSTRUCT_H 

typedef struct espnow_message_mpg {
  char a[32];
  int b;
  float c;
  bool d;
  int mpg1;
} espnow_message_mpg;

extern espnow_message_mpg mpgData;
extern volatile bool update_ready;

#endif
