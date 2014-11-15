#!/bin/sh
#
# FlyRouter Team (c) 2004-2007 | http://www.flyrouter.net
# ADSL balancer script
# Version 0.4 20071021

sleep_time=2s
ping_size=500
ping_count=3

destination=212.110.128.65

ip=/bin/ip

get_myip() { ifconfig $1 | grep -e "P-t-P" | cut -d ':' -f 2 | cut -d ' ' -f 1; }
get_gwip() { ifconfig $2 | grep -e "P-t-P" | cut -d ':' -f 3 | cut -d ' ' -f 1; }

#IFACES="eth1 eth2"
#IFACES="adsl3 adsl2"
IFACES="eth1 adsl1 cdma1"

#eth1_myip="10.10.1.2"
#eth1_gwip="10.10.1.1"
#eth1_ping="$destination"
#eth1_usegw=1

#eth2_myip="10.10.2.2"
#eth2_gwip="10.10.2.1"
#eth2_ping="$destination"
#eth2_usegw=1

#eth3_myip="192.168.3.2"
#eth3_gwip="192.168.3.1"
#eth3_ping="$destination"
#eth3_usegw=1

#eth4_myip="10.10.4.2"
#eth4_gwip="10.10.4.1"
#eth4_ping="$destination"
#eth4_usegw=1

#adsl1_myip="`get_myip adsl1`"
#adsl1_gwip="`get_gwip adsl1`"
#adsl1_ping="$destination"

#adsl2_myip="`get_myip adsl2`"
#adsl2_gwip="`get_gwip adsl2`"
#adsl2_ping="$destination"

#adsl3_myip="`get_myip adsl3`"
#adsl3_gwip="`get_gwip adsl3`"
#adsl3_ping="$destination"

#adsl4_myip="`get_myip adsl4`"
#adsl4_gwip="`get_gwip adsl4`"
#adsl4_ping="$destination"



do_ping() {
	local myip gwip pingip dev usegw size count rtt
    dev="$1"
    eval 'myip=$'${dev}_myip
    eval 'pingip=$'${dev}_ping
    eval 'gwip=$'${dev}_gwip
    eval 'usegw=$'${dev}_usegw

    size="-s ${ping_size:-500}"
    count="-c ${ping_count:-3}"
    [ "$usegw" = 1 ] && $ip route replace $pingip via $gwip || $ip route replace $pingip dev $dev
    if [ -x "`which ping-iputils`" ]; then
        qrtt="`ping-iputils $size $count -I $myip $pingip | tail -n 1`" 
        rtt=`echo "$qrtt" | cut -d/ -f5 | cut -d. -f1 ` 
    else
        rtt=`ping $size $count $pingip | tail -n 1 | cut -d/ -f4 | cut -d. -f1 `
    fi

    # check for losses
    if [ "`echo $rtt | wc -w`" -gt 1 ] || [ "`echo $rtt | wc -w`" -lt 1 ]; then
    	echo "Bad rtt: '$rtt' qrtt: '$qrtt'"
        rtt=99991
    fi 
    eval ${dev}_rtt=$rtt
    echo "${dev}_rtt=$rtt"
}

do_reroute() {
    local dev minrtt minrtt_dev rtt

    for dev in $IFACES; do
        # init
        [ -z "$minrtt_dev" ] && minrtt_dev=$dev
        [ -z "$minrtt" ] && eval 'minrtt=$'${dev}_rtt
        
        eval 'rtt=$'${dev}_rtt
        #echo "${dev}_rtt: $rtt"
        
        # compare
        if [ $rtt -lt $minrtt ]; then
            echo "rtt: $rtt less than $minrtt -> change minrtt_dev to $dev"
            minrtt=$rtt
            minrtt_dev=$dev
        fi
    done

    # replace route
    eval 'usegw=$'${minrtt_dev}_usegw
    eval 'gwip=$'${minrtt_dev}_gwip
    if [ "$usegw" = 1 ]; then
        $ip route replace default via $gwip
    else
        $ip route replace default dev $minrtt_dev
    fi
}


while sleep $sleep_time; do
    for dev in $IFACES; do
        do_ping $dev
    done
    do_reroute
done

