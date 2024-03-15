#  now_pendant a linuxcnc pendant 

This is an example of how to collect data and send it via ESP_NOW to Linuxcnc (or something else)

## pendant: 
    this is a platformio project that has the HID/Pendant code for encoders and sending the data

## bridge: 
    this is an esp-idf project that bridges esp_now to UDP via usb networking (NCM)

## linuxcnc_comp 
    this processes UDP and gets it setup in linuxcnc

## hardware
    prototype board



## TODO

Track state and only send when polled and when there is a change
logic for encoder/rotary switch state and nvram
Receive state from LCNC
make esp_now or plain wifi opptions
use screen
use RGB led data pin
add battery/bms
moar buttons


