#!/usr/bin/haserl

require_js_file "prototype.js"
require_js_file "dsl.js"
tmp_rates="64 64 128 128 192 192 256 256 320 320 384 384 448 448 512 512 576 576 640 640 704 704 768 768 832 832 896 896 960 960 1024 1024 1088 1088 1152 1152 1216 1216 1280 1280 1344 1344 1408 1408 1472 1472 1536 1536 1600 1600 1664 1664 1728 1728 1792 1792 1856 1856 1920 1920 1984 1984 2048 2048 2112 2112 2176 2176 2240 2240 2304 2304 2368 2368 2432 2432 2496 2496 2560 2560 2624 2624 2688 2688 2752 2752 2816 2816 2880 2880 2944 2944 3008 3008 3072 3072 3136 3136 3200 3200 3264 3264 3328 3328 3392 3392 3456 3456 3520 3520 3584 3584 3648 3648 3712 3712 3776 3776 3840 3840 3904 3904 3968 3968 4032 4032 4096 4096 4160 4160 4224 4224 4288 4288 4352 4352 4416 4416 4480 4480 4544 4544 4608 4608 4672 4672 4736 4736 4800 4800 4864 4864 4928 4928 4992 4992 5056 5056 5120 5120 5184 5184 5248 5248 5312 5312 5376 5376 5440 5440 5504 5504 5568 5568 5632 5632 5696 5696 5760 5760 5824 5824 5888 5888 5952 5952 6016 6016"

_sg16_layout(){

	kdb_vars="  int:sys_dsl_${iface}_rate	\
			str:sys_dsl_${iface}_mode	\
			str:sys_dsl_${iface}_code	\
			str:sys_dsl_${iface}_cfg	\
			str:sys_dsl_${iface}_annex	\
			str:sys_dsl_${iface}_crc	\
			str:sys_dsl_${iface}_fill	\
			str:sys_dsl_${iface}_inv"	
	subsys="dsl."$iface

	render_save_stuff

	render_form_header

	render_table_title "$iface MR16H modem settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface

	# sys_dsl_${iface}_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Rate" sys_dsl_${iface}_rate $tmp_rates

	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Mode" sys_dsl_${iface}_mode  master 'Master' slave 'Slave'

	# sys_dsl_${iface}_code
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG16Code();"
	render_input_field select "Coding" sys_dsl_${iface}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_${iface}_cfg
	tip=""
	desc="Select DSL configuration mode"
	id='cfg'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Config" sys_dsl_${iface}_cfg local local preact preact

	# sys_dsl_${iface}_annex
	tip=""
	desc="Select DSL Annex"
	id='annex'
	onchange="OnChangeSG16Code();"	
	render_input_field select "Annex" sys_dsl_${iface}_annex A "Annex A" B "Annex B" F "Annex F"

	# sys_dsl_${iface}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_dsl_${iface}_crc crc32 CRC32 crc16 CRC16

	# sys_dsl_${iface}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_dsl_${iface}_fill  fill_ff FF fill_7e 7E

	# sys_dsl_${iface}_inv
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_dsl_${iface}_inv  normal off invert on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG16Code();"
}


_sg17_layout(){
	kdb_vars="  int:sys_dsl_${iface}_rate	\
			str:sys_dsl_${iface}_mode	\
			str:sys_dsl_${iface}_code	\
			str:sys_dsl_${iface}_annex	\
			str:sys_dsl_${iface}_crc	\
			str:sys_dsl_${iface}_fill	\
			str:sys_dsl_${iface}_inv"	
	subsys="dsl."$iface

	render_save_stuff

	render_form_header

	render_table_title "$iface MR17H modem settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	
	# sys_dsl_${iface}_rate
	tip=""
	desc="Select DSL line rate"
	validator='tmt:message="'$desc'"'
	id='rate'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Rate" sys_dsl_${iface}_rate $tmp_rates

	# sys_dsl_${iface}_mode
	tip=""
	desc="Select DSL mode"
	id='mode'
	onchange="OnChangeSG17Code();"	
	render_input_field select "Mode" sys_dsl_${iface}_mode  master 'Master' slave 'Slave'

	# sys_dsl_${iface}_code
	tip=""
	desc="Select DSL line coding"
	id='code'
	onchange="OnChangeSG17Code();"
	render_input_field select "Coding" sys_dsl_${iface}_code tcpam32 TCPAM32 tcpam16 TCPAM16 tcpam8 TCPAM8 tcpam4 TCPAM4

	# sys_dsl_${iface}_annex
	tip=""
	desc="Select DSL Annex"
	render_input_field select "Annex" sys_dsl_${iface}_annex A "Annex A" B "Annex B"

	# sys_dsl_${iface}_crc32
	tip=""
	desc="Select DSL CRC length"
	render_input_field select "CRC" sys_dsl_${iface}_crc crc32 CRC32 crc16 CRC16

	# sys_dsl_${iface}_fill
	tip=""
	desc="Select DSL fill byte value"
	render_input_field select "Fill" sys_dsl_${iface}_fill  fill_ff FF fill_7e 7E

	# sys_dsl_${iface}_inv
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_dsl_${iface}_inv  normal off invert on

	render_submit_field
	render_form_tail

	run_js_code "OnChangeSG17Code();"
}


sg16_cfg_path="/sys/bus/pci/drivers/sg16lan"
sg17_cfg_path="/sys/class/net"

iface="${FORM_iface}"
	
eval `kdb -qq ls "sys_dsl_*" `
	

if [ -z "$iface" ]; then
	render_table_title "Modem status" 2
	for iface in $sys_dsl_ifaces; do
	    tmp=`ifconfig $iface | grep RUNNING`
	    if [ -n "$tmp" ]; then
		link="online"
	    else
		link="offline"
	    fi
	    tip=""
	    desc="SHDSL Link state"
	    render_input_field static "SHDSL Status" status "$link"
	done
else
    unset mode rate code annex cfg crc fill inv
    eval `kdb -qq sls "sys_dsl_${iface}_" `
    if [ "$mtype" == "mr16h" ]; then
	_sg16_layout $iface
    elif [ "$mtype" == "mr17h" ]; then
	_sg17_layout $iface
    else
	echo "Error module type!"
    fi

fi

