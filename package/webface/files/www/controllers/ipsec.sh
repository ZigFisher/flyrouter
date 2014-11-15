#!/usr/bin/haserl

	page=${FORM_page:-status}
	subsys="ipsec"
	setkey=/usr/sbin/setkey

	handle_list_del_item
		
	eval `kdb -qq ls sys_ipsec*`
	
	render_page_selection "" status "Status" sad "Security Association" spd "Security Policy" random "Random keys" 
	
	render_form_header

	case $page in 
	'status')
		render_console_start "Security Association Database" 2 
		render_console_command $setkey -D
		render_console_end
		render_console_start "Security Policy Database" 2 
		render_console_command $setkey -DP
		render_console_end
		;;
	'sad')
		render_table_title "Security Association Database" 2 
		render_iframe_list "ipsec_table" "table=sad"
		;;
	'spd')
		render_table_title "Security Policy Database" 2 
		render_iframe_list "ipsec_table" "table=spd"
		;;
	'random')
		render_console_start "Random 64 bit key" 2 
		dd if=/dev/urandom bs=1 count=8 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 96 bit key" 2 
		dd if=/dev/urandom bs=1 count=12 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 128 bit key" 2 
		dd if=/dev/urandom bs=1 count=16 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 192 bit key" 2 
		dd if=/dev/urandom bs=1 count=24 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end

		render_console_start "Random 256 bit key" 2 
		dd if=/dev/urandom bs=1 count=32 2>/dev/null | hexdump -e "16/2 \"%04x\"\"\\n\""
		render_console_end
		;;
	esac
	
	render_form_tail
# vim:foldmethod=indent:foldlevel=1
