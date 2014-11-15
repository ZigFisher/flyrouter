#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

. /etc/templates/lib
. lib/service.sh

cfg_flash(){
	# for original midge with 2M flash
	[ -x /sbin/flash ] && /sbin/flash save >/dev/null 2>&1
}

update_configs_and_service_reload(){
	local subsys="$1"
	local s
	local service
	for s in $subsys; do 
		update_configs $s
		[ "$ERROR_MESSAGE" ] && return
	done
	fail_str="Update config failed: $ERROR_DETAIL"
	[ "$ERROR_MESSAGE" ] && return
	for service in $subsys; do
		service_reload $service 2>&1 | $LOGGER ;
	done

}

save(){
	local sybsys="$1"
	local kdb_vars="$2"
	local param;
	local ptype;

	while true; do 
		ok_str="Settings saved"

		for v in $kdb_vars; do
			ptype=${v%%:*}
			param=${v##$ptype:}
			debug "save(): param=$param ptype=$ptype<br>"
			case $ptype in
				str)	kdb_set_string $param ;;
				int)	kdb_set_int $param ;;
				bool)	kdb_set_bool $param ;;
				*)		ERROR_MESSAGE="save(): Unknown type '$ptype'"
			esac
			[ -n "$ERROR_MESSAGE" ] && break;
		done
		[ -n "$ERROR_MESSAGE" ] && break;
		
		kdb_commit
		fail_str="Savings failed: $ERROR_DETAIL"

		[ -n "$ERROR_MESSAGE" ] && break
		
		update_configs_and_service_reload "$subsys"
		break;
	done;
}
