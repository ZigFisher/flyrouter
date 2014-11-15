#!/usr/bin/haserl

	subsys="fw"
	table=$FORM_table
	chain=$FORM_chain

	cautiontip="<b class=red>WARNING:</b> Use with caution!<br> Without any ACCEPT rule, default DROP policy can brick your router"
	if [ -z "$table" ]; then
		kdb_vars="bool:sys_fw_enabled"

		render_save_stuff

		eval `kdb -qq ls sys_fw_*`
		render_form_header fw 
		render_table_title "Firewall settings" 

		# sys_fw_enabled
		tip=""
		desc="Check this item if you want use firewall on your router"
		validator='tmt:required="true"'
		render_input_field checkbox "Enable Firewall" sys_fw_enabled

		render_submit_field
		render_form_tail
	elif [ "$table" = "filter" ]; then
	
		kdb_vars="str:sys_fw_filter_policy_forward str:sys_fw_filter_policy_input str:sys_fw_filter_policy_output"
		render_save_stuff
		
		eval `kdb -qq ls sys_fw_filter_*`
		render_form_header fw_policy 
		
		render_input_field hidden table table "$table"
		render_input_field hidden chain chain "$chain"
		
		help_2="fw_policy"
		render_table_title "Default policy" 2 
		# forward policy
		autosubmit="y"
		render_input_field select "Default policy for FORWARD" sys_fw_filter_policy_forward ACCEPT "ACCEPT" DROP "DROP"
		# input policy
		autosubmit="y"
		tip=$cautiontip
		render_input_field select "Default policy for INPUT" sys_fw_filter_policy_input ACCEPT "ACCEPT" DROP "DROP"
		# output policy
		autosubmit="y"
		tip=$cautiontip
		render_input_field select "Default policy for OUTPUT" sys_fw_filter_policy_output ACCEPT "ACCEPT" DROP "DROP"
		render_form_tail

		# fw forward list
		render_form_header fw_filter_forward
		help_2="fw_forward"
		render_table_title "FORWARD" 2 
		render_iframe_list "fw_chain" "table=filter&chain=forward"
		render_form_tail

		# fw input list
		render_form_header fw_filter_input
		help_2="fw_input"
		render_table_title "INPUT" 2 
		render_iframe_list "fw_chain" "table=filter&chain=input"
		render_form_tail

		# fw output list
		render_form_header fw_filter_output
		help_2="fw_output"
		render_table_title "OUTPUT" 2 
		render_iframe_list "fw_chain" "table=filter&chain=output"
		render_form_tail
	
	elif [ "$table" = "nat" ]; then
		kdb_vars="str:sys_fw_nat_policy_prerouting str:sys_fw_nat_policy_postrouting"
		render_save_stuff
		
		eval `kdb -qq ls sys_fw_nat_*`
		render_form_header fw_policy 
		
		render_input_field hidden table table "$table"
		render_input_field hidden chain chain "$chain"
		
		help_1="nat"
		render_table_title "Default policy"
		# prerouting policy
		autosubmit="y"
		tip=$cautiontip
		render_input_field select "Default policy for PREROUTING" sys_fw_nat_policy_prerouting ACCEPT "ACCEPT" DROP "DROP"
		# postrouting policy
		autosubmit="y"
		tip=$cautiontip
		render_input_field select "Default policy for POSTROUTING" sys_fw_nat_policy_postrouting ACCEPT "ACCEPT" DROP "DROP"
		render_form_tail

		# fw prerouting list
		render_form_header fw_nat_prerouting
		help_1="nat"
		help_2="nat_prerouting"
		render_table_title "PREROUTING" 2 
		render_iframe_list "fw_chain" "table=nat&chain=prerouting"
		render_form_tail
		
		# fw postrouting list
		render_form_header fw_nat_postrouting
		help_1="nat"
		help_2="nat_postrouting"
		render_table_title "POSTROUTING" 2 
		render_iframe_list "fw_chain" "table=nat&chain=postrouting"
		render_form_tail
		
	fi



# vim:foldmethod=indent:foldlevel=1
