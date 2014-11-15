#!/bin/sh

log() {
    echo "$*" >>$LOGFILE
}
log_dev() {
    echo "$*" >>$DEVDIR/log
}


# round-robin btdev polling
get_btdev_push() {
	local dev;
	local q;
	BTDEV_JACK=`echo $BTDEV_JACK` # space trimming
	dev=`echo "$BTDEV_JACK" | cut -f1 -d" "`
	q=${BTDEV_JACK##$dev}
	q=`echo $q` # space trimming
	BTDEV_JACK="${q} ${dev}"
	echo $dev
}

get_info() {
	local mac=$1
	local file=$DEVDIR/info
	
	enter_critical_section $BTDEV_SCAN
	if [ ! -r $file ]; then 
		info 5 "Get info from $mac..."
		hcitool -i $BTDEV_SCAN info $mac >$file
	elif [ 3 -gt "`grep -c '' $file `" ]; then
		info 5 "Reget info from $mac (too low info file)..."
		hcitool info $mac >$file
	fi
	leave_critical_section $BTDEV_SCAN
}

scan_services() {
	local mac=$1
	local file=$DEVDIR/services
	enter_critical_section $BTDEV_SCAN
	if [ ! -r "$file" ]; then
		info 5 "Scan services from $mac..."
		sdptool browse $mac >$file
	elif [ 3 -gt "`grep -c "Service Name" $file`" ]; then
		info 5 "Rescan services from $mac (too low services)..."
		sdptool browse $mac >$file
	elif grep -e 'Fail' $file >/dev/null; then
		info 5 "Rescan services from $mac (found 'Fail')..."
		sdptool browse $mac >$file
	elif grep -e 'timed out' $file >/dev/null; then
		info 5 "Rescan services from $mac (found 'timed out')..."
		sdptool browse $mac >$file
	fi
	leave_critical_section $BTDEV_SCAN
}

get_channel() {
	local servname="$1"
	local filename="$DEVDIR/`echo ${servname}_channel | tr 'A-Z ' 'a-z_'`"
	if [ ! -s $filename ]; then
		grep -A 10 "Service Name: $servname" $DEVDIR/services | grep Channel | cut -d: -f2 >$filename
	fi
	cat $filename
}

can_jack_by_blacklist() {
	local mac="$1"
	if [ -s "$BLACKLIST_FILE" ]; then
		if grep $mac $BLACKLIST_FILE >/dev/null; then
			return 1;
		else
			return 0;
		fi
	fi
	return 0;
}

can_jack_by_push_fail() {
	local filename=$DEVDIR/push_fail_wakeup
	local now wakeup_time;
	if [ ! -s $filename ]; then
		return 0;
	else
		wakeup_time=`cat $filename`
		now=`date +%s`
		if [ "$now" -ge "$wakeup_time" ]; then
			rm $filename
			return 0
		else
			info 5 "Wakeup time is not reached: now=$now wakeup=$wakeup_time"
		fi
	fi
	return 1
}

update_push_fails_waiting() {
	local count="$1"
	if [ "$count" -gt "$PUSH_FAIL_COUNTS_TO_SLEEP" ]; then
		local now=`date +%s`
		echo $(($now+$PUSH_FAIL_SLEEP_TIME)) >$DEVDIR/push_fail_wakeup
	fi
}

inc_push_fail() { 
	local fails=0
	local filename="$DEVDIR/push_fail_count"
	[ -s $filename ] && fails=`cat $filename`
	fails=$(($fails+1))
	echo $fails >$filename
	update_push_fails_waiting $fails
}

clear_push_fail() { 
	local filename="$DEVDIR/push_fail_count"
	echo 0 >$filename
}

update_adv_list() {
	local cwd=$PWD
	info 4 "Updating advertising list..."
	cd $ADV_DIR; ls >$JACKERDIR/adv_list
	info 4 "Updating advertising count..."
	#wc -l $JACKERDIR/adv_list | awk '{print $1}' >$JACKERDIR/adv_count
	grep -c ".*" $JACKERDIR/adv_list >$JACKERDIR/adv_count
	cd $cwd
}


update_bonuses_list() {
	local cwd=$PWD
	info 4 "Updating bonuses_list..."
	cd $BONUS_DIR; ls >$JACKERDIR/bonuses_list
	info 4 "Updating bonuses_count..."
	#wc -l $JACKERDIR/bonuses_list | awk '{print $1}' >$JACKERDIR/bonuses_count
	grep -c ".*" $JACKERDIR/bonuses_list >$JACKERDIR/bonuses_count
	cd $cwd
}



get_bonus_file() {
	local i lineno
	local result
	[ -r $JACKERDIR/bonuses_list ] || update_bonuses_list
	local bonuses_count="`cat $JACKERDIR/bonuses_count`"
	
	RANDOM=`date +%s`;
	for i in `qqq=1; while [ $qqq -le $(cat $JACKERDIR/bonuses_count) ]; do echo -n "$qqq "; qqq=$(($qqq+1)); done` ; do
 		lineno=$(( $RANDOM % ${bonuses_count} + 1 ))
		line=`head -n $lineno $JACKERDIR/bonuses_list | tail -n 1 `
		
		[ -r $DEVDIR/sent_bonuses ] || touch $DEVDIR/sent_bonuses
		if grep "$line" $DEVDIR/sent_bonuses >/dev/null 2>&1; then
			info 3 "File: $line is already sent"
		else
			info 2 "Prepare to send $line"
			break
		fi
	done
	if [ -n "$line" ]; then
		echo $line >>$DEVDIR/sent_bonuses
		echo $BONUS_DIR/$line
	else
		warn "Cannot find bonus file, maybe all bonuses are sent?"
	fi
}

get_adv_file() {
	local counter_file=$DEVDIR/adv_counter
	# counter_file - *next to send* counter
	local counter;
	[ -r "$counter_file" ] && counter="`cat $counter_file`" || counter="0000"
	
	local filename="`ls $ADV_DIR/${counter}.* | head -n 1`"
	
	[ "$counter" = "0000" -a -z "$filename" ] && warn "there is no initial $ADV_DIR/0000.* file!" 
	
	if [ -z "$filename" -a "$ADV_CYCLED" = 1 ]; then 
		info 3 "File $ADV_DIR/$counter.* is not found, reset counter to 0000"
		counter="0000"
		filename="`ls $ADV_DIR/$counter.* | head -n 1`"
	fi
	
	counter=`printf %.4d $(($counter+1))`
	echo $counter >$counter_file
		
	# return value
	info 5 "get_adv_file() returned $filename"
	echo "${filename}"
}

get_file_name() {
	RANDOM=`date +%s`;
	local r=$(($RANDOM%10))
	local sfile
	if [ -s "$DEVDIR/script" ]; then
		info 3 "Execute $DEVDIR/script"
		. $DEVDIR/script
	elif [ ! -r $DEVDIR/adv_counter ]; then
		info 3 "FIRST SEEN - ADVERTISING"
		sfile=`get_adv_file`
	elif [ $r -ge $ADV_PROPORTION ]; then
		info 3 "BONUS"
		sfile=`get_bonus_file`
	else
		info 3 "ADVERTISING"
		sfile=`get_adv_file`
	fi
	echo $sfile
}

enter_critical_section() {
	local fname="/tmp/cs_$1.lock"
	while [ -r $fname ]; do
		usleep 300000;
	done
	echo $$ >$fname
}

leave_critical_section() {
	local fname="/tmp/cs_$1.lock"
	rm -f $fname;
}
