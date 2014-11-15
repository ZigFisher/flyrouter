#!/usr/bin/haserl

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=koi8-r">
<meta http-equiv="refresh" content="60">
</head>

<? title="Command: traffic1"
 echo '<b>Command:</b> <font color="red"><b>traffic1</b></font>'
 echo '<pre>'
 [ -x /usr/bin/vnstat ] && vnstat
 echo '</pre>'
?>
</html>
