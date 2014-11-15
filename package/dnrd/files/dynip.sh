#!/bin/sh
#
# FlyRouter Team (c) 2004-2008 | http://www.flyrouter.net
# Update IP address for DNRD via SSH connection
# Version 0.2 20080310

[ -n "$1" ] && name=$1 || exit 1
[ -n "$2" ] && ip=$2 || exit 1

DNRDCONF="/etc/dnrd/master"
DNRDTEMP="/etc/dnrd/master.new"

{ grep -v "^[12].* $name\$" $DNRDCONF; echo "$ip $name"; } >$DNRDTEMP 
mv $DNRDTEMP $DNRDCONF

/etc/init.d/dnrd reload
