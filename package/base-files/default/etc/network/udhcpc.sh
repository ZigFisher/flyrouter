#!/bin/sh
#
# Sample udhcpc bound script

RESOLV_CONF="/etc/udhcpc/resolv.conf"



echo -n > $RESOLV_CONF
[ -n "$domain" ] && echo domain $domain >> $RESOLV_CONF
for i in $dns
do
	echo adding dns $i
	echo nameserver $i >> $RESOLV_CONF	
done
echo nameserver 208.67.222.222 >> $RESOLV_CONF
echo nameserver 208.67.220.220 >> $RESOLV_CONF
