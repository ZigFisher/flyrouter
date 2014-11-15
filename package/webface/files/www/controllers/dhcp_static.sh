#!/usr/bin/haserl
	
	frame=1

	export iface=${FORM_iface:-eth0}

	case "$FORM_do" in
		del) $kdb lrm "$FORM_item";;
	esac;
	
	eval `kdb -qqc ls sys_iface_${iface}_dhcp_host*`
	render_form_header dhcp

	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_iface_${iface}_dhcp_host_${lineno}}\""
		eval "$val"
		echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
		render_list_btns dhcp_static_edit "sys_iface_${iface}_dhcp_host_${lineno}" "iface=${iface}"
		echo "</td></tr>"
	}
	
	
	render_list_header dhcp_static sys_iface_${iface}_dhcp_host_ "iface=${iface}" "No" "Name" "IP Address" "MAC Address"
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
