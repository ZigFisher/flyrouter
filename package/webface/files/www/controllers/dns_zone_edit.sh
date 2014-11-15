#!/usr/bin/haserl

	#subsys="dns_server"

	if [ -n "$FORM_datatype" ]; then
		[ "$FORM_datatype" = "MX" -a -z "$FORM_prio" ]  && FORM_prio="10"
		[ "$FORM_datatype" != "MX" ]  && FORM_prio=""
	fi
	
	item=$FORM_item
	eval_string="export FORM_$item=\"domain=$FORM_domain datatype=$FORM_datatype prio=$FORM_prio data=$FORM_data\""
	render_popup_save_stuff
	
	render_form_header dns_zonelist_edit
	render_table_title "DNS Zone options" 2
	render_popup_form_stuff
	
	# domain
	tip="<b>Tip:</b> Use @ for current zone"
	desc="Domain"
	validator="$tmtreq $validator_dnsdomain"
	render_input_field text "Domain or host" domain
	
	# datatype
	desc="Select type of record: A, NS,CNAME, MX, PTR, TXT" # spaces in TXT records currently is not supported
	validator='tmt:message="Please select type of record"'
	render_input_field select "Type of record" datatype A "A" CNAME "CNAME" MX "MX" NS "NS" PTR "PTR" TXT "TXT"

	# prio
	desc="Priority used only on MX records"
	validator=$validator_mxprio
	render_input_field text "Priority" prio

	# data
	tip="If the record points to an EXTERNAL server (not defined in this zone) it MUST end with a <b>.</b> (dot) e.g. ns1.example.net. If the name server is defined in this domain (in this zone file) it can be written as ns1 (without the dot)"
	desc="Please input data for this record"
	validator="$tmtreq $validator_dnsdomainoripaddr"
	render_input_field text "Data" data

	render_submit_field

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
