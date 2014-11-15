#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
#

debug(){
	[ $DEBUG ] && echo $* >>/tmp/debug.log #>&2
}

warn(){
	[ $WARN ] && echo $* >>/tmp/debug.log #&2
}

colorizelog(){
	local red='root\|err\|DROP\|REJECT\|fail'
	local yellow='warn'
	sed -e 's-\(/[ubsd][^ ]*\)-<b>\1</b>-g' -e 's-\('$red'\)-<font color=red>\1</font>-g' -e 's-\('$yellow'\)-<font color=yellow>\1</font>-g'
}

handle_list_del_item(){
	if [ "$FORM_do" = "del" ]; then
		$kdb lrm "$FORM_item"
		[ -n "$1" ] && update_configs_and_service_reload "$1"
	fi
}

iface_add() {
	local iface;
	while [ -n "$1" ]; do
		iface=$1
		shift;
	    kdb set sys_ifaces="$sys_ifaces $iface" : \
	    	set sys_iface_${iface}_proto=$iface_proto : \
	    	set sys_iface_${iface}_real=$iface : \
	    	set sys_iface_${iface}_valid=1 : \
	    	set sys_iface_${iface}_auto=0 : \
	    	set sys_iface_${iface}_method=none
	done
}

iface_del() {
	local liface;
	while [ -n "$1" ]; do
		liface=$1
		/sbin/ifdown ${liface} 2>&1 | ${LOGGER}
		shift;
		kdb rm "sys_iface_${liface}_*"
	done
	iface_update_sys_ifaces
}

iface_update_sys_ifaces() {
	local ifaces=`kdb ls sys_iface*valid | sed 's/\(sys_iface_\)\(.*\)\(_valid.*\)/\2/' | sort | uniq`
	kdb set sys_ifaces="$ifaces"
}


humanUnits(){
    local k="0";
    if [ "$1" = "-k" ]; then
        k=1;
        shift
    fi
    local l="$1"
    local units="$2"
    local base=1024;
    local s=0;
    local u;
    local dc;
    /usr/bin/dc -e "1" 2>/dev/null && dc="dc -e "
    /usr/bin/dc "1" 2>/dev/null && dc="dc "
    [ "$3" ] && base=$3
    if [ "$l" -ge $(($base*$base*$base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / 1024 / 1024 / p) || t=$(($l/$base/$base/$base))
        s=$((3+$k));
    elif [ "$l" -ge $(($base*$base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / 1024 / p) || t=$(($l/$base/$base))
        t=$(($l/$base/$base))
        s=$((2+$k));
    elif [ "$l" -ge $(($base)) ]; then
        [ "$dc" ] && t=$($dc $l 1024 / p) || t=$(($l/$base))
        t=$(($l/$base))
        s=$((1+$k));
    else
        t=$l
        s=$k;
    fi

    [ "$s" = "3" ] && u='G'
    [ "$s" = "2" ] && u='M'
    [ "$s" = "1" ] && u='K'
    [ "$s" = "0" ] && u=''
    echo "$t ${u}${units}";
}

getFsType(){
    local dev=$1;
    type=$(mount| grep "^$dev " | cut -f5 -d" " | head -1)
    echo $type
}

#int2ip() {
#	i=$1
#	d0=$(($i/256/256/256));
#	d1=$((($i-$d0*256*256*256)/256/256));
#	d2=$((($i-$d0*256*256*256-$d1*256*256)/256));
#	d3=$(($i-$d0*256*256*256-$d1*256*256-$d2*256));
#	echo "$d0.$d1.$d2.$d3";
#}

#ip2int2() {
#	echo $1 | { IFS="." read a0 a1 a2 a3; echo $(($a0*256*256*256+$a1*256*256+$a2*256+$a3)); }
#}


valid_netmask () {
  return $((-($1)&~$1))
}

ip2int () (
  set $(echo $1 | tr '\.' ' ')
  echo $(($1<<24|$2<<16|$3<<8|$4))
)

int2ip () {
  echo $(($1>>24&255)).$(($1>>16&255)).$(($1>>8&255)).$(($1&255))
}

get_new_serial() {
	local oldserial=$1
	local newserial
	local today=$(date +%Y%m%d)
	for i1 in 0 1 2 3 4 5 6 7 8 9; do
		for i2 in 0 1 2 3 4 5 6 7 8 9; do
			newserial=${today}${i1}${i2}
			if [ "$oldserial" -lt "$newserial" ]; then
				echo $newserial;
				return 0;
			fi
		done
	done
}

