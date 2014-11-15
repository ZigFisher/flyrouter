#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
#

[ "$refresh" ] && echo "Refresh: $refresh;url=$refresh_url"
[ -n "$FORM_SESSIONID" ] && SESSIONID=$FORM_SESSIONID
echo "Set-Cookie: SESSIONID=$SESSIONID; path=/;"
echo 'Cache-Control: no-cache
Content-Type: text/html; Charset=utf-8

'
echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>WebFace</title>
<script type="text/javascript">function overlib(){return true;}</script>
<script type="text/javascript" src="/js/overlib.js"></script>
<script type="text/javascript" src="/js/overlib_bubble.js"> </script>
<script type="text/javascript" src="/js/script_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/vlad_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/baloon_validator.js"></script>
<script type="text/javascript" src="/js/misc.js"> </script>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link href="css/content.css" rel="stylesheet" type="text/css">
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<table width="750" border="0" cellspacing="0" cellpadding="2">
  <tr valign="bottom"> 
    <td width="150" height="65" align="center" valign="middle"> <strong><a href="http://sigrand.ru" target="_blank"><img src="img/logo.gif" width="92" height="84" border="0"></a></strong></td>
    <td height="65" bgcolor="#435370">
	<table border="0" cellspacing="0" cellpadding="0" width="100%">
	<tr><td align="left" valign="middle"><span class="tfrtitle">&nbsp;&nbsp;&nbsp;Sigrand</span></td>
	  <td align="right" valign="bottom">
	  <span class="hostname">'; export KDB=/etc/kdb; kdb get sys_hostname ; echo '</span>
	  </td></tr></table>
	</td>
  </tr>
  <tr valign="top"> 
    <td width="150" bgcolor="#9D9D9D">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>'
. common/menu.sh

echo '	</td>
        </tr>
		</table></td>
    <td width="600"><table width="100%" border="0" cellpadding="10" cellspacing="0">
        <tr><td><br /> <br />' 
		
