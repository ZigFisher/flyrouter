#!/usr/bin/haserl
<? title="Disk usage"
   . ../common/header.sh
   . ../lib/misc.sh
   . ../lib/widgets.sh
 ?>

<? render_title ?>
<? exec 2>/dev/null ?>

<table>
<? render_table_title "Disk information" 6 ?>
<tr><th>Device</th><th>Mount point</th><th>Type</th><th>Size</th><th>Used</th><th>Available</th></tr>
<? echo "$(df | grep -v Filesystem | sed "s/ \+/ /g" | cut -f 1,2,3,5,6 -d' ')" | \
    while read dev size used avail fs; do 
        type=$(getFsType $dev)
        echo "<tr><td>$dev</td><td>$fs</td><td>$type</td><td>$(humanUnits -k $size bytes )</td><td>$(humanUnits $used bytes)</td><td>$(render_chart_h '' $(($size-$used)) $size)</td></tr>"
    done
?>
</table>

<? . ../common/footer.sh ?>
