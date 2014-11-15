#!/usr/bin/haserl

	subsys="adm5120sw"

	eval `kdb -qq ls sys_switch_* `
	for port in $sys_switch_ports; do
		kdb_vars="$kdb_vars str:sys_switch_port${port}_iface str:sys_switch_port${port}_speed str:sys_switch_port${port}_duplex"
	done
	render_save_stuff

	eval `kdb -qq ls sys_switch_* `
	render_form_header

	eth_ifaces=`for i in $sys_switch_ports; do echo "${i} eth${i}"; done`
	for port in $sys_switch_ports; do
		render_table_title "Port $port" 2
		tip="You should reboot router to apply changes"
		desc="Attach port to selected interface"
		render_input_field select "Attach port $port to" sys_switch_port${port}_iface $eth_ifaces
		render_input_field -d select "Speed" sys_switch_port${port}_speed auto 'Auto' 10 '10M' 100 '100M'
		render_input_field -d select "Duplex" sys_switch_port${port}_duplex auto 'Auto' full 'Full' half 'Half'
	done
	render_submit_field
	render_form_tail
# vim:foldmethod=indent:foldlevel=1
