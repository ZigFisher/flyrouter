#!/bin/sh

btstarts=0
scanfail=0
insmod="insmod"
#[ -x `which modprobe` ] && insmod="modprobe"

ins_mod() {
	local mod="$1"
	info 4 "Loading $mod..."
	$insmod $mod || warn "insmod fail"
	sleep 1
}

rm_mod() { 
	local mod="$1"
	info 4 "Unloading $mod..."
	if lsmod | grep $mod >/dev/null; then
		rmmod $mod || warn "rmmod fail"
	fi
	sleep 1
}

stop_bt() {
	local rmmodules="bnep crc32 hci_uart rfcomm hci_usb l2cap bluez "

	hciconfig hci0 down
	killall ussp-push pand sdptool sdpd hcid 2>/dev/null; sleep 2
	killall -9 ussp-push pand sdptool sdpd hcid 2>/dev/null; 

	info 3 Unloading modules...
	for m in $rmmodules; do rm_mod $m; done
}

start_daemon() {
	info 3 Startging $1
	$*
	sleep 1
}

start_bt() {
	local m
	local insmodules="bluez l2cap hci_usb rfcomm hci_uart crc32 bnep"
	# sco

	stop_bt

	info 3 Loading modules...
	for m in $insmodules; do ins_mod $m; done

	info 3 Configuring hci0...
	hciconfig hci0 up
	#hciconfig hci0 aclmtu 50:3
	hciconfig
	info 3 Starting daemons...
	start_daemon hcid
	start_daemon sdpd
	start_daemon sdptool add OPUSH
	start_daemon sdptool add FTP
	start_daemon sdptool add NAP
	#start_daemon sdptool add DUN
	#start_daemon sdptool add SP
	#start_daemon pand --listen --master --role NAP
	scanfail=0
	btstarts=$(($btstarts+1))
	[ -n "$MAX_BTSTARTS" -a "$MAX_BTSTARTS" -gt 0 -a "$btstarts" -gt "$MAX_BTSTARTS" ] && reboot # auto reboot on many restarts
}
