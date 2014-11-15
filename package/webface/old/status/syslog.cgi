#!/usr/bin/haserl
<? title="System log"
   . ../common/header.sh
   . ../lib/misc.sh
   . ../lib/widgets.sh
 ?>

<? render_title ?>
<table width=100%>
<? render_table_title "System log" ?>
<tr><td>

<pre class="code">
<?
    tail_opt=40;
    if [ "$FORM_syslog_size" ]; then
        tail_opt="$FORM_syslog_size"
    fi
	if [ -x /sbin/logread ]; then
		/sbin/logread | tail -"$tail_opt" 
	elif [ -r /var/log/syslog ]; then
		cat /var/log/syslog  | tail -"$tail_opt"
    fi
?>
</pre>
</td></tr>

<tr><td>
<form method="get">
<select name="syslog_size">
<option value="40">40 lines</option>
<option value="100">100 lines</option>
<option value="200">200 lines</option>
<option value="500">500 lines</option>
<option value="1000">1000 lines</option>
</select>
<input value="Set" type="submit">
</form>
</td></tr>
</table>


<? . ../common/footer.sh ?>
