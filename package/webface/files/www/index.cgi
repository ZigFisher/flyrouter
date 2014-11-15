#!/usr/bin/haserl
<? 

controller=$FORM_controller

if [ -n "$FORM_frame" ]; then
	. common/frame_header.sh
elif [ -n "$FORM_popup" ]; then
	. common/popup_header.sh
else
	. common/header.sh 
fi

. conf/conf.sh 
. lib/cfg.sh
. lib/kdb.sh
. lib/widgets.sh
. lib/misc.sh


[ "$controller" ] || controller="welcome"

if [ -f controllers/"$controller".sh ]; then
	if [ -n "$DEBUG" -a `hostname` != "home.localnet" ]; then
		set -x
	fi
	. controllers/"$controller".sh
else
	echo "Error: controller $controller not found"
fi


if [ -n "$FORM_frame" ]; then
	. common/frame_footer.sh
elif [ -n "$FORM_popup" ]; then
	. common/popup_footer.sh
else
	. common/footer.sh
fi
	
?>
