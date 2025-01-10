#pragma once
#include <ESP32Encoder.h>
#include <freertos/FreeRTOS.h>
#include <rom/gpio.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_ADS1X15.h>

#include <MsgPack.h>


enum class InputType{
  Encoder,
  Button,
  Pot
};


struct EncInputData{
  InputType type;
  int id;
  int64_t v;
  int64_t prev_v; 
  enum class ValueType { FLOAT, INT32, INT64, INT8, BOOL };
  ValueType vtype;
  ESP32Encoder *encoder;
};

struct BtnInputData{
 InputType type;
 int id; // button ID
 bool state; // current state of the button
 bool prev_state; // previous state of the button
 bool pending; // flag for sending updates
 int pin; // pin number of the button

};

struct PotInputData{
    InputType type;
    int id;
    int map_value;
    float volts;
    int prev_state;
    int map_size;
};

typedef struct espnow_add_peer_msg {
  byte mac_adddress[6];
} espnow_add_peer_msg;

// Structure example to send data
// Must match the receiver structure
typedef struct espnow_message {
  char a[32];
  int b;
  float c;
  bool d;
} espnow_message;

typedef struct espnow_message_mpg {
  char a[32];
  int b;
  float c;
  bool d;
  int mpg1;
} espnow_message_mpg;


// Feedback packet 
struct fbPacket {
    uint8_t control;
    uint8_t io;
    int32_t pos[6];
    int32_t vel[6];
    uint32_t udp_seq_num;
};

// Structs for processing lcnc msgs


// should fit in an uint8_t
enum class MsgType : uint8_t{
    PING = 7,
    STATE = 5,
    ERRORS = 2,
    POS = 3,
};

struct Ping {
    uint32_t sequence_number;
    MSGPACK_DEFINE(sequence_number);// [sequence_number]
};

struct State {
    int system;
    int machine;
    int motion;
    int homed;
    MSGPACK_DEFINE(system,machine,motion,homed); // [system,machine,motion]
};

struct Amsg{
    //int msg_type;
    uint8_t msg_type;
    State state;
    MSGPACK_DEFINE(msg_type,state);
};



struct Errors{
    int errornumber;
    String msg;
    MSGPACK_DEFINE(errornumber,msg);
};

struct Pos{
    float x;
    float z;
    float x_dtg;
    float z_dtg;
    float jog_scale;
    int g5x;
    int x_jog_counts;
    int z_jog_counts;
    MSGPACK_DEFINE(x,z,x_dtg,z_dtg,jog_scale,g5x,x_jog_counts,z_jog_counts);
};

struct Pmsg{
    //int msg_type;
    uint8_t msg_type;
    Pos pos;
    MSGPACK_DEFINE(msg_type,pos);
};



 
extern const int NUM_BUTTONS;
extern const int NUM_ENC;
extern const int NUM_POTS;
extern struct BtnInputData buttons[];
extern struct EncInputData encoders[];
extern struct PotInputData pots[];
extern esp_now_peer_info_t peerInfo; 
extern MsgPack::Packer packer;
extern uint8_t remotePeerAddress[];
extern espnow_message_mpg mpgData;
extern espnow_message myData;
extern espnow_add_peer_msg espnowAddPeerMsg;
extern esp_now_peer_info_t peerInfo;
extern fbPacket fb;
extern bool espnow_peer_configured;
extern bool connected;
extern class Adafruit_ADS1115 ads; 
extern unsigned long lastHello;
extern class State lastState;
extern class Pos lastPos;

void setupButtons();
void setupEnc(ESP32Encoder *encoder,int a, int b);
void sendUpdate();
void sendBtnUpdate(int btn_id);
void sendEncUpdate(EncInputData data);
void doReadButtons();
void doReadEncoders();
void doReadFast();
void doReadSlow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) ;
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) ;
void setupPeer();
void sendHello();
void doReadPots(bool forceSend);
void sendPotUpdate();
