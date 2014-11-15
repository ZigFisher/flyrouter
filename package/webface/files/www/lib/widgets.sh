#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

tmtreq='tmt:required=true '
validator_ipaddr='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address" tmt:pattern="ipaddr"'
validator_ipaddr_asterisk='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address, or *" tmt:pattern="ipaddr_asterisk"'
validator_ipaddr_range='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip address range, like 192.168.1.100-200" tmt:pattern="ipaddr_range"'
validator_netmask='tmt:required=true tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct ip netmask" tmt:pattern="netmask"'
validator_dnsdomain='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns domain name" tmt:pattern="dnsdomain"'
validator_dnsdomainoripaddr=' tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct dns domain name" tmt:pattern="dnsdomainoripaddr"'
validator_mxprio='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input priority" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=999'
validator_dnszone='tmt:filters="ltrim,rtrim,nohtml,nospaces,nodots,noquotes,nodoublequotes,nocommas,nomagic" tmt:message="Please input zone identifier" tmt:pattern="dnszone"'
validator_email='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct email" tmt:pattern="email"'
validator_refresh='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input refresh time" tmt:pattern="positiveinteger" tmt:minnumber=1200 tmt:maxnumber=500000'
validator_ttl='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input ttl time" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=500000'
validator_retry='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input retry time" tmt:pattern="positiveinteger" tmt:minnumber=180 tmt:maxnumber=20000'
validator_expire='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input expire time" tmt:pattern="positiveinteger" tmt:minnumber=10000 tmt:maxnumber=90000000'
validator_rulename='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct rule name" tmt:pattern="A_z0_9"'
validator_ipnet='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address x.x.x.x/y" tmt:pattern="ipnet"'
validator_ipnet_ipt='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address [!]x.x.x.x/y" tmt:pattern="ipnet_ipt"'
validator_ipportrange='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct port" tmt:pattern="ipportrange"'
validator_ipaddrport='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address" tmt:pattern="ipaddrport"'
validator_password='tmt:required="true" tmt:message="Please input password" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
validator_name='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct name" tmt:pattern="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,nobackquotes"'
validator_macaddr='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic" tmt:message="Please input correct mac address" tmt:pattern="macaddr"'
validator_spi='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct SPI value" tmt:pattern="ipsec_spi"'
validator_ipseckey='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct key" tmt:pattern="ipsec_key"'
validator_ifacelist='tmt:required="true" tmt:message="Please input interfaces" tmt:filters="ltrim,nohtml,nocommas,nomagic"'

render_chart_h(){
	local t="$1";
	local l="$2";
	local f="$3";
	local p=$(($l*100/$f))
	local w=$(($p*2))
	echo '<img src=/img/bar_left.gif><img src=/img/bar_middle.gif height=16 width='$w'><img src=/img/bar_right.gif>' $p '%'
}

render_table_title(){
	local text="$1"
	local lhelp_1 lhelp_2;
	[ -n "$help_1" ] && lhelp_1="$help_1" || lhelp_1="${controller}"
	[ -n "$help_2" ] && lhelp_2="$help_2" || lhelp_2="${page}"
	echo "<tr><td></td></tr>"
	echo "<tr><td class='table_title' colspan=2><table border=0> <tr class='table_title' ><td width=100%>$text </td>"
	echo "<td><a class='help_lnk' href='javascript:openHelp(\"${lhelp_1}\", \"${lhelp_2}\");'><img src='/img/help2.png' border=0></a></td></tr></table>"
	echo "</td></tr>"
	unset help_1 help_2
}


render_console_start(){
	local text="$1"
	local colspan;
	[ -n "$2" ] && colspan="colspan='$2'"
	echo "<tr><td></td></tr>"
	[ -n "$text" ] && echo "<tr class='table_title'> <td $colspan class='table_title'>$text</td> </tr>"
	echo "<tr><td><pre class='console'>"
}

render_console_end(){
	echo "</pre></td></tr>"
	#echo "</table>"
}

render_console_command(){
	local cmd="$*"
	echo "<b>$cmd</b>"
	$cmd
}

render_console(){ 
	render_console_start $*
	render_console_command $*
	render_console_end $*
}

render_message_box() 
{
	local title="$1"
	local text="$2"
	echo "<table>"
	echo "<tr class='table_title'> <td class='table_title'>$title</td> </tr>"
	echo "<tr><td>$text</td></tr></table>"
}

render_save_message(){
	local title;
	echo "<table id='message' width='100%' border='0' cellspacing='0' cellpadding='0'>"
	[ -z "$ERROR_MESSAGE" ] && title="Information" || title="Error"
	echo "<tr><td></td></tr>"
	echo "<tr class='table_title'> <td class='table_title'>$title</td> </tr>"
	
	echo "<tr><td width='100%' class='listr' align='center'>" 
	[ -z "$ERROR_MESSAGE" ] && echo $ok_str || echo "$ERROR_MESSAGE <br> $fail_str"

	echo "</td></tr><tr><td> <br /> <br /></td></tr></table>"
	render_js_hide_message
}

render_form_header(){
	local lname="midge_form"
	local lmethod=${form_method:-POST}
	if [ -n "$1" ]; then lname="$1"; shift; fi

	echo "<form name='$lname' method='$lmethod' tmt:validate='true' tmt:callback='displayError' $* >"
	#echo "<input type=hidden name=SESSIONID value='$SESSIONID'>"
	echo "<input type=hidden name=controller value='$controller'>"
	echo "<table width='100%' border='0' cellspacing='0' cellpadding='0'>"
	unset form_method
}

render_input_field(){
	# options handling
	if [ "x$1" = "x-d" ]; then
		local disabled="disabled='true'"
		shift;
	else
		local disabled=''
	fi
	
	# parameters handling
	local type="$1"
	local text="$2"
	local inputname="$3"
	local inputsize='25'
	local maxlenght='255'
	local idcode=''
	local tipcode=''
	local onchangecode=''
	local i
	#local helpcode="<a href='javascript:openHelp(\"${controller}\", \"${page}\");'><img src='/img/help.gif' border=0 align='right' valign='middle'></a>"
	[ -n "$tip" ] && tipcode="onmouseover=\"return overlib('$tip', BUBBLE, BUBBLETYPE, 'roundcorners')\" onmouseout=\"return nd();\""
	eval 'value=$'$inputname
	[ -z "$value" -a -n "$default" ] && value="$default"
	[ -n "$id" ] && idcode="id='$id'"
	[ -n "$autosubmit" ] && ascode="onchange='this.form.submit()'"
	[ -n "$onchange" ] && onchangecode="onchange='$onchange'"
	
	shift 3

	echo "
<!-- ------- render_input_field $type $text $inputname $* -->"

	[ ! "$type" = "hidden" ] && echo "<tr>
<td width='35%' class='vncellt'><label for='$inputname' $tipcode>$text</label></td>
<td width='65%' class='listr'>";

	case "${type}" in
	text)
		echo "	<input $disabled type='text' class='edit' $idcode $tipcode $onchangecode name='$inputname' size='$inputsize' maxlength='$maxlenght' $validator tmt:errorclass='invalid' value='$value'> "
		;;
	checkbox)
		echo -n "	<input $disabled type='checkbox' class='edit' $idcode $tipcode $onchangecode name='$inputname' $validator tmt:errorclass='invalid'"
		for i in ${value%%0}; do echo -n " checked=1 "; done
		echo '> '
		;;
	radio)
		while [ -n "$1" ]; do
			echo -n "<label $tipcode><input $disabled type='radio' class='button' $idcode $tipcode $onchangecode name='$inputname' $validator tmt:errorclass='invalid'"
			[ "$value" = "$1" ] && echo -n " checked "
			echo "value='$1'>$2</label><br>"
			validator=""
			shift 2
		done
		;;
	select)
		echo -n "<select $disabled $tipcode name='$inputname' class='edit' $idcode $validator $onchangecode tmt:errorclass='invalid' $ascode>"
		while [ -n "$1" ]; do
			echo -n "<option value=$1"
			[ "$value" = "$1" ] && echo -n " selected "
			echo ">$2</option>"
			shift 2
		done
		echo "</select>"
		;;
	hidden)
		value="$1"
		echo "<input type='hidden' name='$inputname' value='$value'>"
		;;
	password)
		echo "	<input $disabled type='password' class='edit' $idcode $tipcode $onchangecode name='$inputname' size='$inputsize' maxlength='$maxlenght' $validator tmt:errorclass='invalid' value='$value'> "
		;;
	static)
		echo "$@"
		;;
	file)
		echo "<input type=file name=$inputname>"
		;;
	esac
	
	[ ! $type = "hidden" ] && echo "<br><span class='inputDesc' $tipcode>$desc</span></td></tr>"
	echo "<!-- ------- /render_input_field $type $text $inputname $* -->"
	unset id autosubmit onchange onmouseover tip desc validator default
}

render_submit_field(){
	local btn="Save";
	[ -n "$1" ] && btn="$1"
	echo "<tr> <td colspan=2 style='text-align: center;'> <input class='button' type='submit' name='submit' value='$btn'> </td> </tr>";
}

render_form_tail(){
	echo "</table> <!-- /fieldset--> </form><br/><br/>";
}

render_form_note(){
	local note="$1"
	echo "<tr> <td colspan=2> <span class='inputDesc'><b>Note:</b> $note </span> </td></tr>";
}

render_iframe_list(){
	local controller="$1"
	local params="$2"
	echo "<tr><td><iframe name=$controller src='/?controller=$controller&frame=1&$params' frameborder=1 width='$IFRAME_WIDTH' height='$IFRAME_HEIGHT' scrolling='auto'></iframe></td></tr>"
}

render_list_header(){
		local controller="$1"
		local item="$2"
		local extparam="&$3"

		shift 3
		local s1="<tr>"
		local s2="<td class='table_title'>"
		local s2_act="<td>"
		local s3="</td>"
		local s4="</tr>"
		
		echo $s1
		for n in "$@"; do
			echo $s2 $n $s3
		done
		#echo $s2_act $s3 $s4
		echo "<td align='left'>&nbsp;<a href='javascript:openPopup(window, \"${controller}_edit&${extparam}\", \"$item\", \"additem=1\");'><img src='img/plus.gif' title='Add item' width='17' height='17' border='0'></a></td>"$s4
}

render_list_cycle_stuff(){
	local i=0
	while [ $i -lt $kdb_lines_count ]; do
		render_list_line $i
		i=$(($i+1))
	done
}

render_list_btns(){
		local edit=$1
		local item=$2
		local extparam="&$3"
		local frameparam=""
		[ -n "$frame" ] && frameparam="&frame=1"
		
		# edit
		echo "&nbsp;<a href='javascript:openPopup(window, \"${edit}${extparam}\", \"$item\");'><img src='img/e.gif' title='Edit item' width='17' height='17' border='0'></a>"
		
		# del
		echo "<a href='/?controller=${FORM_controller}&do=del&item=${item}${frameparam}${extparam}' target='_self' onclick='return confirmSubmit()'><img src='img/x.gif' title='Delete item' width='17' height='17' border='0'></a>"
	
}

render_button_list_add(){
	local controller="$1"
	local item="$2"
	local extparam="&$3"
	echo "<tr><td align='center'><a href='javascript:openPopup(window, \"${controller}_edit&${extparam}\", \"$item\", \"additem=1\");'><img src='img/plus.gif' title='Add item' width='17' height='17' border='0'>Add item</a></td></tr>"
}

render_popup_save_stuff(){
	if [ "$REQUEST_METHOD" = POST ]; then
		eval $eval_string
		if [ -z "$FORM_additem" ]; then
			eval `$kdb -qqc list "$item"`
			save "$subsys" "str:$item" 
		else
			ok_str="Item added"
			kdb_ladd_string $item
			kdb_commit
			update_configs_and_service_reload "$subsys"
		fi
		render_save_message
		[ -z "$DEBUG" ] && render_js_close_popup
		render_js_refresh_parent
	fi

	eval `$kdb -qqc list "$item"`
	[ -z "$FORM_additem" ] && eval "export \$${item}"
}

render_popup_form_stuff(){
	render_input_field hidden item item $item
	render_input_field hidden popup popup 1
	[ -n "$FORM_additem" ] && render_input_field hidden additem additem 1
}

render_save_stuff(){
	if [ $REQUEST_METHOD = POST ]; then
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi
}

render_js_close_popup() {
	local timeout=${1:-3000}
	echo "<script language=\"JavaScript\">setTimeout('window.close()',$timeout);</script>"
}

render_js_refresh_parent() {
	local location="$1"
	[ -z "$location" ] && location="window.opener.location"	
	echo "<script language=\"JavaScript\">window.opener.location = $location</script>"
}

render_js_refresh_window() {
	local timeout=${1:-4000}
	echo "<script language=\"JavaScript\">setTimeout('window.location = window.location', $timeout);</script>"
}

render_js_hide_message(){
	local timeout=${1:-1500}
	echo "<script language=\"JavaScript\">setTimeout('document.getElementById(\"message\").style.display = \"none\";', $timeout);</script>"
}

render_page_selection(){
	local class;
	extparam=$1
	shift
	echo "<table class='page_select'><tr>"
	while [ -n "$1" ]; do
		class="pagesel"
		[ "$page" = "$1" ] && class="pagesel_a"
		echo -n "<td class='$class'><a class='$class' href='/?controller=${controller}&page=$1&$extparam'>$2</a></td>"
		shift 2
	done
	echo "</tr></table>"
}

require_js_file() {
	echo "<script language=\"JavaScript\" src=\"js/$1\"></script>"
}

run_js_code() {
	echo "<script language=\"JavaScript\">$*</script>"
}
