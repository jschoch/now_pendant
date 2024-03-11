#include <Arduino.h>
//#define ESP_ARDUINO_VERSION_MAJOR 3

#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
  #warning ESP_ARDUINO_VERSION_MAJOR
#else 
  #error "doh"
#endif
#include <freertos/FreeRTOS.h>
	#include <rom/gpio.h>
#include <ESP32Encoder.h>
#include <Ticker.h >


int enc1a = 13;
int enc1b = 14;

int enc2a = 15;
int enc2b = 16;

int enc3a = 17; // b
int enc3b = 18;

int enc4a = 19; // b
int enc4b = 21;

int btn1 = 39;
int btn2 = 38;


ESP32Encoder encoder1;
ESP32Encoder encoder2;

ESP32Encoder encoder3;

ESP32Encoder encoder4;

Ticker readEncoders;


void setupEnc(ESP32Encoder *encoder,int a, int b){
  encoder->attachFullQuad(a, b);
  encoder->setCount ( 0 );
}

void doReadEncoders(){
  int64_t c1 = encoder1.getCount();
  int64_t c2 = encoder2.getCount();
  int64_t c3 = encoder3.getCount();
  int64_t c4 = encoder4.getCount();
  Serial.printf("1: %i 2: %i 3: %i 4: %i ",(int)c1,(int)c2,(int)c3,(int)c4);
  //Serial.printf("1: %" PRId64 " 2: %" PRId64 " 3: %" PRId64 " 4: %" PRId64 " ",c1,c2,c3,c4);


  bool a1 = encoder1.isAttached();
  bool a2 = encoder2.isAttached();
  bool a3 = encoder3.isAttached();
  bool a4 = encoder4.isAttached();

  Serial.printf("a: %d b: %d c: %d d: %d \n",a1,a2,a3,a4);









}


void setup() {
  // put your setup code here, to run once:
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  setupEnc(&encoder1,enc1a,enc1b); 
  setupEnc(&encoder2,enc2a,enc2b); 
  setupEnc(&encoder3,enc3a,enc3b); 
  setupEnc(&encoder4,enc4a,enc4b); 

  /*

   encoder1.attachFullQuad(enc1a, enc1b);
   encoder1.setCount ( 0 );
   encoder2.attachFullQuad(enc2a, enc2b);
   encoder2.setCount ( 0 );
   */




  Serial.begin ( 115200 );
  readEncoders.attach_ms(1000,doReadEncoders);
  Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("loop")
}

