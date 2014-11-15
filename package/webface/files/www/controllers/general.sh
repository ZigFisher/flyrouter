#!/usr/bin/haserl
	
	if [ $REQUEST_METHOD = POST ]; then
		kdb_vars="str:sys_hostname"
		subsys="hostname"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi
	
	eval `$kdb -qq list sys_hostname`
	render_form_header general
	help_1="begin"
	help_2="hostname"
	render_table_title "General settings" 2

	# sys_hostname
	tip="Hostname of router"
	desc="Please enter router's hostname"
	validator='tmt:required="true" tmt:message="Please input name of host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
	render_input_field text "Hostname" sys_hostname 
	
	render_submit_field
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
