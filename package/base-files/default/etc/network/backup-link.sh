#!/bin/sh
#
# FlyRouter Team (c) 2004-2008 | http://www.flyrouter.net
# BackUp route and change backbone script
# Version 0.3a 20080408
#
# Please start this script from backup interface

export normal_ip=192.168.100.1
export normal_gw=192.168.100.1
export normal_bb=eth1
export backup_ip=192.168.200.1
export backup_gw=192.168.200.1
export backup_bb=eth2

export sleep_time=5s
export ping_cmd="ping -q -c 1"

( \
  while sleep $sleep_time; do
      curr_gw=`ip r | grep default | cut -f3 -d" "`
      if $ping_cmd $normal_ip >/dev/null ; then
	  ip route replace default via $normal_gw
	  [ "$curr_gw" != $normal_gw ] && BACKBONE_DEV=$normal_bb /etc/network/fw
      else
          ip route replace default via $backup_gw
	  [ "$curr_gw" != $backup_gw ] && BACKBONE_DEV=$backup_bb /etc/network/fw
      fi
   done
) &
