#!/usr/bin/haserl

cfg_path="/sys/bus/pci/drivers/mr16g/"

iface="${FORM_iface}"
	
eval `kdb -qq ls "sys_e1_*" `
	

if [ -n "$iface" ]; then

	kdb_vars="	str:sys_e1_${iface}_proto	\
			str:sys_e1_${iface}_hdlc_enc	\
			str:sys_e1_${iface}_hdlc_parity	\
			int:sys_e1_${iface}_cisco_int	\
			int:sys_e1_${iface}_cisco_to	\
		  	bool:sys_e1_${iface}_fram	\
			bool:sys_e1_${iface}_ts16	\
			str:sys_e1_${iface}_smap	\
			bool:sys_e1_${iface}_clk	\
			bool:sys_e1_${iface}_crc4	\
			bool:sys_e1_${iface}_cas	\
			bool:sys_e1_${iface}_lhaul	\
			int:sys_e1_${iface}_lcode	\
			int:sys_e1_${iface}_hcrc	\
			int:sys_e1_${iface}_fill	\
			int:sys_e1_${iface}_inv	"
	subsys="e1."$iface

	render_save_stuff

	render_form_header
	render_table_title "$iface modem settings" 2
	
	# refresh settings
	eval `kdb -qq ls "sys_e1_*" `
	
	# sys_e1_${iface}_name
	render_input_field "hidden" "hidden" iface $iface


	# sys_e1_${iface}_proto
	tip=""
	desc=""
	render_input_field select "HDLC protocol" sys_e1_${iface}_proto  hdlc HDLC hdlc-eth ETHER-HDLC cisco CISCO-HDLC fr FR ppp PPP x25 X25

#TODO:	1. Make in Java script
#	2. find out what options is for FR!
	proto=`kdb get sys_e1_${iface}_proto`
        case "$proto" in
	hdlc*)
	    # sys_e1_${iface}_hdlc_enc
	    encodings="nrz nrzi fm-mark fm-space manchester"
	    tip=""
	    desc=""
	    render_input_field select "Encoding" sys_e1_${iface}_hdlc_enc $(for i in $encodings; do echo $i $i;done)

	    # sys_e1_${iface}_hdlc_parity
	    parity="crc16-itu no-parity crc16 crc16-pr0 crc16-itu-pr0 crc32-itu"
	    tip=""
	    desc=""
	    render_input_field select "Parity" sys_e1_${iface}_hdlc_parity $(for i in $parity; do echo $i $i;done)
	    ;;	    
	cisco)
    	    # sys_e1_${iface}_cisco_int
	    default=10
            tip=""
            render_input_field select "Interval" sys_e1_${iface}_cisco_int $(for i in `seq 1 10`; do n=$(($i*10)); echo $n $n; done)
	    
    	    # sys_e1_${iface}_cisco_to
	    default=25
    	    tip=""
            render_input_field select "Timeout" sys_e1_${iface}_cisco_to  $(for i in `seq 1 20`; do n=$(($i*5)); echo $n $n; done)
	    ;;

	*)
	    ;;
	esac

        # sys_e1_${iface}_fram
        default=0
        tip=""
        desc="check to enable"
        render_input_field checkbox "E1 framed mode" sys_e1_${iface}_fram

	# TODO: Java-script?
	# Valid only in framed mode
	fram=`kdb get sys_e1_${iface}_fram`	
	if [ $fram = 1 ]; then
            # sys_e1_${iface}_ts16
	    default=0
    	    tip=""
            desc="check to use"
	    render_input_field checkbox "Use time slot 16" sys_e1_${iface}_ts16

    	    # sys_e1_${iface}_smap
            default=0
	    tip=""
    	    desc="example: 2-3,6-9,15-20"
            render_input_field text "Slotmap" sys_e1_${iface}_smap

	    # sys_e1_${iface}_clk
    	    default=0
    	    tip=""
	    desc="check to enable"
	    render_input_field checkbox "E1 internal transmit clock" sys_e1_${iface}_clk
    
	    # sys_e1_${iface}_crc4
    	    default=0
	    tip=""
    	    desc="check to enable"
	    render_input_field checkbox "E1 CRC4 multiframe" sys_e1_${iface}_crc4

    	    # sys_e1_${iface}_cas
    	    default=0
    	    tip=""
    	    desc="check to enable"
    	    render_input_field checkbox "E1 CAS multiframe" sys_e1_${iface}_cas


	    

	fi

        # sys_e1_${iface}_lhaul
        default=0
        tip=""
        desc="check to enable"
        render_input_field checkbox "E1 long haul mode" sys_e1_${iface}_lhaul

        # sys_e1_${iface}_lcode
        tip=""
        desc=""
        render_input_field select " E1 HDB3/AMI line code" sys_e1_${iface}_lcode  1 HDB3 0 AMI

        # sys_e1_${iface}_crc32
        tip=""
        desc="Select DSL CRC length"
        render_input_field select "CRC" sys_e1_${iface}_hcrc 0 CRC32 1 CRC16
				
        # sys_e1_${iface}_fill
        tip=""
        desc="Select DSL fill byte value"
        render_input_field select "Fill" sys_e1_${iface}_fill  0 FF 1 7E
							
        # sys_e1_${iface}_inv
	tip=""
	desc="Select DSL inversion mode"
	render_input_field select "Inversion" sys_e1_${iface}_inv  0 off 1 on
												
	render_submit_field
	render_form_tail

fi


# vim:foldmethod=indent:foldlevel=1
