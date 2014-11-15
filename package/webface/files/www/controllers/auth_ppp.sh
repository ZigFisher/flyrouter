#!/usr/bin/haserl

	page=${FORM_page:-chap}
	auth_mode=${page}

	subsys="auth_ppp"
	
	handle_list_del_item
	eval `kdb -qq ls "sys_auth_*" `

	render_page_selection "" chap CHAP pap PAP 
	render_form_header "${controller}_${auth_mode}"
	render_list_line(){
		local lineno=$1
		eval "val=\"\${sys_auth_ppp_${auth_mode}_${lineno}}\""
		unset username server password ipaddr
		eval "$val"
		echo "<tr><td>$lineno</td><td>$username</td><td>"'*******'"</td><td>$server</td><td>"
		render_list_btns auth_ppp_edit "sys_auth_ppp_${auth_mode}_${lineno}" "page=${page}&auth_mode=${auth_mode}"
		echo "</td></tr>"
	}

	render_list_header auth_ppp sys_auth_ppp_${auth_mode}_ "auth_mode=${auth_mode}" "No" "Username" "Password" "Server"
	eval `$kdb -qqc list sys_auth_ppp_${auth_mode}*`

	render_list_cycle_stuff

	render_form_tail

# vim:foldmethod=indent:foldlevel=1
