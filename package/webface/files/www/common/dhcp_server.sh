#!/bin/sh

set_dhcp_server_vars() {
	kdb_vars="$kdb_vars bool:sys_iface_${iface}_dhcp_enabled "
	eval "dhcp_enabled=\$$sys_iface_${iface}_dhcp_enabled"
	if [ "$dhcp_enabled" = 1 ]; then
		kdb_vars="$kdb_vars int:sys_iface_${iface}_dhcp_lease_time"
		kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_router str:sys_iface_${iface}_dhcp_nameserver"
		kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_domain_name "
		kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_ntpserver str:sys_iface_${iface}_dhcp_winsserver "
		kdb_vars="$kdb_vars str:sys_iface_${iface}_dhcp_startip str:sys_iface_${iface}_dhcp_endip str:sys_iface_${iface}_dhcp_netmask "
	fi
}

render_dhcp_server_common() {

	help_1="dhcp_server"
	help_2=""
	render_table_title "DHCP Server on $iface"
	# sys_iface_${iface}_dhcp_enabled
	tip=""
	desc="Check this item if you want use DHCP server on your LAN"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable DHCP server" sys_iface_${iface}_dhcp_enabled

	eval "dhcp_enabled=\$$sys_iface_${iface}_dhcp_enabled"
	if [ "$dhcp_enabled" = 1 ]; then
		desc="Start of dynamic ip address range for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "Start IP" sys_iface_${iface}_dhcp_startip

		desc="End of dynaic ip address range for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "End IP" sys_iface_${iface}_dhcp_endip

		tip="<b>Example:</b> 255.255.255.0"
		desc="Netmask for your LAN (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_netmask"
		render_input_field text "Netmask" sys_iface_${iface}_dhcp_netmask

		# sys_iface_${iface}_dhcp_router
		tip="Router for subnet"
		desc="Default router for your LAN hosts (dotted quad) "
		validator=$validator_ipaddr
		render_input_field text "Default router" sys_iface_${iface}_dhcp_router

		# sys_iface_${iface}_dhcp_lease_time
		tip="Select default lease time"
		desc="Please select lease time"
		validator='tmt:message="Please select lease time"'
		render_input_field select "Default lease time" sys_iface_${iface}_dhcp_lease_time 600 "10 minutes" 1800 "30 minutes" 3600 "1 hour" \
				10800 "3 hours" 36000 "10 hours" 86400 "24 hours" #infinite "Infinite"

		# sys_iface_${iface}_dhcp_nameserver
		tip="DNS server for your LAN hosts<br>You can use this device as DNS server"
		desc="DNS server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "DNS server" sys_iface_${iface}_dhcp_nameserver

		# sys_iface_${iface}_dhcp_domain_name
		tip="Most queries for names within this domain can use short names relative to the local domain"
		desc="Allows DHCP hosts to have fully qualified domain names"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
		render_input_field text "Domain" sys_iface_${iface}_dhcp_domain_name

		# sys_iface_${iface}_dhcp_ntpserver
		tip="NTP server for your LAN hosts"
		desc="NTP server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "NTP server" sys_iface_${iface}_dhcp_ntpserver

		tip="WINS server for your LAN hosts"
		desc="WINS server for your LAN hosts (dotted quad)"
		validator=$validator_ipaddr
		render_input_field text "WINS server" sys_iface_${iface}_dhcp_winsserver
	fi
}

render_dhcp_server_static(){

	eval "dhcp_enabled=\$$sys_iface_${iface}_dhcp_enabled"
	if [ "$dhcp_enabled" = 1 ]; then
		render_list_line(){
			local lineno=$1
			eval "val=\"\${sys_iface_${iface}_dhcp_host_${lineno}}\""
			unset name ipaddr hwaddr
			eval "$val"
			echo "<tr><td>$lineno</td><td>$name</td><td>$ipaddr</td><td>$hwaddr</td><td>"
			render_list_btns dhcp_static_edit "sys_iface_${iface}_dhcp_host_${lineno}" "page=${page}&iface=${iface}"
			echo "</td></tr>"
		}
		
		
		render_list_header dhcp_static sys_iface_${iface}_dhcp_host_ "iface=${iface}" "No" "Name" "IP Address" "MAC Address"
		
		eval `kdb -qqc ls sys_iface_${iface}_dhcp_host*`
		render_list_cycle_stuff
	fi
}
