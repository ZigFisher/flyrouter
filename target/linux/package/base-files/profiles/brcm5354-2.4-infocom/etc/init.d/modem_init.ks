#!/bin/sh

test -f /bin/router_functions && . /bin/router_functions
test -x /usr/sbin/usb_modeswitch || exit 0

#MODEL="-v 0x12d1 -p 0x1003 -H 1 -d 1"                                                                                      # Huawei E219 - Utel
#MODEL="-v 0x12d1 -p 0x1001 -H 1 -d 1"                                                                                      # Huawei EC226 - PeopleNet
#MODEL="-v 0x1c9e -p 0x1001 -V 0x1c9e -P 0x6061 -m 0x05 -M 55534243123456780000000000000606f50402527000000000000000000000"  # Novacom Wireless GNS-3.5G & Alcatel X030
MODEL="-v 0x12d1 -p 0x1446 -m 0x01 -M 55534243000000000000000000000011060000000000000000000000000000"                       # Huawei E1550 - Kievstar
#MODEL="-v 0x1199 -p 0x0fff -V 0x1199 -P 0x0023 -S 1"                                                                       # Sierra Wireless Compass 597 - Peoplenet
#MODEL="-v 0x19d2 -p 0xfff5 -V 0x19d2 -P 0xfff1 -m 0x0a -M 5553424312345678c00000008000069f010000000000000000000000000000"  # ZTE AC2726
#MODEL="-v 0x19d2 -p 0x2000 -V 0x19d2 -P 0x0031 -m 0x01 -M 55534243123456782000000080000c85010101180101010101000000000000"  # ZTE MF626 (MF100?)
#MODEL="-v 0x19d2 -p 0x2000 -V 0x19d2 -P 0x0031 -m 0x01 -M 5553424312345678000000000000061b000000030000000000000000000000"  # ZTE MF100
#MODEL="-v 0x12d1 -p 0x1413 -H 1"                                                                                           # Huawei E168 - Intertelecom
#MODEL="-v 0x106c -p 0x3b03 -V 0x106c -P 0x3715 -m 0x05 -M 555342431234567824000000800008ff024445564348470000000000000000"  # Intertelecom UM175
#MODEL="-v 05c6 -p 1000 -V 16d5 -P 6502 -m 08 -M 5553424312345678000000000000061b000000020000000000000000000000"            # ADU-500A
#MODEL="-v 0x12d1 -p 0x1446 -V 0x12d1 -P 0x1001 -M 55534243123456780000000000000011060000000000000000000000000000"          # Huawei EC122
#MODEL="-v 0x12d1 -p 0x1446 -M 55534243123456780000000000000011060000000000000000000000000000"                              # Huawei EC122

case "$1" in
  start)
	if [ -x /usr/sbin/usb_modeswitch ]; then
		msgn "3G modem with internal flash initialization: ";
		[ -f /proc/bus/usb/devices ] || mount -t usbfs usbdev /proc/bus/usb ; sleep 1;
		usb_modeswitch -Q $MODEL >/dev/null && ok || fail
	fi
	;;
  *)
	echo $"Usage: $0 {start}"
	exit 1
esac

exit $?

