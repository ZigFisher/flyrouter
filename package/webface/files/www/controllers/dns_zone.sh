#!/usr/bin/haserl
	
	zoneid=$FORM_zoneid
	
	handle_list_del_item
	
	eval `kdb -qqc ls svc_dns_zone_${zoneid}*`
	render_form_header dns_zone

	render_list_line(){
		local lineno=$1
		local item="svc_dns_zone_${zoneid}_${lineno}"
		eval "val=\"\$${item}\""
		eval "$val"
		[ "$datatype" != "MX" ] && prio=""
		echo "<tr><td>$lineno</td><td>$domain</td><td>$datatype</td><td>$prio</td><td>$data</td><td>"
		render_list_btns dns_zone_edit "$item" "zoneid=$zoneid"
		echo "</td></tr>"
	}
	
	
	render_list_header dns_zone svc_dns_zone_${zoneid}_ "" "No" "Domain" "Type" "Priority" "Data"
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
