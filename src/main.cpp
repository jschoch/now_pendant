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

#include <esp_now.h>
#include <WiFi.h>
#include <MsgPack.h>

MsgPack::Packer packer;

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


// (84:f7:03:c0:24:38)
uint8_t remotePeerAddress[] = {0x84, 0xf7, 0x03, 0xc0, 0x24, 0x38}; // TODO: Implement broadcast with security
//uint8_t remotePeerAddress[] = {0x24, 0xD7, 0xEB, 0xB7, 0xA7, 0x2C}; // TODO: Implement broadcast with security

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
  //MsgPack::bool b = 0;
  MsgPack::arr_t<int> enc1_list {1, 2, (int32_t)c1};
  MsgPack::arr_t<int> enc2_list {1, 2, (int32_t)c2};

  //MsgPack::arr_t<int> encoder_list {enc1_list,enc2_list};

  //packer.to_map("h",0,"e",encoder_list);
  packer.serialize(MsgPack::map_size_t(2), "h",1, "e" , MsgPack::arr_size_t(2), enc1_list, enc2_list);

  esp_err_t result = esp_now_send(remotePeerAddress, packer.data(), packer.size()); // send esp-now addPeerMsg
  packer.clear();
  //char *mm = "hello";
  //esp_err_t result = esp_now_send(remotePeerAddress, (uint8_t *) mm, sizeof(mm)); // send esp-now addPeerMsg
  

  if (result == ESP_OK) {
    Serial.print(".");
  }else{
    Serial.println(" couldn't send msg");
    Serial.println(result);
  }

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

void setup() {
  // put your setup code here, to run once:
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  setupEnc(&encoder1,enc1a,enc1b); 
  setupEnc(&encoder2,enc2a,enc2b); 
  setupEnc(&encoder3,enc3a,enc3b); 
  setupEnc(&encoder4,enc4a,enc4b); 

  WiFi.mode(WIFI_MODE_APSTA);
  
  Serial.printf("WiFi MAC: %s\r\n", WiFi.macAddress().c_str());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  // Once ESPNow is successfully Init, we will register for TX&RX CBs
  esp_now_register_send_cb(OnDataSent);
  //esp_now_register_recv_cb(OnDataRecv);

  Serial.begin ( 115200 );
  readEncoders.attach_ms(1000,doReadEncoders);
  setupPeer();
  Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("loop")
}

