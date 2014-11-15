#!/usr/bin/haserl
	. lib/misc.sh
	
	frame=1

	table=$FORM_table
	
	handle_list_del_item "ipsec"	# restart ipsec on item deletion
	
	eval `$kdb -qqc list sys_ipsec_${table}_*`
	render_form_header ipsec_${table}

	
	render_list_line(){
		local lineno=$1
		local item="sys_ipsec_${table}_${lineno}"
		local target_img="<img src=img/blank.gif>"
		local style
		eval "var=\$$item"
		eval "$var"
		[  "x${enabled}x" = "xx"  ] && style="class='lineDisabled'"
		
		case $table in
		sad)	echo "<tr $style><td>$spi</td><td>$src</td><td>$dst</td><td>$ah_alg<br>$ah_key</td><td>$esp_alg<br>$esp_key</td><td>";;
		spd)	echo "<tr $style><td>$src</td><td>$dst</td><td>$upperspec</td><td>$direction</td><td>$policy</td><td>$mode</td><td>$level</td><td>";;
		esac
		render_list_btns ipsec_table_edit "$item" "table=$table"
		echo '</td></tr>'
	}

	
	case $table in
	sad)	render_list_header ipsec_table sys_ipsec_${table}_ "table=$table" "SPI" "Src" "Dst" "AH" "ESP";;
	spd)	render_list_header ipsec_table sys_ipsec_${table}_ "table=$table" "Src" "Dst" "Upper-Level" "Direction" "Policy" "Proto" "Level";;
	esac
	
	
	render_list_cycle_stuff

	render_form_tail
# vim:foldmethod=indent:foldlevel=1
