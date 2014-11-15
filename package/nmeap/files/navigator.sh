#!/bin/sh
#
# FlyRouter Team (c) 2004-2008 | http://www.flyrouter.net
# Check reset button status and write current KML file to USB
# Version 0.2 20070805

echo "led switch 0" > /dev/switch

i=0
while sleep 1 ; do
  if [ q`cat /dev/switch `q = q0q ]; then
    while true; do [ -r /mnt/usb/file${i} ] && i=$(($i+1))  || break; done
    echo "led blink 50">/dev/gpio3
    cat /www/location.kml >/mnt/usb/file${i}
    echo "led off">/dev/gpio3
    sleep 3
    echo "led on">/dev/gpio3
  fi;
done 
