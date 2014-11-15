#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
#

#local s="set last_update=`date +%Y%m%d-%H:%M:%S`";
#KDB_PARAMS="set last_update=`date +%Y%m%d-%H:%M:%S` : "
KDB_PARAMS=""

kdb_ladd_string(){
	eval "export $1=\$FORM_$1"
	KDB_PARAMS="${KDB_PARAMS} : ladd $1=%ENV "
}

kdb_set_string(){
	eval "export $1=\$FORM_$1"
	KDB_PARAMS="${KDB_PARAMS} : set $1=%ENV "
}

kdb_set_int(){
	kdb_set_string $1
}

kdb_set_bool(){
	local param="$1";
	local value;

	eval "value=\$FORM_${param}"
	if [ "$value" = "on" ]; then
		KDB_PARAMS="${KDB_PARAMS} : set $param=1 "
	else
		KDB_PARAMS="${KDB_PARAMS} : set $param=0 "
	fi
}

kdb_commit(){
	debug "KDB_PARAMS: ${KDB_PARAMS}"
	eval `kdb ${KDB_PARAMS}`
	return $?
}
