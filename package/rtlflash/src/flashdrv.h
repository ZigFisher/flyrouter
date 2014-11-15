/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/cvsroot/uClinux-dist/user/goahead-2.1.4/LINUX/flashdrv.h,v 1.13 2004/09/02 08:54:14 yjlou Exp $
 *
 * $Author: yjlou $
 *
 * Abstract:
 *
 *   Flash driver header file for export include.
 *
 * $Log: flashdrv.h,v $
 * Revision 1.13  2004/09/02 08:54:14  yjlou
 * *: fixed the bug of flashdrv_isOver5xBBootInstructions(): direct access CRMR in MMU user space is NOT allowed.
 *
 * Revision 1.12  2004/08/26 13:20:00  yjlou
 * *: Loader upgrades to "00.00.12".
 * +: support "Loader Segment Descriptors Table"
 * -: remove romcopystart/pause/resume
 *
 * Revision 1.11  2004/08/20 05:26:00  danwu
 * + Add support for EON EN29LV800AB
 *
 * Revision 1.10  2004/08/11 04:00:17  yjlou
 * *: _flash_init() is renamed to flashdrv_init()
 *
 * Revision 1.9  2004/08/10 12:57:03  yjlou
 * +: add flashdrv prefix for the following functions:
 *      getBoardInfoAddr(void);
 *      uint32 getCcfgImageAddr(void);
 *      uint32 getRunImageAddr(void);
 *
 * Revision 1.8  2004/08/10 12:13:21  yjlou
 * *: modify to support kernel mode
 *
 * Revision 1.7  2004/08/04 14:48:56  yjlou
 * *: merge rtl_bdinfo.h into flashdrv.h
 *
 * Revision 1.6  2004/07/27 10:50:21  cfliu
 * no message
 *
 * Revision 1.5  2004/05/21 11:40:33  yjlou
 * *: fixed the bug of INTEL flash (clear status register before program or erase flash)
 *
 * Revision 1.4  2004/05/14 03:12:47  yjlou
 * +: support FUJI flash
 *
 * Revision 1.3  2004/05/13 13:27:01  yjlou
 * +: loader version is migrated to "00.00.07".
 * +: new architecture for INTEL flash (code is NOT verified).
 * *: FLASH_BASE is decided by IS_REV_A()
 * -: remove flash_map.h (content moved to flashdrv.h)
 * -: remove un-necessary calling setIlev()
 *
 * Revision 1.1.1.1  2003/08/27 06:20:15  rupert
 * uclinux initial
 *
 * Revision 1.1.1.1  2003/08/27 03:08:53  rupert
 *  initial version 
 *
 * Revision 1.3  2003/06/23 11:11:04  elvis
 * change include path of  rtl_types.h
 *
 * Revision 1.2  2003/05/20 08:52:24  elvis
 * change the include path of "rtl_types.h"
 *
 * Revision 1.1  2003/04/29 14:17:05  orlando
 * flashdrv module initial check-in (used by cfgmgr), ported from srcroot.
 *
 * Revision 1.1  2002/07/19 08:39:33  danwu
 * Create file.
 *
 *
 * 
 */

#ifndef _FLASHDRV_H_
#define _FLASHDRV_H_



#if defined(__KERNEL__) && defined(__linux__)
	/* kernel mode */
	#include "rtl865x/rtl_types.h"
	#include "rtl_flashdrv.h"
	#include "linux/sched.h" /* for jiffies */
	#include "board.h"
	#include "rtl865x/asicRegs.h"
	#define _KERNEL_MODE_FLASHDRV_
#else
	#ifdef LOADER
		/* Loader */
		#include "rtl_types.h"
		#include "rtl_flashdrv.h"
		#include "board.h"
		#include <asicregs.h>
		#define rtlglue_printf printf
		#define _LOADER_MODE_FLASHDRV_
	#else
		/* runtime code */
		#include "rtl_types.h"
		#include "rtl_flashdrv.h"
		#include <version.h>
		#include <linux/autoconf.h>
		#include "board.h"
		#include <asicRegs.h>
		#define _USER_MODE_FLASHDRV_
	#endif
#endif



/* Command Definitions 
*/
#define AM29LVXXX_COMMAND_ADDR1         ((volatile uint16 *) (FLASH_BASE + 0x555 * 2))
#define AM29LVXXX_COMMAND_ADDR2         ((volatile uint16 *) (FLASH_BASE + 0x2AA * 2))
#define AM29LVXXX_COMMAND1              0xAA
#define AM29LVXXX_COMMAND2              0x55
#define AM29LVXXX_SECTOR_ERASE_CMD1     0x80
#define AM29LVXXX_SECTOR_ERASE_CMD2     0x30
#define AM29LVXXX_PROGRAM_CMD           0xA0

/* INTEL command set */
#define IN28FXXX_READ_ARRAY             0xFF
#define IN28FXXX_READ_ID                0x90
#define IN28FXXX_READ_STATUS            0x70
#define IN28FXXX_CLEAR_STATUS           0x50
#define IN28FXXX_PROGRAM                0x40
#define IN28FXXX_ERASE_BLOCK1           0x20
#define IN28FXXX_ERASE_BLOCK2           0xD0
#define IN28FXXX_UNLOCK_BLOCK1          0x60
#define IN28FXXX_UNLOCK_BLOCK2          0xD0



/* for using gdb loader, which assumes different flash map than
 * rome loader
 */
//#define GDB_LOADER 



#ifdef CONFIG_RTL865X_CUSTOM_FLASH_MAP
// User has specified flash map in 'make menuconfig'
#define FLASH_MAP_BOARD_INFO_ADDR         (FLASH_BASE+CONFIG_RTL865X_CUSTOM_BDINFO_ADDRESS)
#define FLASH_MAP_CCFG_IMAGE_ADDR         (FLASH_BASE+CONFIG_RTL865X_CUSTOM_CCFG_ADDRESS)
#define FLASH_MAP_RUN_IMAGE_ADDR          (FLASH_BASE+CONFIG_RTL865X_CUSTOM_RUNTIME_ADDRESS)
#else//CONFIG_RTL865X_CUSTOM_FLASH_MAP
// User does not specify. Flash driver will auto detect.
#define FLASH_MAP_BOARD_INFO_ADDR         (flashdrv_getBoardInfoAddr())
#define FLASH_MAP_CCFG_IMAGE_ADDR         (flashdrv_getCcfgImageAddr())
#define FLASH_MAP_RUN_IMAGE_ADDR          (flashdrv_getRunImageAddr())
#endif//CONFIG_RTL865X_CUSTOM_FLASH_MAP

//
//	For compatibility between top and bottom flashes,
//	  we must spare an area at 0x4000~0x8000.
//
#define FLASH_ROMCOPYSTART 0x00000
#define FLASH_ROMCOPYPAUSE 0x04000
#define FLASH_ROMCOPYRESUME 0x08000

#define MAX_FLASH_CHIPS 2
struct FLASH_CHIP_INFO
{
	const char* Type;
	uint32 ChipSize;			/* total size of this flash chip, ex: 0x100000 for 1MB ... */
	uint32 BlockBase;			/* the start address of this block, ex: 0xbfc00000, 0xbfe00000 ... */
	const uint32 *BlockOffset;	/* sector addresss table */
	uint32 NumOfBlock;			/* number of sector */
	uint16 devId;				/* ID of this flash chip */
	uint16 vendorId;			/* ID of the manufacture */
	uint32 BoardInfo;			/* offset of board info */
	uint32 CGIConfig;			/* offset of web config */
	uint32 RuntimeCode;			/* offset of runtime code */
	uint32 isBottom:1;			/* bottom flash */
	uint32 isTop:1;				/* top flash */
};

// definition for vendorID
#define VENDOR_UNKNOWN		0x0000
#define VENDOR_AMD		0x0001
#define VENDOR_FUJI		0x0004
#define VENDOR_ST		0x0020
#define VENDOR_MXIC		0x00C2	/* Macronix */
#define VENDOR_INTEL		0x0089
#define VENDOR_EON		0x7f1c


extern struct FLASH_CHIP_INFO flash_chip_info[MAX_FLASH_CHIPS];
void flashdrv_init(void);
uint32 flashdrv_read(void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size);
uint32 flashdrv_getBoardInfoAddr(void);
uint32 flashdrv_getCcfgImageAddr(void);
uint32 flashdrv_getRunImageAddr(void);
uint32 flashdrv_getFlashBase(void);


/* Loader Segment Descriptor Table */
#define LDR_SEG_DESC_TABLE_OFFSET	0x100      /* offset to flash base */
#define LDR_SEG_DESC_MAGIC			0x57888651 /* magic number */
struct LDR_SEG_DESC
{
	int32 start;
	int32 end;
};


/****************************************************************
 *  From rtl_bdinfo.h.
 */
typedef struct bdinfo_s {
	uint8  mac[6];				// mac address
	uint8  rev[2];				// aligin
	//uint32 macNbr;            /* mac address number */
	uint32 ramStartAddress;
	uint32 rootStartOffset;
	uint32 rootMaxSize;
	uint32 bootSequence;		// 0:normal, 1:debug, 2:L2 switch mode(50A), 3:L2 switch mode(50B)
								// 4:hub mode
	uint32 BackupInst[2];       // Backup Instructions stored at 0xbe400000 or 0xbe800000.
} bdinfo_t; 

extern bdinfo_t bdinfo;

/* exported function prototypes */
int32 cmdBdinfo(uint32 channel, int32 argc, int8 ** argv);

/* function: bdinfo_getMac
 * return  0  when successful
 *         -1 when fail
 * note: caller should allocate buffer for mac_P
 */
int32 bdinfo_getMac(macaddr_t *mac_P);

/* function: bdinfo_getMacNbr
 * return  0  when successful
 *         -1 when fail
 * note: caller should allocate buffer for pMacNbr
 */
int32 bdinfo_getMacNbr(uint32 * pMacNbr);

/* function: bdinfo_setMac
 * return  0  when successful
 *         -1 when fail
 * note: caller should allocate buffer for mac_P
 */
int32 bdinfo_setMac(macaddr_t *mac_P);

/*  end of rtl_bdinfo.h.
 ********************************************************************
 */

#endif  /* _FLASHDRV_H_ */

