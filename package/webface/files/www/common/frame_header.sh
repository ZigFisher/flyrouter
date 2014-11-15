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
<script type="text/javascript" src="/js/overlib.js"></script>
<script type="text/javascript" src="/js/overlib_bubble.js"> </script>
<script type="text/javascript" src="/js/script_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/vlad_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/misc.js"> </script>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link href="css/content.css" rel="stylesheet" type="text/css">
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
' 
		