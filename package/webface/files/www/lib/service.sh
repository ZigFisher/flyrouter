#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

service_reload(){
	local service="$1"
	
	case "$service" in
	network)
		if [ -n "$iface" ]; then
			/sbin/ifdown $iface;
			/sbin/ifup $iface
		else
			/etc/init.d/network restart
		fi
	;;
	dhcp)
		/etc/init.d/udhcpd restart $iface
	;;
	dns_server)
		/etc/init.d/bind restart
	;;
	dsl*)
		iface=${sybsys##*.}
		/etc/init.d/dsl restart $iface
	;;
	e1*)
		iface=${sybsys##*.}
		/etc/init.d/e1 restart $iface
	;;
	fw)
		/etc/init.d/fw restart
	;;
	logging)
		/etc/init.d/sysklog restart
	;;
	ipsec)
		/etc/templates/ipsec-tools.sh | /usr/sbin/setkey -c
	;;
	esac
}

