#!/usr/bin/haserl

	kdb_vars="str:sys_dns_nameserver str:sys_dns_domain"
	subsys="dns"

	render_save_stuff

	eval `kdb -qq ls sys_*`
	render_form_header dns
        help_1="begin"
	help_2="dns"
	render_table_title "DNS Settings" 2 

	# sys_dns_nameserver
	#tip="Dns server for your router"
	desc="Please enter ip address of upstream dns server"
	validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"  tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
	render_input_field text "Upstream server" sys_dns_nameserver

	# sys_dns_domain
	#tip="Domain for your net"
	desc="Please enter your domain"
	validator=''
	render_input_field text "Domain" sys_dns_domain

	render_submit_field
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
