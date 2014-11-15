#!/bin/sh
#
# FlyRouter Team (c) 2004-2007 | http://www.flyrouter.net
# Network script for repaired people
# Version 0.2 20070808

IFACE="eth0"
NET="192.168."
NET_GW=".253"
NET_MASK="24"
NET_START="101"
NET_END="120"
MY_ADDR=".10"
PING_COUNT="1"
PING_INTERVAL="0.2"
PING_CMD="ping-iputils -A -n " #-l 5 -W 0.2"
ARP_REQUESTS="2"
RETRANS_TIME="50"
pass=0;

echo "Start scan in:  ${NET}${NET_START}${MY_ADDR} - ${NET}${NET_END}${MY_ADDR}" 

while true; do
	echo $ARP_REQUESTS >/proc/sys/net/ipv4/neigh/${IFACE}/mcast_solicit
	echo $RETRANS_TIME >/proc/sys/net/ipv4/neigh/${IFACE}/retrans_time
	for n in `seq $NET_START $NET_END`; do
        pre_addr="${NET}${n}"
		ip addr add ${pre_addr}${MY_ADDR}/${NET_MASK} dev $IFACE
		$PING_CMD -c $PING_COUNT -i $PING_INTERVAL ${pre_addr}${NET_GW}
		if [ $? = 0 ]; then
			echo "Found net ${pre_addr}.0/$NET_MASK"
			echo "led on" > /dev/led0
			ip route add default via ${pre_addr}${NET_GW}
			pass=1;
			break;
		fi;
		ip addr del ${pre_addr}${MY_ADDR}/${NET_MASK} dev $IFACE
	done
	[ "$pass" = 1 ] && break;
	[ $PING_COUNT -lt 10 ] && PING_COUNT=$(($PING_COUNT+1))
	[ $ARP_REQUESTS -lt 10 ] && ARP_REQUESTS=$(($ARP_REQUESTS+1))
done


