#!/bin/sh

eval `$kdb -qq list sys_*`

echo '
		<form action="" method="POST">
            <table width="100%" border="0" cellspacing="0" cellpadding="0">
              <tr align="center" valign="top"> 
                <td height="10" colspan="2">&nbsp;</td>
              </tr>
              <tr align="center" valign="top"><br /> 
                <td height="170" colspan="2"><img src="img/SG.jpg" width="520" ><br /></td>
              </tr>
              <tr> 
                <td colspan="2" class="listtopic">System information</td>
              </tr>
              <tr> 
                <td width="25%" class="vncellt">Name</td>
                <td width="75%" class="listr">'$sys_hostname'</td>
              </tr>
              <tr> 
                <td width="25%" valign="top" class="vncellt">Version</td>
                <td width="75%" class="listr">'$WEBFACE_VER'
				
                </td>
              </tr>
              <tr> 
                <td width="25%" class="vncellt">Platform</td>
                <td width="75%" class="listr">'`uname` - `uname -r`'</td>
              </tr>
              <tr> 
                <td width="25%" class="vncellt">Hardware</td>
                <td width="75%" class="listr"> ' `cat /proc/cpuinfo | head -1 | cut -f2 -d:`' </td>
              </tr>
              <tr> 
                <td width="25%" class="vncellt">Time</td>
                <td width="75%" class="listr"> '`date`'

                </td>
              </tr>
			  <tr> 
                <td width="25%" class="vncellt">Uptime</td>
                <td width="75%" class="listr"> '`uptime | cut -f1 -d,`'

                </td>
              </tr>
<!--
              <tr> 
                <td width="25%" class="vncellt">Last config change</td>
                <td width="75%" class="listr"> 
                  
                </td>
              </tr>
-->
			  <tr> 
                <td width="25%" class="vncellt">CPU usage</td>
                <td width="75%" class="listr">'`uptime  | cut -f5 -d:`'
				</td>
              </tr>
			  <tr> 
                <td width="25%" class="vncellt">Memory usage</td>
                <td width="75%" class="listr">

'
# vim:foldmethod=indent:foldlevel=1
