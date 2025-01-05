#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <rom/gpio.h>
#include <ESP32Encoder.h>
#include <Ticker.h >

#include <esp_now.h>
#include <WiFi.h>
#include <User_Setup_Select.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();


#include <MsgPack.h>
MsgPack::Packer packer;

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

/* these pins clobber the t-display pins


int enc2a = 15;
int enc2b = 16;

int enc3a = 17; // b
int enc3b = 18;

int enc4a = 19; // b
int enc4b = 21;
*/

int enc1a = 12;
int enc1b = 13;


// Don't use 36,39 there are no pullups
int btn1 = 32; // 
int btn2 = 33;  // 

int16_t adc0, adc1, adc2, adc3;

ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
ESP32Encoder encoder4;

Ticker readEncoders;
Ticker readBtns;
Ticker slowReadEncoders;

enum class InputType{
  Encoder,
  Button
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



//             initilize the structs, why doesn't teh commented code work  gah!@
EncInputData e1 = {
  InputType::Encoder,
  1, // encoder ID
  0, // Encoder count
  0, // the previous count
  EncInputData::ValueType::INT64, 
  &encoder1
};
EncInputData e2 = {
  InputType::Encoder,
  2,
  0,
  0,
  EncInputData::ValueType::INT64,
  &encoder2
}; 
EncInputData e3 = {
  InputType::Encoder,
  3,
  0,
  0,
  EncInputData::ValueType::INT64,
  &encoder3
};
EncInputData e4 = {
  InputType::Encoder,
  4,
  0,
  0,
  EncInputData::ValueType::INT64,
  &encoder4
};

//  THe buttons
#define NUM_BUTTONS 2

BtnInputData b1 = {
  InputType::Button,
  0, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn1
};
BtnInputData b2 = {
  InputType::Button,
  1, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn2
};

BtnInputData buttons[] = {b1,b2};



#define NUM_ENC 1
//EncInputData encoders[] = {e1,e2,e3,e4};
EncInputData encoders[] = {e1};


void setupButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    Serial.printf("setting pinmode for pin: %d ",buttons[i].pin);
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}





// (84:f7:03:c0:24:38)
uint8_t remotePeerAddress[] = {0x84, 0xf7, 0x03, 0xc0, 0x24, 0x38}; // TODO: Implement broadcast with security
//uint8_t remotePeerAddress[] = {0x24, 0xD7, 0xEB, 0xB7, 0xA7, 0x2C}; // TODO: Implement broadcast with security

void setupEnc(ESP32Encoder *encoder,int a, int b){
  //encoder->attachHalfQuad(a, b);
  encoder->attachFullQuad(a,b);
  //encoder->attachSingleEdge(a,b);
  encoder->setCount ( 0 );
  encoder->setFilter(1023);
}

void sendUpdate(){
  esp_err_t result = esp_now_send(remotePeerAddress, packer.data(), packer.size()); // send esp-now addPeerMsg
  packer.clear();

  if (result == ESP_OK) {
    Serial.print(".");
  }else{
    Serial.println(" couldn't send msg");
    Serial.println(result);
  }

}

void sendBtnUpdate(int btn_id){
  //packer.serialize(MsgPack::map_size_t(2), "h",false, "b" , btnData); 
  // Send 4 element array with "b" signaling a button msg
  packer.serialize(MsgPack::arr_size_t(4),false,"b",btn_id,buttons[btn_id].state);
  sendUpdate();
  // reset state
  buttons[btn_id].pending= false;
}


void sendEncUpdate(EncInputData data){
  // Send 4 element array with "e" signaling an encoder msg
  packer.serialize(MsgPack::arr_size_t(5),false,"e",data.id,data.prev_v,data.v);
  sendUpdate();  
  
}




void doReadButtons(){
  for(int i = 0;i < NUM_BUTTONS;i++){
    buttons[i].state = digitalRead(buttons[i].pin);

    // detect state changes
    if(buttons[i].state != buttons[i].prev_state && !buttons[i].pending){
      Serial.printf("Button %d current state: %d prev_state %d\n",i,buttons[i].state,buttons[i].prev_state);
      buttons[i].pending = true;      
      sendBtnUpdate(i);
      buttons[i].prev_state = buttons[i].state;      

    }
    //buttons[i].prev_state = buttons[i].state;    
    
  }

}


void doReadEncoders(bool print){

  for(int i = 0;i < NUM_ENC;i++){
    encoders[i].v = encoders[i].encoder->getCount();

    if(encoders[i].prev_v != encoders[i].v){
      sendEncUpdate(encoders[i]);
      // set previous value 
      encoders[i].prev_v = encoders[i].v;
    }

    
  }

 /*
   if(print){
    Serial.printf("1: %i 2: %i 3: %i 4: %i ",(int)e1.v,(int)e2.v,(int)e3.v,(int)e4.v);
    //Serial.printf("1: %" PRId64 " 2: %" PRId64 " 3: %" PRId64 " 4: %" PRId64 " ",c1,c2,c3,c4);
  }
  */

}

void doReadFast(){
  doReadEncoders(false);
}

void doReadSlow(){
  doReadEncoders(true);
}




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

//typedef struct 

espnow_message_mpg mpgData;

// Feedback packet (with max 6 axis) that is sent from motion controller
struct fbPacket {
    uint8_t control;
    uint8_t io;
    int32_t pos[6];
    int32_t vel[6];
    uint32_t udp_seq_num;
};

espnow_message myData; // Create a espnow_message struct called myData
espnow_add_peer_msg espnowAddPeerMsg; // Used to send local mac address to motion controller. Gets added to its esp-now peer list enabling bi-directional communication
esp_now_peer_info_t peerInfo;

fbPacket fb = { 0 };

// callback when esp-now data is sent including status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("ESP-NOW: Unable to send data to peer. Resetting configured flag");
    //espnow_peer_configured = false;
  }
}

// callback function that will be executed when new esp-now data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
    //lastPacketRxTimeMs = millis();
    //Serial.printf("ESP-NOW RX: %d\r\n", len);
    //espnow_peer_configured = true;
    if (len == sizeof(myData)) { // if rx example data struct
      memcpy(&myData, incomingData, sizeof(myData));

    } else if (len == sizeof(fb)) { // if rx feedback packet struct. Print it for testing
      Serial.println("got packet");
      /*
      memcpy(&fb, incomingData, sizeof(fb));
      Serial.printf("FB: Control: %d, IO: %d, UDPSeq: %d\r\n", fb.control, fb.io, fb.udp_seq_num);
      Serial.printf("FB: POS0: %d, POS1: %d, POS2: %d\r\n", fb.pos[0], fb.pos[1], fb.pos[2]);
      Serial.printf("FB: VEL0: %d, VEL1: %d, VEL2: %d pot: %d\r\n", fb.vel[0], fb.vel[1], fb.vel[2],analogRead(34));
      */
    }
}
bool espnow_peer_configured = false;

void setupPeer(){

 /// TODO: you need to reset this peering up if the reciever resets
 memcpy(peerInfo.peer_addr, remotePeerAddress, 6);
 peerInfo.channel = 0;  
 peerInfo.encrypt = false;
  
  esp_now_del_peer(peerInfo.peer_addr); // clear old peers before adding new one
  
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer. Retry in 5s...");
  }
  
  WiFi.macAddress(espnowAddPeerMsg.mac_adddress); // Get local wifi mac and store in esp-now peer struct

  Serial.printf("ESP-NOW: Sending MAC: '%02X:%02X:%02X:%02X:%02X:%02X' to peer...\r\n", 
    espnowAddPeerMsg.mac_adddress[0],espnowAddPeerMsg.mac_adddress[1],espnowAddPeerMsg.mac_adddress[2],
    espnowAddPeerMsg.mac_adddress[3],espnowAddPeerMsg.mac_adddress[4],espnowAddPeerMsg.mac_adddress[5],
    espnowAddPeerMsg.mac_adddress[6]);

  esp_err_t result = esp_now_send(remotePeerAddress, (uint8_t*) &espnowAddPeerMsg, sizeof(espnowAddPeerMsg)); // send esp-now addPeerMsg

  if (result == ESP_OK) {
    Serial.println("Successfully sent an addPeerMessage to remote node");
    espnow_peer_configured = true;
  }
  else {
    Serial.println("ERROR: Could not send an addPeerMessage to remote node. Retry in 5s...");
    espnow_peer_configured = false;
  }
}

void mockupIdle(){

  //tft.fillScreen(TFT_WHITE);
  //tft.setRotation(3);  
  tft.setCursor(0,0);
  //tft.setTextSize(1);
  float jog_scale = 0.0025;
  float x_pos = 0.0;
  float y_pos = 0.0;
  float x_dtg = 1.0;
  float y_dtg = 1.0;
  String lcnc_state = "Idle";
  tft.printf("js: %.4d\n",jog_scale);
  tft.printf("x: %.4d\n", x_pos);
  tft.printf("y: %.4d\n", y_pos);
  tft.printf("xdtg: %.4d\n", x_dtg);
  tft.printf("ydtg: %.4d\n", y_dtg);
  tft.printf("lcnc state: %s\n", lcnc_state.c_str());
 tft.println("foo\n" );
 Serial.println("mock done");


  
}


void setupDisplay(){
  tft.init();
  //tft.begin();
  tft.fillScreen(TFT_BLACK);
  if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
         //pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
         //digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
         Serial.printf("TFT_BL is defined as %d\n", TFT_BL);
         //analogWrite(TFT_BL, 128); // Set backlight brightness (0-255)

    }

  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  tft.printf("Hello Setup! %d\n",10);
  tft.println("foo bar");

  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  tft.setTextSize(1);

  // Set the font
  tft.setCursor(0,0,2);

  tft.setTextColor(TFT_WHITE,TFT_BLACK,true);

}




void setup() {
  // put your setup code here, to run once:
  Serial.begin ( 115200 );
  setupDisplay(); 

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  setupEnc(&encoder1,enc1a,enc1b); 
  /*
  setupEnc(&encoder2,enc2a,enc2b); 
  setupEnc(&encoder3,enc3a,enc3b); 
  setupEnc(&encoder4,enc4a,enc4b); 
  */

  setupButtons();

  WiFi.mode(WIFI_MODE_APSTA);
  
  Serial.printf("WiFi MAC: %s\r\n", WiFi.macAddress().c_str());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  // Once ESPNow is successfully Init, we will register for TX&RX CBs
  esp_now_register_send_cb(OnDataSent);
  //esp_now_register_recv_cb(OnDataRecv);


  // Polling Interval is this timer's value in ms
  readEncoders.attach_ms(10,doReadFast);

  readBtns.attach_ms(50,doReadButtons);
  //slowReadEncoders.attach_ms(1000,doReadSlow);
  setupPeer();


  // THis didn't work
  //TwoWire tw = TwoWire(1); // bus 1
  //tw.setPins(21,17); // SDA, SCL
  //tw.setClock(400000); // 400kHz clock speed
  //tw.begin(); // Start I2C



  //if (!ads.begin(0x48,&tw)) {
  // use standard pins SDA 21 and SCL 22
  if(!ads.begin()) {

    Serial.println("Failed to initialize ADS.");
  }
  else{
    Serial.println("ADS1115 initialized.");

  }
  Serial.println("setup done");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("loop")
  delay(1000);
  mockupIdle();
  adc0 = ads.readADC_SingleEnded(0);
  float volts0 = ads.computeVolts(adc0);
  Serial.printf("adc0: %d, volts: %f\n",adc0,volts0);

}

