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

#include <driver/adc.h>
#include "esp_adc_cal.h"

//#define NUM_BUTTONS 2

constexpr int NUM_BUTTONS = 7;
constexpr int NUM_ENC = 1;
constexpr int NUM_POTS = 2;

bool connected = false;

float battery_voltage = 0.0;
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





int16_t adc0, adc1, adc2, adc3;

ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
ESP32Encoder encoder4;

Ticker readEncoders;
Ticker readBtns;
Ticker slowReadEncoders;
Ticker readPots;
Ticker doPings;
Ticker doScreen;
Ticker doBat;

unsigned long lastHello = millis();

State lastState;
Pos lastPos;


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
//EncInputData encoders[] = {e1,e2,e3,e4};
EncInputData encoders[] = {e1};

//  THe buttons

// Don't use 36..39 there are no pullups
// The builtin buttons on the t-display
int btn0 = 0; // 
int btn1 = 34;  // 
int btn2 = 17;
int btn3 = 2;
int btn4 = 15;
int btn5 = 33;
int btn6 =  26;

BtnInputData b0 = {
  InputType::Button,
  0, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn0
};
BtnInputData b1 = {
  InputType::Button,
  1, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn1 // pin
};

BtnInputData b2{
  InputType::Button,
  2, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn2// pin
};

BtnInputData b3 = {
  InputType::Button,
  3, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn3 // pin
};

BtnInputData b4 = {
  InputType::Button,
  4, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn4 // pin
};

BtnInputData b5 = {
  InputType::Button,
  5, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn5 // pin
};

BtnInputData b6 = {
  InputType::Button,
  6, // button ID
  1, // current state of the button
  1, // previous state of the button
  0, // pending state
  btn6 // pin
};


BtnInputData buttons[] = {b0, b1,b2,b3,b4,b5,b6};

// Pots
PotInputData p0 = {
  InputType::Pot,
  0, // id
  0, // mapped value
  0.0, // volts
  0, // prev_value
  10 // map size (range for mapping volts)
};

PotInputData p1 = {
  InputType::Pot,
  1, // id
  0, // mapped value
  0.0, // volts
  0, // prev_value
  50 // map size
};


PotInputData pots[] = {p0,p1};


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
}

void initScreen(){
  
  tft.setTextColor(TFT_BLACK,TFT_RED,true);
  tft.setCursor(0,0,2);
  tft.setTextSize(3);
  tft.println("Not Connected to Server");
  tft.printf("Peer connected?",espnow_peer_configured ? "Y":"N");
}

void idleScreen(){
  
  tft.setTextColor(TFT_BLACK,TFT_WHITE,true);
  tft.setCursor(0,0,2);
  tft.setTextSize(2);
  tft.println("Connected, waiting for estop");
}

void notAutoScreen(){
  
  tft.setTextColor(TFT_BLACK,TFT_WHITE,true);
  tft.setCursor(0,0,2);
  tft.setTextSize(2);
  // You should only do DTG in auto and mdi modes
  //tft.printf("X: %f Z: %f XDTG: %f ZDTG: %f",lastPos.x,lastPos.z,lastPos.x_dtg, lastPos.z_dtg);
  if(lastState.homed){
    tft.printf("X:  %07.2f \nZ:  %07.2f\n",lastPos.x,lastPos.z);
  }
  else{
    tft.println("Not Homed, cant' mpg jog");
  }

  tft.printf("Jog: %1.3f\n",lastPos.jog_scale);
  tft.setTextSize(1);
  tft.printf("t: %i",millis());
  tft.printf("  vbat: %f\n",battery_voltage);
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
  tft.setRotation(1);
  tft.setTextSize(1);

  // Set the font
  tft.setCursor(0,0,2);

  tft.setTextColor(TFT_WHITE,TFT_BLACK,true);

}

// the screen states
enum class ScS{
  INIT,
  IDLE,
  NA,
  A,
  MDI
};

ScS lss = ScS::INIT;

void drawScreen(){
  if(connected){
    if(lastState.machine){
      if(lss != ScS::IDLE){
        tft.fillScreen(TFT_YELLOW);
        lss = ScS::IDLE;
      }
      idleScreen();
    }
    else if(lastState.machine == 0){
      // terrible name
      if(lss != ScS::NA){
        tft.fillScreen(TFT_WHITE);
        lss = ScS::NA;
        // we should send the pot position here, but also where else on lcnc startup it will have the wrong value
        // TODO readd ads
        doReadPots(true);
      }
      notAutoScreen();
    }
  }else{
    if(lss != ScS::INIT){
      tft.fillScreen(TFT_RED);
      lss= ScS::INIT;
    } 
    
    initScreen();
  }

}

void doReadBat(){
  digitalWrite(14, HIGH);
  delay(1);
  float measurement = (float) analogRead(34);
  battery_voltage = (measurement / 4095.0) * 7.26;
  digitalWrite(14, LOW);
  //Serial.println(battery_voltage);
}


void doReadPotsHack(){
  // arg
  doReadPots(false);
  
}


void doPing(){
  sendHello();
  int delta = millis() - lastHello;
  if(delta > 1500){
    Serial.printf("Connection timeout %i",delta);
    connected = false;
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin ( 115200 );
  setupDisplay(); 
  initScreen();
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


  //if (!ads.begin(0x48,&tw)) {
  // use standard pins SDA 21 and SCL 22

  // TODO add ads back
  if(!ads.begin()) {

    Serial.println("Failed to initialize ADS.");
  }
  else{
    Serial.println("ADS1115 initialized.");

  }

  // Polling Interval is this timer's value in ms
  readEncoders.attach_ms(20,doReadFast);

  readBtns.attach_ms(50,doReadButtons);
  readPots.attach_ms(100,doReadPotsHack);
  doPings.attach_ms(1500,doPing);
  doScreen.attach_ms(20,drawScreen);
  doBat.attach_ms(1000,doReadBat);

  //slowReadEncoders.attach_ms(1000,doReadSlow);

  // BATTERY STUFF

  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  pinMode(14, OUTPUT);

  setupPeer();


  


  
  Serial.println("setup done");

  
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("loop")
  delay(100);

  /*
  bool a = digitalRead(enc1a);
  bool b = digitalRead(enc1b);
  Serial.printf("Enc v: %i  \n",encoders[0].encoder->getCount());
  Serial.printf(" a: %s b: %s",a ? "H":"L",b ? "H":"L");
  */

  
  //mockupIdle();
  

}

