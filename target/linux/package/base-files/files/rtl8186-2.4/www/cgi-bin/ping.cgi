#!/usr/bin/haserl

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=koi8-r">
<meta http-equiv="refresh" content="60">
</head>

<? title="Command: ping *"
 echo '<b>Command:</b> <font color="red"><b>ping *</b></font>'
 echo '<pre>'
 ping -c 3 zftlab.org
 echo
 echo
 ping -c 3 flyrouter.net
 echo '</pre>'
?>
</html>
