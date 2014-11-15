#!/usr/bin/haserl
	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="bool:sys_ntpclient_enabled str:sys_ntpclient_server str:sys_timezone"
		subsys="time"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi

	eval `$kdb -qq list sys_*`
	render_form_header time time_save
	help_1="begin"
	help_2="time"
	render_table_title "Time settings" 2 

	# sys_ntpclient_enabled
	tip="Time synchronization on boot and each 3 hours"
	desc="Check this item if you want use time synchronizing"
	validator='tmt:required="true" tmt:message="Please input timeserver" tmt:filters="ltrim,rtrim"'
	render_input_field checkbox "Use time synchronizing" sys_ntpclient_enabled

	# sys_ntpclient_server
	tip="Input hostname of time server <br>Example: <b>pool.ntp.org</b>"
	desc="Please input hostname or ip address time server"
	validator="$tmtreq $validator_dnsdomainoripaddr"
	render_input_field text "Time server" sys_ntpclient_server 

	# sys_timezone
	tip="Select time zone<br>Example: <b>GMT</b>, <b>GMT+1</b>, <b>GMT+2</b>"
	desc="Please select timezone"
	validator='tmt:invalidindex=0 tmt:message="Please select timezone"'
	render_input_field select "Time zone" sys_timezone bad "Please select timezone" -12 GMT-12 -11 GMT-11 -10 GMT-10 -9 GMT-9 -8 GMT-8 -7 GMT-7 -6 GMT-6 -5 GMT-5 -4 GMT-4 -3 GMT-3 -2 GMT-2 -1 GMT-1 0 GMT 1 GMT+1 2 GMT+2 3 GMT+3 4 GMT+4 5 GMT+5 6 GMT+6 7 GMT+7 8 GMT+8 9 GMT+9 10 GMT+10 11 GMT+11 12 GMT+12

	render_submit_field
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
