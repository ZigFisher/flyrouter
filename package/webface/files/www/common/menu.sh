
L1(){
	[ -n "$2" ] && echo "<strong><a href='?controller=$2'>$1</a></strong><br>" \
			|| echo "<strong>$1</strong><br>"
}

L2(){
	local class=navlnk
	[ -n "$2" ] && echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>" \
		|| echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class='$class'>$1</span><br>"
}

L3(){
	local class=navlnk
	[ -n "$3" ] && class="$3"
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>"
}

L1 System welcome
	L2 General 	'general'
	L2 Security 'passwd'
		L3 'PPP secrets' 'auth_ppp'
	L2 Time		'time'
	L2 Logging	'logging'
	L2 SHDSL	'dsl'
	for i in `kdb get sys_dsl_ifaces`; do
		class=""
		[ "$FORM_iface" = "$i" ] && class="navlnk_a"
		L3	$i "dsl&iface=$i" $class
	done
	tmp=`kdb get sys_e1_ifaces`
	if [ -n "$tmp" ]; then
	    L2 E1	'e1'
	    for i in `kdb get sys_e1_ifaces`; do
		class=""
		[ "$FORM_iface" = "$i" ] && class="navlnk_a"
		L3	$i "e1&iface=$i" $class
	    done
	fi
	L2 Switch	'adm5120sw'
	L2 DNS		'dns'
	
L1 Network
	L2 Interfaces ifaces
	for i in `kdb get sys_ifaces`; do
		lclass=''
		lpage=''
		if [ "$controller" = "iface" ]; then
		    [ "$FORM_iface" = "$i" ] && lclass="navlnk_a"
			[ -n "$FORM_page" ] && lpage="&page=$FORM_page"
		fi
		L3	$i "iface&iface=${i}${lpage}" $lclass
	done
	L2 Firewall	fw
		L3 Filter	"fw&table=filter"
		L3 NAT		"fw&table=nat"
	L2 IPSec ipsec
	
L1 Services
	L2 "DNS Server" dns_server
	L2 "DHCP Server" dhcp_server
	L2 "PPTP Server" pptp_server
L1 Tools 
	L2 syslog	"tools&page=syslog"
	L2 dmesg	"tools&page=dmesg"
	L2 ping	"tools&page=ping"
	L2 mtr	"tools&page=mtr"
	L2 dig	"tools&page=dig"
	L2 tcpdump	"tools&page=tcpdump"
	L2 reboot	"tools&page=reboot"
L1 Configuration
	L2 backup	"cfg&page=backup"
	L2 restore	"cfg&page=restore"
L1 Expert
	L2 kdb	"expert&page=kdb"
	L2 "kdb set"	"expert&page=kdb_set"

#	<a href="javascript:showhide('diag','tri_diag')">
#					<img src="img/tri_c.gif" id="tri_diag" width="14" height="10" border="0"></a><strong><a href="javascript:showhide('diag','tri_diag')" class="navlnk">Diagnostics</a></strong><br>
#					<span id="diag"></span>
