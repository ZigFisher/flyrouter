/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/cvsroot/uClinux-dist/user/goahead-2.1.4/LINUX/rtl_flashdrv.h,v 1.2 2004/05/10 10:48:50 yjlou Exp $
 *
 * $Author: yjlou $
 *
 * Abstract:
 *
 *   Flash driver header file for export include.
 *
 * $Log: rtl_flashdrv.h,v $
 * Revision 1.2  2004/05/10 10:48:50  yjlou
 * *: porting flashdrv from loader
 *
 * Revision 1.3  2004/03/31 01:49:20  yjlou
 * *: all text files are converted to UNIX format.
 *
 * Revision 1.2  2004/03/22 05:54:55  yjlou
 * +: support two flash chips.
 *
 * Revision 1.1  2004/03/16 06:36:13  yjlou
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2003/09/25 08:16:56  tony
 *  initial loader tree 
 *
 * Revision 1.1.1.1  2003/05/07 08:16:07  danwu
 * no message
 *
 */

#ifndef _RTL_FLASHDRV_H_
#define _RTL_FLASHDRV_H_



typedef struct flashdriver_obj_s {
	uint32  flashSize;
	uint32  flashBaseAddress;
	uint32 *blockBaseArray_P;
	uint32  blockBaseArrayCapacity;
	uint32  blockNumber;
} flashdriver_obj_t;



/*
 * FUNCTION PROTOTYPES
 */
void _init( void );
uint32 flashdrv_getDevSize( void );
uint32 flashdrv_eraseBlock( uint32 ChipSeq, uint32 BlockSeq );
uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size);



#endif  /* _RTL_FLASHDRV_H_ */

