#!/usr/bin/haserl
<? title="DHCP"
   . ../common/header.sh
   . ../lib/misc.sh
   . ../lib/widgets.sh
 ?>

<? render_title ?>
<table>

<tr><th colspan=6>DHCP leases</th></tr>
<tr><th>IP address</th><th>Hardware type</th><th>Flags</th><th>MAC address</th><th>Mask</th><th>Device</th></tr>

<?
echo "$( cat /proc/net/arp )" | \
        while read ip hwtype flags hwaddr mask dev; do
            [ "$ip" = "IP" ] && continue;
            echo "<tr><td>$ip</td><td>$hwtype</td><td>$flags</td><td>$hwaddr</td><td>$mask</td><td>$dev</td></tr>";
        done
        # TODO - decode hwtype and flags to human readable
?>
    </table>


<? . ../common/footer.sh ?>
