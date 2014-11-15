#!/bin/bash
# script to convert a binary fw package (output from gen_fw)
# into a .h file for static inclusion
# call fwbin2h.sh "comment" input_file output_file

if [ $# -ne 3 ]; then
        echo "#ERR to convert a binary fw file into a .h file"
        echo "#ERR call $0  "comment" input_file output_file"
        exit 1
fi

DEBUG=1

COMMENT=$1
INF=$2
OUTF=$3

[ $DEBUG -eq 1 ] && echo "#DBG comment $COMMENT, converting $INF into $OUTF"

echo '/* $Id: fwbin2pkg.sh,v 1.2 2003/12/25 22:40:27 jal2 Exp $ */' > $OUTF
echo "/* $COMMENT */" >> $OUTF
echo '/**************************************************************************/' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/*            Copyright (c) 1999-2000 by Atmel Corporation                */' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/*  This software is copyrighted by and is the sole property of Atmel     */' >> $OUTF
echo '/*  Corporation.  All rights, title, ownership, or other interests        */' >> $OUTF
echo '/*  in the software remain the property of Atmel Corporation.  This       */' >> $OUTF
echo '/*  software may only be used in accordance with the corresponding        */' >> $OUTF
echo '/*  license agreement.  Any un-authorized use, duplication, transmission, */' >> $OUTF
echo '/*  distribution, or disclosure of this software is expressly forbidden.  */' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/*  This Copyright notice may not be removed or modified without prior    */' >> $OUTF
echo '/*  written consent of Atmel Corporation.                                 */' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/*  Atmel Corporation, Inc. reserves the right to modify this software    */' >> $OUTF
echo '/*  without notice.                                                       */' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/*  Atmel Corporation.                                                    */' >> $OUTF
echo '/*  2325 Orchard Parkway               literature@atmel.com               */' >> $OUTF
echo '/*  San Jose, CA 95131                 http://www.atmel.com               */' >> $OUTF
echo '/*                                                                        */' >> $OUTF
echo '/**************************************************************************/' >> $OUTF
echo '' >> $OUTF
echo 'u8 fw_bin[] = {' >> $OUTF

od -An -tx1 -v -w8 $INF | sed 's/\([0-9a-f][0-9a-f]\)/0x\1,/g' >> $OUTF
echo '};' >> $OUTF
echo '' >> $OUTF
echo "const struct firmware static_fw = {`ls -l $INF | awk '{print $5}'`, fw_bin};" >> $OUTF
echo '' >> $OUTF