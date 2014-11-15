#!/usr/bin/haserl
<? title="Routes"
	. ../common/header.sh
	. ../lib/misc.sh
	. ../lib/widgets.sh
 ?>
<? render_title ?>

<table>

<? render_table_title "Kernel routing table" 6 ?>
<tr><th>Network</th><th>Via</th><th>Device</th><th>Proto</th><th>Scope</th><th>Src</th></tr>

<?
if [ /usr/sbin/ip ]; then
echo "$( /usr/sbin/ip ro )" | \
		while read net c2 c3 c4 c5 c6 c7 c8 c9; do
			echo "<tr><td>$net</td><td>"
			if [ "$c2" = "via" ]; then
				echo "$c3"
			else
				echo "&nbsp;"
			fi
			echo "<td>"
			if [ "$c2" = "dev" ]; then
				echo "$c3"
			elif [ "$c4" = "dev" ]; then
				echo "$c5"
			fi
			echo "&nbsp;</td><td>";
			if [ "$c4" = "proto" ]; then
				echo "$c5"
			fi
			echo "&nbsp;</td><td>";
			if [ "$c6" = "scope" ]; then
				echo "$c7"
			fi
			echo "&nbsp;</td><td>";
			if [ "$c8" = "src" ]; then
				echo "$c9"
			fi
			echo "&nbsp;</td></tr>"
		done
fi
?>
	</table>


<? . ../common/footer.sh ?>
