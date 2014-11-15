#!/bin/sh
echo "#!/usr/sbin/setkey -f"

. /etc/templates/lib
show_header $0

eval `kdb -q sls sys_ipsec`

echo 
echo "flush;"
echo "spdflush;"


echo 
echo "############# Security Association Database ############# "
echo
SAs=`kdb kls sys_ipsec_sad*`

for sa in $SAs; do 
	eval "`kdb get $sa`"
	[ "x${enabled}x" != "xx" ] && c="" || c="#  "

	[ -n "$ah_alg"	-a "$ah_alg" != "none" ] && ah="-A $ah_alg $ah_key" || ah="";
	if [ -n "$comp_alg" -a "$comp_alg" != "none" ]; then
		echo "${c} add $src $dst ipcomp $spi -m $mode -C $comp_alg;"
	elif [ -n "$esp_alg" -a "$esp_alg" != "none" ]; then
		echo "${c} add $src $dst esp $spi -m $mode -E $esp_alg $esp_key $ah;"
	elif [ -n "ah" ]; then
		echo "${c} add $src $dst ah $spi -m $mode $ah;"
	fi
done

echo 
echo "############# Security Policy Database ############# "
echo
SPs=`kdb kls sys_ipsec_spd*`
for sp in $SPs; do 
	eval "`kdb get $sp`"
	[ "x${enabled}x" != "xx" ] && c="" || c="#  "

	echo "${c} spdadd $src $dst $upperspec -P $direction $policy"
	[ "x$ah_enabled" != "x" ] && \
		echo "${c}	ah/$mode/$src_dst/$level"
	[ "x$esp_enabled" != "x" ] && \
		echo "${c}	esp/$mode/$src_dst/$level"
	echo "${c}		;"
done
