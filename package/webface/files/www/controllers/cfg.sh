#!/usr/bin/haserl

	page=${FORM_page:-backup}
	
	render_page_selection "" backup "backup" restore "restore" default "default"

	render_form_header backup 'action="/cfg.cgi"' 'enctype="multipart/form-data"'
	render_table_title "$page" 2 
	render_input_field hidden act act "$page"

	case $page in
	backup)
		render_submit_field "Backup"
	;;
	restore)
		desc="Restore configuration from file"
		render_input_field file "Restore configuration" "uploadfile"
		render_submit_field "Restore"
	;;
	default)
		render_submit_field "Restore default"
	;;
	esac
	render_form_tail

# vim:foldmethod=indent:foldlevel=1
