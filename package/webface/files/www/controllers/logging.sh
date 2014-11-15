#!/bin/sh
	
	subsys="logging"

	kdb_vars="int:sys_log_console int:sys_log_dmesg_level int:sys_log_buf_size bool:sys_log_remote_enabled str:sys_log_remote_server "

	render_save_stuff

	eval `kdb -qq ls sys_log_*`

	render_form_header logging
	render_table_title "Logging"

	# sys_log_console
	default=1
	tip=""
	desc=""
	render_input_field select "Console priority logging" sys_log_console 0 0 1 1 2 2 3 3 4 4 5 5 6 6 7 7

	# sys_log_dmesg_level
	default=1
	tip="For example: 1 prevents all messages, expect panic messages"
	desc="Set the level at which logging of messages is done to the console."
	render_input_field select "Kernel console priority logging" sys_log_dmesg_level 1 1 2 2 3 3 4 4 5 5 6 6 7 7

	# sys_log_buf_size
	default=64
	desc="Circular buffer size"
	render_input_field select "Circular buffer" sys_log_buf_size 0 0k 8 8k 16 16k 32 32k 64 64k 128 128k 256 256k 512 512k 

	# sys_log_remote_enabled
	default=0
	tip=""
	desc="Check this item if you want to enable remote logging"
	render_input_field checkbox "Enable remote syslog logging" sys_log_remote_enabled

	# sys_log_remote_server
	desc="Domain name or ip address of remote syslog server"
	validator="$validator_dnsdomainoripaddr"
	render_input_field text "Remote syslog server" sys_log_remote_server

	render_submit_field
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
