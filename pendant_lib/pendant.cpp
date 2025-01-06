#include <Arduino.h>
#include "pendant.h"

#include <iostream>
#include <esp_now.h>


void setupButtons(){
  for (int i = 0; i < NUM_BUTTONS; i++) {
    Serial.printf("setting pinmode for pin: %d ",buttons[i].pin);
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

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
}

void doReadFast(){
  doReadEncoders(false);
}

void doReadSlow(){
  doReadEncoders(true);
}

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