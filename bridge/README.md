# ESP_NOW to UDP bridge

This takes esp_now packets and forwards them via UDP to the ip/port specified in enow.c  as `const char* target_ip `

Thanks to Kasper Nyhus @kaspernyhus  for his repo here: kaspernyhus/esp_tinyusb_test

NB: Work in progress!


#### Test

You should be able to ping the IP set in usb_ncm.h, this may require ifup/ifdown and some network fuddling on the host side.  There are issues with the windows NCM driver, but since this is linuxcnc it shouldn't be a problem.

Once you can test it should "just work" assuming you've set STATIC_IP_ADDR and SERVER_IP_ADDR





if you want to test from the linuxcnc side you can run

`netcat -u 192.168.11.4 8080`  and press any key and hit enter, you should see a msg back.


`netstat -an |grep 8080` assuming the linuxcnc hal component is running should show you a udp 0.0.0.0:8080 listener

You may need a static route  but only if you somehow don't setup the hal component UDP server on the correct interface

`sudo ip route add 192.168.11.160/32 via 192.168.10.22 dev usb0`
