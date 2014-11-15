#!/usr/bin/haserl

	iface_proto=$FORM_iface_proto
	del_iface=$FORM_del_iface

	eval `kdb -qq ls sys_ifaces`

	get_next_iface() {
		local n=0
		local name
		local notfound
		case $1 in
			pptp|pppoe) name=$1;;
			bonding)	name=bond;;
			bridge)		name=br;;
		esac

		while [ 1 ]; do
			notfound=y
			for i in $sys_ifaces; do
				if [ ${name}${n} = $i ]; then notfound=n; break; fi
			done
			if [ $notfound = y ]; then
				echo ${name}${n}
				break;
			else
				n=$(($n+1))
			fi
		done
	}

	if [ $REQUEST_METHOD = POST ]; then
		if [ -n "$iface_proto" ]; then
			iface=`get_next_iface $iface_proto`
			iface_add $iface
			ok_str="Interface <b>$iface</b> added, please reload page"
			render_save_message
		elif [ -n "$del_iface" ]; then
			iface_del $del_iface
			del_iface=""
			ok_str="Interface deleted, please reload page"
			render_save_message
		fi
	fi

	# Add interface
	render_form_header ifaces
	render_table_title "Add dynamic interface" 2

	desc="Please select interface protocol"
	validator='tmt:invalidindex=0 tmt:message="Please select protocol"'
	render_input_field select "Protocol" iface_proto bad "Please select interface protocol" bridge Bridge pppoe PPPoE pptp PPtP  bonding Bonding # ipsec IPSec

	render_submit_field Add
	render_form_tail

	# Delete interface
	render_form_header ifaces
	render_table_title "Delete dynamic interface" 2

	desc="Please select interface"
	validator='tmt:invalidindex=0 tmt:message="Please select interface"'
	params=""
	for i in $sys_ifaces; do
		case $i in
			eth*|dsl*)	;;
			*)		params="$params $i $i";;
		esac
	done
	render_input_field select "Interface" del_iface bad "Please select interface" $params

	render_submit_field Delete
	render_form_tail
# vim:foldmethod=indent:foldlevel=1
