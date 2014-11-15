#!/bin/sh

rm -rf photo video
/home/builder/trunk/staging_dir_mips/bin/mips-linux-uclibc-gcc-3.3.3 -o photo comm_low.c c328_demo.c
/home/builder/trunk/staging_dir_mips/bin/mips-linux-uclibc-gcc-3.3.3 -o video video.c
/home/builder/trunk/staging_dir_mips/bin/mips-linux-uclibc-strip photo video