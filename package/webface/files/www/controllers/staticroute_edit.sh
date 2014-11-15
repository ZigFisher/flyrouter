#!/usr/bin/haserl

	subsys=""

	item=$FORM_item
	eval_string="export FORM_$item=\"net=$FORM_net netmask=$FORM_netmask gw=$FORM_gw\""

	render_popup_save_stuff
	
	render_form_header sroute_static_edit
	render_table_title "Static route settings" 2
	render_popup_form_stuff
	
	# net
	desc="Network or host (dotted quad) <b>required</b>"
	validator="$tmtreq $validator_ipaddr"
	render_input_field text "Network" net
	
	# netmask
	tip="<h2>Examples:</h2><b>255.255.255.0</b> - /24 - Class <b>C</b> network<br><b>255.255.255.252</b> - /30<br><b>255.255.255.255</b> - /32 for a single host<br>"
	desc="Netmask (dotted quad) <b>required</b>"
	validator="$tmtreq $validator_netmask"
	render_input_field text "Netmask" netmask

	# gw
	desc="Gateway for route  (dotted quad) <b>required</b>"
	validator="$tmtreq $validator_ipaddr"
	render_input_field text "Gateway" gw

	render_submit_field

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
