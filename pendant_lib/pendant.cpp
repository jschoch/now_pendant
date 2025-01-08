#include <Arduino.h>
#include "pendant.h"

#include <iostream>
#include <esp_now.h>
#include <MsgPack.h>


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

void sendPotUpdate(int id){

  packer.serialize(MsgPack::arr_size_t(4),false,"p",id,pots[id].map_value);
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

int mapFloatToInt(float floatValue) {
  // Check if the float value is within the expected range
  if (floatValue < 0.0f) {
    floatValue = 0.0f; // Clamp to minimum
  } else if (floatValue > 3.3f) {
    floatValue = 3.3f; // Clamp to maximum
  }

  // Perform the linear mapping
  int intValue = map(static_cast<int>(floatValue * 100), 0, 330, 1, 10);

  return intValue;
}

void doReadPots(){
    for (int i = 0;i < NUM_POTS;i++){
        int16_t adc0 = ads.readADC_SingleEnded(i);
        float volts0 = ads.computeVolts(adc0);
        int mapped_pot = mapFloatToInt(volts0);
        pots[i].map_value = mapped_pot;
        if(pots[i].prev_state != mapped_pot){
            pots[i].prev_state = mapped_pot;
            sendPotUpdate(i);
        }
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





int server_msg_count = 0;
// callback function that will be executed when new esp-now data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
    Serial.printf("got a message: len: %i\n",len);
    MsgPack::Unpacker unpacker;

    bool t = unpacker.feed(incomingData,len);
    //Serial.printf("Unpacker worked?  %s\n",t ? "true": "false");

    /* Testing array
    if(t){
        String s3;
        int i3;
        unpacker.feed(incomingData,len);
        unpacker.from_array(s3,i3);
        unpacker.clear();
        Serial.printf("ary  %s : %i\n",s3,i3);
    }
    */

    // Testing map
    /*
    if(t){
        String s4;
        int i4;
        unpacker.feed(incomingData,len);
        unpacker.from_map(s4,i4);
        unpacker.clear();
        Serial.printf("map  %s : %i\n",s4,i4);
    }
    */

    if(t){
        uint8_t msg_type;
        //std::array<unsigned char, (len-1)> msg;
        std::vector<unsigned char> msg;
        unpacker.feed(incomingData,len);
        unpacker.from_array(msg_type,msg);
        unpacker.clear();
        Serial.printf("Got msg type: %u and array %u",msg_type,sizeof(msg));
        
        //YUCK
        if (msg_type == static_cast<uint8_t>(MsgType::PING)){
            Serial.println("got a ping");
            //  should get some basic machine state with this msg
            Amsg amsg;
            unpacker.feed(incomingData, len);
            //unpacker.from_array(a1,a2,a3,a4);
            unpacker.deserialize(amsg);
            Serial.printf("t: %u system: %i machine: %i motion: %i homed: %i\n",amsg.msg_type,amsg.state.system,amsg.state.machine,amsg.state.motion,amsg.state.homed);
            lastState.system = amsg.state.system;
            lastState.machine = amsg.state.machine;
            lastState.motion = amsg.state.motion;
            lastState.homed = amsg.state.homed;

        }else if (msg_type == static_cast<uint8_t>(MsgType::STATE)){
            Serial.println("got a state msg");
            int a1,a2,a3,a4;
            Amsg amsg;
            unpacker.feed(incomingData, len);
            //unpacker.from_array(a1,a2,a3,a4);
            unpacker.deserialize(amsg);
            Serial.printf("t: %u system: %i machine: %i motion: %i homed: %i\n",amsg.msg_type,amsg.state.system,amsg.state.machine,amsg.state.motion,amsg.state.homed);
        }
        else{
            Serial.println("Unknown msg");
        }
        unpacker.clear();
    }else{
        Serial.println("could not unpack data");
    }
    

    /*   This was failed attemps to get unpacking working, array is working
    String s2  ;
    int i2 ;
    //unpacker.deserialize(MsgPack::map_size_t(1), s2, i2);
    unpacker.feed(incomingData,len);
    unpacker.from_map(s2,i2);
    unpacker.clear();

    Serial.printf("trying again....   %s : %i\n",s2,i2);

    

    unpacker.feed(incomingData,len);
    unpacker.deserialize(s2);
    unpacker.clear();
    Serial.printf("single %s \n",s2);
    */
    


    //lastPacketRxTimeMs = millis();
    //Serial.printf("ESP-NOW RX: %d\r\n", len);
    //espnow_peer_configured = true;

    /*
    if (len == sizeof(myData)) { // if rx example data struct
      memcpy(&myData, incomingData, sizeof(myData));

    } else if (len == sizeof(fb)) { // if rx feedback packet struct. Print it for testing
      
      memcpy(&fb, incomingData, sizeof(fb));
      Serial.printf("FB: Control: %d, IO: %d, UDPSeq: %d\r\n", fb.control, fb.io, fb.udp_seq_num);
      Serial.printf("FB: POS0: %d, POS1: %d, POS2: %d\r\n", fb.pos[0], fb.pos[1], fb.pos[2]);
      Serial.printf("FB: VEL0: %d, VEL1: %d, VEL2: %d pot: %d\r\n", fb.vel[0], fb.vel[1], fb.vel[2],analogRead(34));
    }
    */
    connected = 1;
    lastHello = millis();
    server_msg_count++;
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

void sendHello(){
  packer.serialize(MsgPack::arr_size_t(2),true,"b");
  esp_err_t result = esp_now_send(remotePeerAddress, packer.data(), packer.size()); // send esp-now addPeerMsg
  packer.clear();
  //Serial.println("sending ping");

  if (result == ESP_OK) {
    Serial.print(".");
  }else{
    Serial.println(" couldn't send msg");
    Serial.println(result);
  }
}