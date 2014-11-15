#!/usr/bin/haserl -u
<?

. conf/conf.sh

act=$FORM_act
uploadfile=$FORM_uploadfile


case "$act" in
	backup)
		echo "Content-type: application/octet-stream"
		echo "Content-Disposition: attachment; filename=\"`hostname`-`date +%Y%m%d%H%M%S`.cfg\""
		echo 

		kdb export -
	;;
	restore)
		echo "Content-type: text/html"
		echo 
		echo "<html><body>"
		echo "<h2>"

		if [ -r $uploadfile ]; then
			tmpkdb=$KDB
			export KDB=/tmp/kdbupload	
			kdb create
			if kdb import $uploadfile; then
				rm /tmp/kdbupload
				export KDB=$tmpkdb
				if kdb import $uploadfile; then
					echo "File imported successfully"
				else
					echo "Something wrong"
				fi
				echo "<script language=\"JavaScript\">setTimeout('window.location = \"/\"', 2000);</script>"
			else
				echo "Error occured while import configuration"
			fi
			rm -f $uploadfile
		else
			echo "File '$uploadfile' not found "
		fi
	;;
	default)
		echo "Content-type: text/html"
		echo 
		cp /etc/kdb.default /etc/kdb
		echo "<html><body>"
		echo "<h2>Default configuration restored"
		echo "<script language=\"JavaScript\">setTimeout('window.location = \"/\"', 2000);</script>"
	;;
esac
		
?>
