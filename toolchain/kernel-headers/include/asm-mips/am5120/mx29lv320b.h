/*****************************************************************************
;
;   (C) Unpublished Work of ADMtek Incorporated.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF ADMTEK INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) ADMTEK EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN ADMTEK WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ADMTEK.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : ADM5120
;    Creator : 
;    File    : include/asm/am5120/mx29lv320b.h
;    Date    : 2003.07.30
;    Abstract: 
;
;Modification History:
; 
;
;*****************************************************************************/


#ifndef  __MX29LV320B_H__
#define  __MX29LV320B_H__

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#define FLASH_PHYS_ADDR 	0x1FC00000

#include<asm/am5120/mx_parts.h>

#define FLASH_PART_ROOT_ADDR (FLASH_PART_KERNEL_ADDR + FLASH_PART_KERNEL_SIZE)
#define FLASH_PART_DATA_ADDR (FLASH_PART_ROOT_ADDR + FLASH_PART_ROOT_SIZE)
#define FLASH_PART_DATA_SIZE (FLASH_SIZE - FLASH_PART_DATA_ADDR)


struct mtd_partition mx29lv320b_parts[] = {
	{
		.name =		"Boot Partition",
		.offset =	FLASH_PART_BOOT_ADDR,
		.size =		FLASH_PART_BOOT_SIZE,
		.mask_flags =   MTD_WRITEABLE
	},
	{
		.name =		"Kernel",
		.offset =	FLASH_PART_KERNEL_ADDR,
		.size =		FLASH_PART_KERNEL_SIZE
	},
	{
		.name =		"Root",
		.offset =	FLASH_PART_ROOT_ADDR,
		.size =		FLASH_PART_ROOT_SIZE
	},
	{
		.name =		"Data",
		.offset =	FLASH_PART_DATA_ADDR,
		.size =		FLASH_PART_DATA_SIZE
	},
	{
		.name =		"WholeFlash",
		.offset =	FLASH_PART_KERNEL_ADDR,
		.size =		0x1F0000
	}
};

#define PARTITION_COUNT (sizeof(mx29lv320b_parts)/sizeof(struct mtd_partition))

#endif /* __MX29LV320B_H__ */

