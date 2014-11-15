#!/usr/bin/haserl
<? title="General info"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>

<? render_title ?>

<table>
<? render_table_title "General information" 2 ?>
<tr><td>Hostname:</td><td><? hostname ?></td></tr>
<tr><td>Kernel version:</td><td><? uname -s -r ?></td></tr>
<tr><td>Kernel build time:</td><td><? uname -v ?></td></tr>
<tr><td>Uptime:</td><td>
<?  b=`cat /proc/uptime| cut -f1 -d. `; 
	m=$(($b/60)); h=$(($m/60)); d=$(($h/24)); h=$(($h-$d*24)); m=$(($m-($d*60*24)-($h*60)))
	echo $d days, $h hours $m minutes?></td></tr>
<tr><td>Users:</td><td><?  uptime | cut -d, -f2 | cut -f2 -d' ' ?></td></tr>
<tr><td>Load avarage:</td><td><?  uptime | cut -f5 -d: ?></td></tr>
</table>

<table>
<? render_table_title "Memory usage information" 3 ?>
<? echo "$(cat /proc/meminfo | sed 's/ \+/ /')" | \
		while read text amount unit; do 
			if [ "$text" = "MemTotal:" ]; then
				export memtotal=$amount
				echo "<tr><td>Memory total:</td><td>$(humanUnits -k $amount bytes)</td><td>$(render_chart_h '' $amount $amount)</td></tr>";
			fi;
			[ "$text" = "MemFree:" ] && echo "<tr><td>Memory free:</td><td>$(humanUnits -k $amount bytes)</td><td>$(render_chart_h '' $amount $memtotal)</td></tr>";
			[ "$text" = "Buffers:" ] && echo "<tr><td>Buffers:</td><td>$(humanUnits -k $amount bytes)</td><td>$(render_chart_h '' $amount $memtotal)</td></tr>";
			[ "$text" = "Cached:" ] &&  echo "<tr><td>Cached:</td><td>$(humanUnits -k $amount bytes)</td><td>$(render_chart_h '' $amount $memtotal)</td></tr>";
		done
?>
</table>


<? . ../common/footer.sh ?>
