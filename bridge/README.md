# ESP_NOW to UDP bridge

This takes esp_now packets and forwards them via UDP to the ip/port specified in enow.c  as `const char* target_ip `

Thanks to Kasper Nyhus @kaspernyhus  for his repo here: kaspernyhus/esp_tinyusb_test

NB: Work in progress!


#### Test

You should be able to ping the IP set in usb_ncm.h, this may require ifup/ifdown and some network fuddling on the host side.  There are issues with the windows NCM driver, but since this is linuxcnc it shouldn't be a problem.

Once you can test it should "just work" assuming you've set STATIC_IP_ADDR and target_ip
