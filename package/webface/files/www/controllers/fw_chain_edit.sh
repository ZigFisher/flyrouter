#!/usr/bin/haserl

	subsys="fw"

	table=$FORM_table
	chain=$FORM_chain
	item=$FORM_item

	eval_string="export FORM_$item=\"name=$FORM_name enabled=$FORM_enabled proto=$FORM_proto src=$FORM_src dst=$FORM_dst sport=$FORM_sport dport=$FORM_dport natto=$FORM_natto target=$FORM_target\""
	render_popup_save_stuff
	
	render_form_header fw_chain_edit
	render_table_title "Firewall $table/$chain edit rule" 2
	render_popup_form_stuff
	
	render_input_field hidden table table "$table"
	render_input_field hidden chain chain "$chain"

	# name
	desc="Name of rule"
	validator="$tmtreq $validator_rulename"
	render_input_field text "Short name" name
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled

	# src
	default="0.0.0.0/0"
	srctip="Address can be either a network IP address (with /mask), or a plain IP address, A ! argument before the address specification inverts the sense of the address.<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>"
	tip="$srctip"
	desc="Source address specification"
	validator=$validator_ipnet_ipt
	render_input_field text "Source" src

	# dst
	default="0.0.0.0/0"
	tip="$srctip"
	desc="Destination address specification"
	validator=$validator_ipnet_ipt
	render_input_field text "Destination" dst

	# proto
	tip=""
	desc="The protocol of the rule or of the packet to check"
	validator='tmt:message="Please select lease time"'
	render_input_field select "Protocol" proto all "ALL" tcp "TCP" udp "UDP" icmp "ICMP"

	# sport
	default="any"
	sporttip="An inclusive range can also be specified, using the format <b>port:port</b>. If the first port is omitted, 0 is assumed; if the last is omitted, 65535 is assumed."
	tip="$sporttip"
	desc="Source port or port range specification."
	validator="$tmtreq $validator_ipportrange"
	render_input_field text "Source port" sport

	# dport
	default="any"
	tip="$sporttip"
	desc="Destination port or port range specification."
	validator="$tmtreq $validator_ipportrange"
	render_input_field text "Destination port" dport

	if [ "$table" = "nat" ]; then
		# natto
		id='natto'
		default=""
		tip="You can add port number after ip address<br><b>Example: </b> 192.168.0.1:80"
		desc="Do Source NAT or Destination NAT to address"
		validator="$validator_ipaddrport tmt:natcheck='true'"
		render_input_field text "Nat to address" natto
	fi
	
	# target
	tip=""
	desc=""
	validator=''
	if [ "$table" = filter ]; then
		render_input_field select "Action" target ACCEPT "ACCEPT" DROP "DROP" REJECT "REJECT"
	elif [ "$table" = nat -a "$chain" = "prerouting" ]; then
		require_js_file "tmt_fw_chain_edit.js"
		id='target'
		onchange='OnChange_nat(this);'
		render_input_field select "Action" target ACCEPT "ACCEPT" DROP "DROP" REJECT "REJECT" DNAT "DNAT"
	elif [ "$table" = nat -a "$chain" = "postrouting" ]; then
		require_js_file "tmt_fw_chain_edit.js"
		id='target'
		onchange='OnChange_nat(this);'
		render_input_field select "Action" target ACCEPT "ACCEPT" DROP "DROP" REJECT "REJECT" SNAT "SNAT" MASQUERADE "MASQUERADE"
	fi
	
	render_submit_field
	render_form_tail
	
	
# vim:foldmethod=indent:foldlevel=1
