#!/usr/bin/haserl
	
	subsys="pptp"

	kdb_vars="str:svc_pptp_name bool:svc_pptp_enabled str:svc_pptp_localip str:svc_pptp_remoteip str:svc_pptp_pap str:svc_pptp_chap "
	kdb_vars="$kdb_vars str:svc_pptp_mschap str:svc_pptp_mschap2 str:svc_pptp_mppe str:svc_pptp_dns1 str:svc_pptp_dns2 "
	kdb_vars="$kdb_vars str:svc_pptp_wins1 str:svc_pptp_pppdopt "

	render_save_stuff

	eval `kdb -qq ls svc_pptp_*`

	render_form_header pptp_server_common
	render_table_title "PPTP Server"

	# svc_pptp_enabled
	default=0
	tip=""
	desc="Check this item if you want to start PPTP server"
	render_input_field checkbox "Enable PPTP server" svc_pptp_enabled

	# svc_pptp_name
	default="pptpd"
	tip="Name of the local system for authentication purposes (must match the server field in PPP secrets submenu (/etc/ppp/chap-secrets) entries)"
	desc="Name of the local system for authentication purposes <b>required</b>"
	validator=$validator_name
	render_input_field text "Name" svc_pptp_name

	# svc_pptp_localip
	tip="<b>Examples:</b> 192.168.100.1-200, 10.0.0.100-110"
	desc="Local ip address range (dotted quad) <b>required</b>"
	validator="$tmtreq $validator_ipaddr_range"
	render_input_field text "Local IP range" svc_pptp_localip

	# svc_pptp_remoteip
	tip="<b>Examples:</b> 192.168.200.1-200, 10.0.0.200-210"
	desc="Remote ip address range (dotted quad) <b>required</b>"
	validator="$tmtreq $validator_ipaddr_range"
	render_input_field text "Remote IP range"  svc_pptp_remoteip

	# svc_pptp_pap
	default=none
	desc="Password Authentication Protocol"
	render_input_field select "PAP Mode" svc_pptp_pap none "None" require-pap "Require PAP" refuse-pap "Refuse-PAP"

	# svc_pptp_chap
	default=none
	desc="Challenge Handshake Authentication Protocol"
	render_input_field select "CHAP Mode" svc_pptp_chap none "None" require-chap "Require CHAP" refuse-chap "Refuse-CHAP"

	# svc_pptp_mschap
	default=none
	desc="Microsoft Challenge Handshake Authentication Protocol"
	render_input_field select "MS-CHAP Mode" svc_pptp_mschap none "None" require-mschap "Require MS-CHAP" refuse-mschap "Refuse MS-CHAP"

	# svc_pptp_mschap2
	default=none
	desc="Microsoft Challenge Handshake Authentication Protocol, version 2"
	render_input_field select "MS-CHAPv2 Mode" svc_pptp_mschap2 none "None" require-mschap-v2 "Require MS-CHAPv2" refuse-mschap-v2 "Refuse MS-CHAPv2"

	# svc_pptp_mppe
	default=nomppe
	desc="Microsoft Point to Point Encryption"
	validator='tmt:message="Please select mppe mode"'
	render_input_field select "MPPE Mode" svc_pptp_mppe nomppe "Disable MPPE" require-mppe "Require MPPE" #require-mppe-40 "Require MPPE 40"

	# svc_pptp_dns1
	desc="Primary DNS server for clients"
	validator="$validator_ipaddr"
	render_input_field text "DNS Server" svc_pptp_dns1

	# svc_pptp_dns2
	desc="Secondary DNS server for clients"
	validator="$validator_ipaddr"
	render_input_field text "DNS Server" svc_pptp_dns2

	# svc_pptp_wins1
	desc="Windows Internet Name Services server address"
	validator="$validator_ipaddr"
	render_input_field text "WINS Server" svc_pptp_wins1

	# pppd options
	default="nodefaultrote noauth nobsdcomp nodeflate"
	render_input_field text "PPPD Options" svc_pptp_pppdopt
	render_submit_field
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
