/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/cvsroot/Belkin_RTL8651B/loader_srcroot/inc/rtl_image.h,v 1.1.1.1 2005/01/21 17:34:29 hyin Exp $
 *
 * $Author: hyin $
 *
 * Abstract:
 *
 *   Structure definitions of root directory and image header.
 *
 * $Log: rtl_image.h,v $
 * Revision 1.1.1.1  2005/01/21 17:34:29  hyin
 * Hank, imported from Realtek sdk 0.6.4 - 2005-01-21
 *
 * Revision 1.3  2004/08/19 02:22:26  rupert
 * +: Support Kernel+Root FS
 *
 * Revision 1.2  2004/03/31 01:49:20  yjlou
 * *: all text files are converted to UNIX format.
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

#ifndef _RTL_IMAGE_H_
#define _RTL_IMAGE_H_

/* "productMagic" field of fileImageHeader_t for ROME
0x59a0e842 - D-Link
0x47363134 - Netgear
0x47363134 - Edimax official
0x59a0e845 - Edimax non-public
 */
#define RTL_PRODUCT_MAGIC     0x59a0e845


/* "imageType" field of rootDirEntry_t and
 * fileImageHeader_t
 */
#define RTL_IMAGE_TYPE_RDIR   0xb162
#define RTL_IMAGE_TYPE_BOOT   0xea43
#define RTL_IMAGE_TYPE_RUN    0x8dc9
#define RTL_IMAGE_TYPE_KFS    0xd92f
#define RTL_IMAGE_TYPE_CCFG   0x2a05
#define RTL_IMAGE_TYPE_DCFG   0x6ce8
#define RTL_IMAGE_TYPE_LOG    0xc371

#define RTL_IMAGE_ROOT_DIR_ENTRY_NAME_MAX_LEN   15
typedef struct rootDirEntry_s {
	char name[RTL_IMAGE_ROOT_DIR_ENTRY_NAME_MAX_LEN+1];
	     /* entry name.
	      * example value:
	      *   "run"
	      *   "web"
	      *   "ccfg"
	      *   "dcfg"
	      *   "log"
	      */
	uint32 startOffset;  /* starting offset
	                      * offset relative to memory map origin
                          * of this file device
	                      * !! not the absolute address in memory map !!
	                      */
	uint32 maxSize;      /* this entry has been allocated an area of size
	                      * 'maxSize' on the flash memory device.
	                      * The size of image header is counted in this maxSize.
	                      * That is, the effective size is actually
	                      * ( maxSize - sizeof(fileImageHeader_t) )
	                      */
	uint32 reserved1;  /* used to store the offset when this file image is loaded
	                    * into this module's local buffer
	                    */
	uint8  filePerm;  /* rtl_fcmn.h: FCMN_FP_R | FCMN_FP_W | FCMN_FP_RW */
	uint8  fileType;  /* rtl_fcmn.h: FCMN_FT_REG_FILE | FCMN_FT_DIR */
	uint16 imageType; /* RTL_IMAGE_TYPE_XXX */
} rootDirEntry_t;


#define RTL_IMAGE_HDR_VER_1_0    1
typedef struct fileImageHeader_s {
    uint32 productMagic; /* Product Magic Number */
	uint16 imageType;   /* RTL_IMAGE_TYPE_XXX */
	uint8  imageHdrVer; /* image header format version */
	uint8  reserved1; /* for 32-bit alignment */
	uint32 date;      /* Image Creation Date (in Network Order)
                       * B1B2:year(0..65535) (BigEndian)
                       * B3:month(1..12)
                       * B4:day(1..31)
                       */
	uint32 time;     /* Image Creation Time (in Network Order)
                      * B1:hour(0..23)
                      * B2:minute(0..59)
                      * B3:second(0..59)
                      */
	uint32 imageLen;      /* image header length not counted in */
	uint16 reserved2;
	uint8 imageBdyCksm;  /* cheacksum cover range: image body */
	uint8 imageHdrCksm;  /* cheacksum cover range: image header */	
} fileImageHeader_t;


#endif /* _RTL_IMAGE_H_ */
