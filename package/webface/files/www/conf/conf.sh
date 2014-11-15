#!/bin/sh
# (c) Vladislav Moskovets 2005
# Sigrand webface project
# 

CONF_REFRESH=3
CONF_KDB=/etc/kdb
#DEBUG=1
WARN=1
export KDB=$CONF_KDB
kdb=/usr/bin/kdb
IFRAME_WIDTH=600
IFRAME_HEIGHT=200
WEBFACE_VER=0.2
LOGGER="logger -s -p daemon.info -t webface"
ADM5120SW_PORTS="0 1 2 3"
