#!/bin/bash
# arg1: kernel source root dir
# arg2..n: files to copy

#set -x
if [ $# -ne 1 ]; then
        echo "#ERR call $0 kernel_root_dir"
        echo "set environment vars DRV_SRC, DRV_HDR to the file names"
        exit 1
fi

KSRC=$1
DEST=$KSRC/drivers/net/wireless
# subdir created in the above path
SUBDIR=at76c503
# source of the makefile for the kernel subdir
KMAKEFILE=Makefile.k26

if [ "$DRV_SRC" == "" ]; then echo "#ERR env var DRV_SRC not set or empty"; exit 1; fi

if [ "$DRV_HDR" == "" ]; then echo "#ERR env var DRV_HDR not set or empty"; exit 1; fi

if [ ! -r at76c503.h ]; then echo "#ERR cannot read at76c503.h"; exit 2; fi

if [ ! -d $DEST ]; then echo "#ERR $DEST is no directory"; exit 3; fi

if [ ! -w $DEST ]; then  echo "#ERR cannot write to directory $DEST"; exit 4; fi

if [ ! -d $DEST/$SUBDIR ]; then
    echo "creating $DEST/$SUBDIR"
    if  mkdir -p $DEST/$SUBDIR; then true; else echo "#ERR cannot create dir $DEST/$SUBDIR" && exit 5; fi
else
    if [ ! -d $DEST/$SUBDIR ]; then
        echo "#ERR $DEST/$SUBDIR exists and is no directory"; exit 5
    fi
    if [ ! -w $DEST/$SUBDIR ]; then
        echo "#ERR dir $DEST/$SUBDIR exists and is not writable"; exit 5
    fi
fi

VERSION=`grep '^#define DRIVER_VERSION' at76c503.h | sed 's/#define.*DRIVER_VERSION.*\"\(.*\)\".*$/\1/g'`

echo "patching driver version $VERSION into kernel source $KSRC"

echo "copying $DRV_SRC $DRV_HDR $KMAKEFILE into $DEST/$SUBDIR"

# for tests only
#exit 0

for f in $DRV_SRC $DRV_HDR; do
    if cp -v $f $DEST/$SUBDIR; then true; else echo "#ERR cannot copy $i into $DEST/$SUBDIR"; exit 5; fi
done

# copy the Makefile
if cp -v $KMAKEFILE $DEST/$SUBDIR/Makefile; then 
    true;
else
    echo "#ERR cannot copy $KMAKEFILE into $DEST/$SUBDIR/Makefile"
    exit 5
fi

# patch Kconfig
PWD=`pwd`
cd $DEST
if grep "^config USB_ATMEL" Kconfig; then
    echo "found \"config USB_ATMEL\" in $DEST/Kconfig - leave it untouched"
else
    echo "going to patch $DEST/Kconfig (original copied into Kconfig.orig)"

    mv Kconfig Kconfig.orig
# remove the last endmenu in Kconfig
    head -$((`grep -n endmenu Kconfig.orig | tail -1 | sed 's/\([0-9]*\).*/\1/'`-1)) Kconfig.orig > Kconfig
     (cat <<"EOF"
config USB_ATMEL
	tristate "Atmel at76c503a/505/505a USB adapter"
	depends on NET_RADIO && ATMEL && USB
	select FW_LOADER
	---help---
	  Enable support for WLAN USB adapters containing the
	  Atmel at76c503a, 505 and 505a chips.

endmenu
EOF
  ) >> Kconfig
fi
     
# patch the Makefile
if grep "CONFIG_USB_ATMEL" Makefile; then
    echo "found \"config CONFIG_USB_ATMEL\" in $DEST/Makefile - leave it untouched"
else
    echo "going to patch $DEST/Makefile (original copied into Makefile.orig)"

    cp Makefile Makefile.orig
    (cat <<EOF

obj-\$(CONFIG_USB_ATMEL)		+= $SUBDIR/

EOF
  ) >> Makefile

fi
    
    
