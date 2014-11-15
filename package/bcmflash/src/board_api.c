/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : board_api.c
 *
 * Description: interface for board level calls: flash,
 *              led, soft reset, free memory page, and memory dump.
 *              Adapted form flash_api.c by Yen Tran.
 *
 * Created on : 02/20/2002  seanl
 *
 ***************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <memory.h>

#include "bcmtypes.h"
#include "board_api.h"

#define USE_ALL 1

int boardIoctl(int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset, char *buf)
{
    BOARD_IOCTL_PARMS IoctlParms;
    int boardFd = 0;

    boardFd = open("/dev/brcmboard", O_RDWR);
    if ( boardFd != -1 ) {
        IoctlParms.string = string;
        IoctlParms.strLen = strLen;
        IoctlParms.offset = offset;
        IoctlParms.action = action;
        IoctlParms.buf    = buf;
        ioctl(boardFd, board_ioctl, &IoctlParms);
        close(boardFd);
        boardFd = IoctlParms.result;
    } else
        printf("Unable to open device /dev/brcmboard.\n");

    return boardFd;
}
#ifdef USE_ALL


/*****************************************************************************
* sysScratchPadGet - get the contents of persistent stratch pad flash memory
* INPUT:   tokenId -- token id, ASCIZ tokBuf (up to 15 char)
*          tokBuf  -- buf 
*          bufLen  -- buf len
* RETURNS: 0 - ok, -1 fail.
*/
int sysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD, tokenId, bufLen, bufLen, tokBuf));
}

/*****************************************************************************
* sysScratchPadSet - write the contents of persistent scratch pad flash memory
* INPUT:   tokenId -- token id, ASCIZ tokBuf (up to 15 char)
*          tokBuf  -- buf 
*          bufLen  -- buf len
* RETURNS: 0 - ok, -1 fail.
*/
int sysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD, tokenId, bufLen, bufLen, tokBuf));
}


/*****************************************************************************
* sysPersistentGet - get the contents of non-volatile RAM,
* RETURNS: OK, always.
*/
int sysNvRamGet(char *string, int strLen, int offset)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_READ, NVRAM, string, strLen, offset, ""));
}

/*****************************************************************************
* sysPersistentSet - write the contents of non-volatile RAM
* RETURNS: OK, always.
*/
int sysNvRamSet(char *string, int strLen, int offset)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_WRITE, NVRAM, string, strLen, offset, ""));
}

/*****************************************************************************
* sysNrPagesGet - returns number of free system pages.  Each page is 4K bytes.
* RETURNS: Number of 4K pages.
*/
int sysNrPagesGet(void)
{
    return (boardIoctl(BOARD_IOCTL_GET_NR_PAGES, 0, "", 0, 0, ""));
}

/*****************************************************************************
* sysDumpAddr - Dump kernel memory.
* RETURNS: OK, always.
*/
int sysDumpAddr(char *addr, int len)
{
    return (boardIoctl(BOARD_IOCTL_DUMP_ADDR, 0, addr, len, 0, ""));
}

/*****************************************************************************
* sysDumpAddr - Set kernel memory.
* RETURNS: OK, always.
*/
int sysSetMemory(char *addr, int size, unsigned long value )
{
    return (boardIoctl(BOARD_IOCTL_SET_MEMORY, 0, addr, size, (int) value, ""));
}

/*****************************************************************************
 * image points to image to be programmed to flash; size is the size (in bytes)
 * of the image.
 * if error, return -1; otherwise return 0
 */

#endif // USE_ALL

/*****************************************************************************
* sysPersistentGet - get the contents of persistent flash memory
* RETURNS: OK, always.
*/
int sysPersistentGet(char *string, int strLen, int offset)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_READ, PERSISTENT, string, strLen, offset, ""));
}

/*****************************************************************************
* sysPersistenSet - write the contents of persistent Scrach Pad flash memory
* RETURNS: OK, always.
*/
int sysPersistentSet(char *string, int strLen, int offset)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_WRITE, PERSISTENT, string, strLen, offset, ""));
}

//********************************************************************************
// Get PSI size
//********************************************************************************
int sysGetPsiSize( void )
{
    return( boardIoctl(BOARD_IOCTL_GET_PSI_SIZE, 0, "", 0, 0, "") );
}

int sysFlashImageSet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType)
{
    int result;

    result = boardIoctl(BOARD_IOCTL_FLASH_WRITE, imageType, image, size, addr, "");

    return(result);
}

/*****************************************************************************
 * Get flash size 
 * return int flash size
 */
int sysFlashSizeGet(void)
{
    return (boardIoctl(BOARD_IOCTL_FLASH_READ, FLASH_SIZE, "", 0, 0, ""));
}

/*****************************************************************************
* kerSysMipsSoftReset - soft reset the mips. (reboot, go to 0xbfc00000)
* RETURNS: NEVER
*/
void sysMipsSoftReset(void)
{  
    boardIoctl(BOARD_IOCTL_MIPS_SOFT_RESET, 0, "", 0, 0, "");
}

//********************************************************************************
// Get Chip Id
//********************************************************************************
int sysGetChipId( void )
{
    return( boardIoctl(BOARD_IOCTL_GET_CHIP_ID, 0, "", 0, 0, "") );
}

#ifdef USE_ALL
//********************************************************************************
// LED status display:  ADSL link: DOWN/UP, PPP: DOWN/STARTING/UP
//********************************************************************************
void sysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    boardIoctl(BOARD_IOCTL_LED_CTRL, 0, "", (int)ledName, (int)ledState, "");
}


//********************************************************************************
// Get board id
//********************************************************************************
int sysGetBoardIdName(char *name, int length)
{
    return( boardIoctl(BOARD_IOCTL_GET_ID, 0, name, length, 0, "") );
}


//********************************************************************************
// Get MAC Address
//********************************************************************************
int sysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId )
{
    return(boardIoctl(BOARD_IOCTL_GET_MAC_ADDRESS, 0, pucaAddr, 6, (int) ulId, ""));
}


//********************************************************************************
// Release MAC Address
//********************************************************************************
int sysReleaseMacAddress( unsigned char *pucaAddr )
{
    return( boardIoctl(BOARD_IOCTL_RELEASE_MAC_ADDRESS, 0, pucaAddr, 6, 0, "") );
}


//********************************************************************************
// Get SDRAM size
//********************************************************************************
int sysGetSdramSize( void )
{
    return( boardIoctl(BOARD_IOCTL_GET_SDRAM_SIZE, 0, "", 0, 0, "") );
}


//********************************************************************************
// Get Enet mode flag 
//********************************************************************************
int sysGetEnetModeFlag(void)
{
    return (boardIoctl(BOARD_IOCTL_GET_ENET_MODE_FLAG, 0, "", 0, 0, ""));
}


//********************************************************************************
// Set Enet mode flag 
//********************************************************************************
int sysSetEnetModeFlag(unsigned long value)
{
    return (boardIoctl(BOARD_IOCTL_SET_ENET_MODE_FLAG, 0, "", 0, (int) value, ""));
}


/*****************************************************************************
* sysGetBooline - get bootline
* RETURNS: OK, always.
*/
int sysGetBootline(char *string, int strLen)
{
    return (boardIoctl(BOARD_IOCTL_GET_BOOTLINE, 0, string, strLen, 0, ""));
}

/*****************************************************************************
* sysSetBootline - write the bootline to nvram
* RETURNS: OK, always.
*/
int sysSetBootline(char *string, int strLen)
{
    return (boardIoctl(BOARD_IOCTL_SET_BOOTLINE, 0, string, strLen, 0, ""));
}

//********************************************************************************
// Get MAC Address
//********************************************************************************
int sysGetBaseMacAddress( unsigned char *pucaAddr )
{
    return(boardIoctl(BOARD_IOCTL_GET_BASE_MAC_ADDRESS, 0, pucaAddr, 6, 0, ""));
}

#endif // USE_ALL
