# install

you likely need to install gramme, pip3 would do with the debian package override, should fix this...

`pip3 install gramme --break-system-packages`

after that it should just work.  It will listen 0.0.0.0 so any udp from any connected network on the 8080 port should be able to send jog commands.
If you are secure minded you may not like that
It would make sense to lock the network down to the 192.168.11.3 that the usb NCM driver uses...
