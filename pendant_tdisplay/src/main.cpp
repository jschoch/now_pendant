#include <Arduino.h>




#include "../../pendant_lib/pendant.h"
#include <freertos/FreeRTOS.h>
#include <rom/gpio.h>
#include <ESP32Encoder.h>
#include <Ticker.h >

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <User_Setup_Select.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();


#include <MsgPack.h>
MsgPack::Packer packer;

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

//#define NUM_BUTTONS 2
constexpr int NUM_BUTTONS = 2;
constexpr int NUM_ENC = 1;

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
// The builtin buttons on the t-display
int btn1 = 0; // 
int btn2 = 34;  // 

int16_t adc0, adc1, adc2, adc3;

ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
ESP32Encoder encoder4;

Ticker readEncoders;
Ticker readBtns;
Ticker slowReadEncoders;





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



//EncInputData encoders[] = {e1,e2,e3,e4};
EncInputData encoders[] = {e1};


// (84:f7:03:c0:24:38)
uint8_t remotePeerAddress[] = {0x84, 0xf7, 0x03, 0xc0, 0x24, 0x38}; // TODO: Implement broadcast with security
//uint8_t remotePeerAddress[] = {0x24, 0xD7, 0xEB, 0xB7, 0xA7, 0x2C}; // TODO: Implement broadcast with security















 /*
   if(print){
    Serial.printf("1: %i 2: %i 3: %i 4: %i ",(int)e1.v,(int)e2.v,(int)e3.v,(int)e4.v);
    //Serial.printf("1: %" PRId64 " 2: %" PRId64 " 3: %" PRId64 " 4: %" PRId64 " ",c1,c2,c3,c4);
  }
  */






//typedef struct 

espnow_message_mpg mpgData;


espnow_message myData; // Create a espnow_message struct called myData
espnow_add_peer_msg espnowAddPeerMsg; // Used to send local mac address to motion controller. Gets added to its esp-now peer list enabling bi-directional communication
esp_now_peer_info_t peerInfo;

fbPacket fb = { 0 };


bool espnow_peer_configured = false;



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

  // only one encoder on this board
  setupEnc(&encoder1,enc1a,enc1b); 

  setupButtons();

  WiFi.mode(WIFI_MODE_APSTA);
  //WiFi.setChannel(11);
  esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE);
  
  Serial.printf("WiFi MAC: %s\r\n", WiFi.macAddress().c_str());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  // Once ESPNow is successfully Init, we will register for TX&RX CBs
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);


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

