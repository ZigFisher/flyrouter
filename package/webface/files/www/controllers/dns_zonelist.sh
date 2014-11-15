#!/usr/bin/haserl
	
	frame=1

	handle_list_del_item
	
	eval `$kdb -qqc list svc_dns_zonelist*`
	render_form_header dns_zonelist

	render_list_line(){
		local lineno=$1
		local item="svc_dns_zonelist_${lineno}"
		eval "val=\"\${svc_dns_zonelist_${lineno}}\""
		eval "$val"
		enabled_img="<img src=img/disabled.gif>"
		[  "x${enabled}x" != "xx"  ] && enabled_img="<img src=img/enabled.gif>"
		local href="/?controller=dns_zone&zoneid=$zoneid"
		echo "<tr><td>$enabled_img <a href='$href' target=_parent>$zoneid</a></td><td><a href='$href' target=_parent>$zone</a></td><td>$admin</td><td>$serial</td><td>"
		render_list_btns dns_zonelist_edit "$item"
		echo "</td></tr>"
	}
	
	
	render_list_header dns_zonelist svc_dns_zonelist_ "" "Zone id" "Zone name" "Admin" "Serial"
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
