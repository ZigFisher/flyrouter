#!/bin/sh

clear
#
rm -rf cramfs*
#
wget http://ftp.de.debian.org/debian/pool/main/c/cramfs/cramfs_1.1.orig.tar.gz
#wget http://sourceforge.net/projects/cramfs/files/cramfs/1.1/cramfs-1.1.tar.gz
#
wget http://svn.gumstix.com/gumstix-buildroot/trunk/target/cramfs/cramfs-01-devtable.patch
wget http://svn.gumstix.com/gumstix-buildroot/trunk/target/cramfs/cramfs-02-endian.patch
#wget http://ftp.de.debian.org/debian/pool/main/c/cramfs/cramfs_1.1-6.diff.gz
#
tar xvfz cramfs_1.1.orig.tar.gz
gunzip cramfs_1.1-6.diff.gz
#
patch -d cramfs-1.1 < cramfs-01-devtable.patch
patch -d cramfs-1.1 < cramfs-02-endian.patch
#patch -d cramfs-1.1 < cramfs_1.1-6.diff
#
make clean -s -C ./cramfs-1.1
make -s -C ./cramfs-1.1
