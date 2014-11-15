#!/usr/bin/haserl

	page=${FORM_page:-ping}
	host=${FORM_host:-localhost}
	lines=${FORM_lines:-40}
	count=${FORM_count:-5}
	sleep=${FORM_sleep:-2}
	iface=${FORM_iface:-eth0}
	tcpdump_arg=${FORM_tcpdump_arg}
	query=${FORM_query}
	type=${FORM_type}
	
	render_page_selection "" syslog "syslog" dmesg "dmesg" ping "ping" mtr "mtr" dig "dig" tcpdump "tcpdump" reboot "reboot" 

	render_form_header 
	if [ $REQUEST_METHOD = POST ]; then
		#render_table_title "$page" 2 
		resolve="-n"
		count="-c $count"
		packetsize="-s 100"
		case $page in
		ping)
			render_console_start "$page"
			render_console_command ping $count $packetsize $host
			render_console_end
		;;
		mtr)
			render_console_start "$page"
			render_console_command /usr/sbin/mtr -r $resolve $count $packetsize $host
			render_console_end
		;;
		dig)
			server=""
			[ -n "FORM_server" ] && server="@${FORM_server}"
			render_console_start "$page"
			render_console_command /usr/bin/dig $query $server
			render_console_end
		;;
		tcpdump)
			render_console_start "$page"
			/usr/sbin/tcpdump -i $iface $resolve $tcpdump_arg & pid=${!}; sleep $sleep; kill $pid
			render_console_end
		;;
		reboot)
			render_console_start "$page"
			echo "Reboot in progress..."
			( sleep 3; reboot )&
			render_console_end
		;;
		esac

	else 
		render_table_title "$page" 2 
		render_input_field hidden page page "$page"
		case $page in
			ping|mtr)
				validator='tmt:required="true" tmt:message="Please input destination host" tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic"'
				render_input_field text "Host" host 
				default="5"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=50  tmt:message="Please input number"'
				render_input_field text "Count" count
				render_submit_field "Run"
			;;
			tcpdump)
				ifaces=`/bin/ip -o link | cut -f2 -d":" | while read i; do echo -n "$i $i "; done`
				desc="Listen on interface"
				render_input_field select "Interface" iface $ifaces
				
				tip="<b>Examples:</b><br><ol> <li>[src|dst] host 192.168.0.1<li>ether host 02:ff:1c:d9:9b:f8<li> [src|dst] net 192.168.1.0/24"
				desc="Selects which packets will be dumped"
				validator='tmt:filters="nohtml,nocommas,nomagic"'
				render_input_field text "Expression" tcpdump_arg
				
				default=2
				desc="seconds"
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="positiveinteger" tmt:minnumber=1 tmt:maxnumber=50  tmt:message="Please input number"'
				render_input_field text "Time" sleep
				render_submit_field "Run"
			;;
			dig)
				render_input_field select "Type" type any any soa soa mx mx a a	
				render_input_field text "Query" query
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:pattern="dnsdomainoripaddr"'
				tip=" This can be an IPv4 address in dotted-decimal notation. If no server argument is provided, dig consults /etc/resolv.conf and queries the name servers listed there."
				desc="The name or IP address of the name server to query. "
				render_input_field text "Server" server
				render_submit_field "Run"
			;;
			syslog)
				render_console_start
				render_console_command /sbin/logread | tail -$lines | colorizelog
				render_console_end
			;;
			dmesg)
				render_console_start
				render_console_command /bin/dmesg | tail -$lines
				render_console_end
			;;
			reboot)
				render_submit_field "Reboot"
			;;
		esac
	fi
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
