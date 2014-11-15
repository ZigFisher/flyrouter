#!/bin/sh
#
# Need:  apt-get install zlib1g-dev
#
rm -rf cramfs-mod2.tar.bz2 cramfs
wget http://j0t.de/cramfs-mod2.tar.bz2
tar jxf cramfs-mod2.tar.bz2
patch -s -d ./cramfs < ./mod2.patch
#patch -s -d ./cramfs < ./make-static.patch
#
make clean -s -C ./cramfs
make -s -C ./cramfs
