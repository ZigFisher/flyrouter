#!/bin/sh

btstarts=0
scanfail=0
insmod="insmod"
#[ -x `which modprobe` ] && insmod="modprobe"
[ -x `which usleep` ] && modsleep="usleep 300000" || modsleep="sleep 1"

ins_mod() {
	local mod="$1"
	info 4 "Loading $mod..."
	$insmod $mod || warn "insmod fail"
	$modsleep
}

rm_mod() { 
	local mod="$1"
	info 4 "Unloading $mod..."
	if lsmod | grep $mod >/dev/null; then
		rmmod $mod || warn "rmmod fail"
	fi
	$modsleep
}

stop_bt() {
	local rmmodules="hci_uart hci_usb rfcomm l2cap bluez "
	## sco bnep crc32

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
	local insmodules="bluez l2cap rfcomm hci_usb hci_uart"
	## crc32 bnep sco

	stop_bt

	info 3 Loading modules...
	for m in $insmodules; do ins_mod $m; done

	info 3 Starting daemons...
	start_daemon hcid
	# start_daemon sdpd
	# start_daemon sdptool add OPUSH
	#
	# start_daemon sdptool add FTP
	# start_daemon sdptool add DUN
	# start_daemon sdptool add SP
	# start_daemon sdptool add NAP
	# start_daemon pand --listen --master --role NAP
	
	BTDEVICES="`hciconfig | grep Type: | cut -f1 -d: `"
	BTDEVICES="`echo $BTDEVICES`"
	[ -n "BTDEVICES" ] ||  die "No bluetooth devices found"
	[ -z "$BTDEV_SCAN" ] && BTDEV_SCAN=`echo "$BTDEVICES" | cut -f1 -d" "`
	if [ -z "$BTDEV_JACK" ]; then
		BTDEV_JACK="${BTDEVICES##$BTDEV_SCAN}"
		BTDEV_JACK=`echo $BTDEV_JACK`
		[ -z "$BTDEV_JACK" ] && BTDEV_JACK="$BTDEV_SCAN" 
	fi
	
	local dev;
	for dev in $BTDEVICES; do 
		info 3 "Configuring ${dev}..."
		hciconfig $dev up
		# hciconfig $dev aclmtu 50:3
		hciconfig $dev
	done
	
	hciconfig | grep "BD Address" | while read q1 q2 m q3 q4; do
		local devdir="$JACKDIR/`echo $m | sed s/://g`"
		mkdir -p $devdir
		echo 1 > $devdir/iam
	done
	scanfail=0
	btstarts=$(($btstarts+1))
	[ -n "$MAX_BTSTARTS" -a "$MAX_BTSTARTS" -gt 0 -a "$btstarts" -gt "$MAX_BTSTARTS" ] && reboot # auto reboot on many restarts
}
