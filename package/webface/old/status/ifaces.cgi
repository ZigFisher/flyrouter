#!/usr/bin/haserl
<? title="Interfaces"
   . ../common/header.sh
   . ../lib/misc.sh
   . ../lib/net.sh
   . ../lib/widgets.sh
 ?>
<? render_title ?>

<table>
<? render_table_title "Interface information" 7 ?>
<tr class='statusTrHeader'>
	<th rowspan=2>Interface</th>
	<th rowspan=2>Status</th>
	<th rowspan=2>MTU</th>
	<th colspan=2>Receive</th>
	<th colspan=2>Transmit</th>
</tr>
<tr class='statusTrHeader'>
	<th>Bytes</th>
	<th>Packets</th>
	<th>Bytes</th>
	<th>Packets</th>
</tr>
<?
echo "$( cat /proc/net/dev | sed -e "s/ \+/ /g" -e "s/:/ /"| grep -v er)" | \
	while read iface rxb rxp rxe rxd rxfi rxfr rxc rxm mc txb txp txe txd txfi txcol txcar txc; do
		rxb=$(humanUnits $rxb bytes )
		txb=$(humanUnits $txb bytes )
		eval $(getIfaceParams $iface)
		echo "<tr class='statusTrData'>";
		echo "<td>$iface</td>"
		echo " <td>$ifstatus</td>"
		echo " <td>$ifmtu</td>"
		echo " <td>$rxb</td>"
		echo " <td>$rxp</td>"
		echo " <td>$txb</td>"
		echo " <td>$txp</td>"
		echo '</tr>';
	done
?>
</table>

<table>
<? render_table_title "Interface detail information" 17 ?>
<tr><th></th><th colspan=8>Receive</th><th colspan=8>Transmit</th></tr>
<tr><th>Device</th><th>Bytes</th><th>Packets</th><th>Errors</th><th>Drop</th><th>FIFO</th>
<th>Frame</th><th>Compressed</th><th>Multicast</th><th>Bytes</th><th>Packets</th><th>Errors</th><th>Drop</th>
<th>FIFO</th><th>Colls</th><th>Carrier</th><th>Compressed</th></tr>

<?
echo "$( cat /proc/net/dev | sed -e "s/ \+/ /g" -e "s/:/ /"| grep -v er)" | \
        while read iface rxb rxp rxe rxd rxfi rxfr rxc rxm mc txb txp txe txd txfi txcol txcar txc; do
            echo "<tr><td>$iface</td><td>$rxb</td><td>$rxp</td><td>$rxe</td><td>$rxd</td><td>$rxfi</td>";
            echo "<td>$rxfr</td><td>$rxc</td><td>$rxm</td><td>$mc</td><td>$txb</td><td>$txp</td>";
            echo "<td>$txe</td><td>$txd</td><td>$txfi</td><td>$txcol</td><td>$txcar</td><td>$txc</td></tr>";
        done
?>
    </table>


<? . ../common/footer.sh ?>
