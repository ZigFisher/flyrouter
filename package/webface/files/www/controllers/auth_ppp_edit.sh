#!/usr/bin/haserl

	auth_mode=${FORM_auth_mode:-chap}
	subsys="auth_ppp"

	item=$FORM_item
	eval_string="export FORM_$item=\"username=$FORM_username password=$FORM_password server=$FORM_server ipaddr=$FORM_ipaddr\""

	render_popup_save_stuff
	
	render_form_header $controller
	render_table_title "PPP $auth_mode secrets"
	render_popup_form_stuff
	
	# username
	tip="Case sensitive"
	desc="Username for secrets file <b>required</b>"
	validator="$tmtreq $validator_name"
	render_input_field text "Username" username

	# password
	desc="Password for secrets file <b>required</b>"
	validator="$tmtreq $validator_password"
	render_input_field text "Password" password
	
	# server
	tip=" * - matches any name. Case sensitive."
	default='*'
	desc="Server name for secrets file <b>required</b>"
	validator="$tmtreq $validator_name"
	render_input_field text "Server" server

	# ipaddr
	tip="* - matches any ip address."
	default='*'
	desc="IP address for secrets file <b>required</b>"
	validator="$tmtreq $validator_ipaddr_asterisk"
	render_input_field text "IP address" ipaddr

	render_submit_field

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
