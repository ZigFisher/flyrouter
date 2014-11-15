#!/usr/bin/haserl

	subsys="ipsec"

	table=$FORM_table
	item=$FORM_item

	eval_string="export FORM_$item=\"enabled=$FORM_enabled mode=$FORM_mode src=$FORM_src dst=$FORM_dst "
	case "$table" in
	sad)	eval_string="$eval_string spi=$FORM_spi ah_alg=$FORM_ah_alg ah_key=$FORM_ah_key esp_alg=$FORM_esp_alg esp_key=$FORM_esp_key \"";;
	spd) 	eval_string="$eval_string upperspec=$FORM_upperspec direction=$FORM_direction policy=$FORM_policy esp_enabled=$FORM_esp_enabled ah_enabled=$FORM_ah_enabled src_dst=$FORM_src_dst level=$FORM_level \"";;
	esac
	render_popup_save_stuff
	
	render_form_header ipsec_edit
	render_table_title "IPSec $table edit" 2
	render_popup_form_stuff
	
	render_input_field hidden table table "$table"

	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled
	case $table in
	sad)
		# spi
		tip="SPI must be a decimal number, or a hexadecimal number with a <b>0x</b> prefix<br/> SPI values between 0 and 255 are reserved for future use by IANA and cannot be used. TCP-MD5 associations must use 0x1000 and therefore only have per-host granularity at this time."
		desc="Security Parameter Index (SPI) <b>required</b>"
		validator=$validator_spi
		render_input_field text "SPI" spi

		# mode
		tip=""
		desc="Security protocol mode for use."
		render_input_field select "Mode" mode any 'Any' transport 'Transport' tunnel 'Tunnel'
		
		# src
		desc="Source address (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "Source" src

		# dst
		desc="Destination address (dotted quad) <b>required</b>"
		validator="$tmtreq $validator_ipaddr"
		render_input_field text "Destination" dst

		# ah_alg
		tip=""
		desc="Authentication algorithm"
		render_input_field select "AH Algorithm" ah_alg none none null null hmac-md5 'hmac-md5 (128bit)' hmac-sha1 'hmac-sha1 (160bit)' keyed-md5 'keyed-md5 (128bit)' keyed-sha1 'keyed-sha1 (160bit)'  tcp-md5 'tcp-md5 (8 to 640 bit)' #hmac-sha256 'hmac-sha256 256bit' hmac-sha384 'hmac-sha384 384bit' hmac-sha512 'hmac-sha512 512bit' hmac-ripemd160 'hmac-ripemd160' aes-xcbc-mac 'aes-xcbc-mac' 

		# ah_key
		default="0x"
		tip="Must be a series of hexadecimal digits preceded by <b>0x</b>"
		desc=""
		validator=$validator_ipseckey
		render_input_field text "AH Key" ah_key

		# esp_alg
		tip=""
		desc="Encryption algorithm"
		render_input_field select "ESP Algorithm" esp_alg none none null null 'des-cbc' 'des-cbc (64bit)' '3des-cbc' '3des-cbc (192bit)' 'blowfish-cbc' 'blowfish-cbc (40 to 448 bit)' cast128-cbc 'cast128-cbc (40 to 448 bit)' #des-deriv 'des-deriv' 3des-deriv '3des-deriv' rijndael-cbc 'rijndael-cbc' twofish-cbc 'twofish-cbc' aes-ctr 'aes-ctr'
		
		# esp_key
		default="0x"
		tip="Must be a series of hexadecimal digits preceded by <b>0x</b>"
		validator=$validator_ipseckey
		render_input_field text "ESP Key" esp_key
		;;
	spd)
		# src
		tip="<b>Example: <b/><ol><li>192.168.0.1<li>192.168.1.0/24</li>"
		desc="Can be an IPv4 address or an IPv4 address range <b>required</b>"
		validator=$validator_ipnet
		render_input_field text "Source range" src

		# dst
		tip="<b>Example: <b/><ol><li>192.168.0.1<li>192.168.1.0/24</li>"
		desc="Can be an IPv4 address or an IPv4 address range <b>required</b>"
		validator=$validator_ipnet
		render_input_field text "Destination" dst
		
		# upperspec
		tip=""
		desc="Upper-layer protocol to be used."
		render_input_field select "Upper-layer" upperspec any any ip4 ip4 icmp icmp tcp tcp udp udp

		# direction
		tip=""
		desc=""
		render_input_field select "Direction" direction in 'In' out 'Out' fwd 'Forward'

		# policy
		tip=""
		desc=""
		render_input_field select "Policy" policy ipsec ipsec none none discard discard

		[ -z "$src" ] && default=1 # for new record
		render_input_field checkbox "AH" ah_enabled
		[ -z "$src" ] && default=1 # for new record
		render_input_field checkbox "ESP" esp_enabled

		# Note: Some simplification - mode, src_dst, level both for AH and ESP
		# mode
		tip=""
		desc=""
		render_input_field select "Mode" mode transport 'Transport' tunnel 'Tunnel'

		# src_dst
		tip="If mode is tunnel, you must specify the end-point addresses of the SA as src and dst with - between these addresses, which is used to specify the SA to use<br><b>Exmples:</b><ol><li>192.168.0.1-192.168.1.2<li>192.168.0.1[4500]-192.168.1.2[30000]</ol>"
		desc="If mode is transport, it can be omitted"
		validator='tmt:filters="ltrim,rtrim,nohtml,nospaces,nocommas,nomagic,noquotes,nodoublequotes" tmt:message="Please input correct address range" tmt:pattern="ipsec_src_dst"'
		render_input_field text "Source-Destination" src_dst 
		
		# level
		tip="<ol><li><b>default</b> means the kernel consults the system wide default for the protocol you specified, e.g. the esp_trans_deflev sysctl variable, when the kernel processes the packet.  <li><b>use</b> means that the kernel uses an SA if its available, otherwise the kernel keeps normal operation.  <li><b>require</b> means SA is required whenever the kernel sends a packet matched with the policy.  <li><b>unique</b> is the same as require; in addition, it allows the policy to match the unique out-bound SA.</ol>"
		desc=""
		render_input_field select "Level" level use use require require default default unique unique
	;;
	esac
	render_submit_field
	render_form_tail
	
# vim:foldmethod=indent:foldlevel=1
