#!/usr/bin/haserl
	
	page=${FORM_page:-kdb}
	name=${FORM_name}
	value=${FORM_value}
	
	render_page_selection "" kdb "kdb" kdb_set 'kdb set'


	render_form_header 
	if [ $REQUEST_METHOD = POST ]; then
		case $page in
		kdb_set)
			kdb set "$name"="$value"
			render_console_start "kdb"
			render_console_command kdb ls $name
			render_console_end
		;;
		esac
	else 
		render_table_title "$page" 2 
		render_input_field hidden page page "$page"
		case $page in
			kdb)
				render_console_start ""
				render_console_command kdb ls
				render_console_end
			;;
			kdb_set)
				value="`kdb -q get $name`"
				validator='tmt:required="true" tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic" tmt:message="Please input name" '
				render_input_field text "Name" name
				validator='tmt:filters="ltrim,rtrim,nohtml,nocommas,nomagic"'
				render_input_field text "Value" value
				render_submit_field "Set"
		esac
	fi
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
