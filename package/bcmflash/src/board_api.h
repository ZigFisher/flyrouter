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
 * File Name  : board_api.h (adapted from flash_api.h by Yen Tran)
 *
 * Created on :  02/20/2002  seanl
 ***************************************************************************/

#if !defined(_BOARD_API_H_)
#define _BOARD_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_BOARD_H)

// for the action in BOARD_IOCTL_PARMS for flash operation
typedef enum 
{
    PERSISTENT = 3,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
	BCM_IMAGE_TECOM,
} BOARD_IOCTL_ACTION;  
    
typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;


// LED defines 
typedef enum
{   
    kLedAdsl,
    kLedWireless,
    kLedUsb,
    kLedHpna,
    kLedWanData,
    kLedPPP,
    kLedVoip,
    kLedEnd,                // NOTE: Insert the new led name before this one.  Alway stay at the end.
} BOARD_LED_NAME;

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateBlinkOnce,                  /* blink once, ~100ms and ignore the same call during the 100ms period */
    kLedStateSlowBlinkContinues,         /* slow blink continues at ~600ms interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at ~200ms interval */
} BOARD_LED_STATE;


/* GPIO Definitions */
#define GPIO_BOARD_ID_1             0x0020
#define GPIO_BOARD_ID_2             0x0040
#define GPIO_BOARD_ID_3             0x0080

/* Identify BCM96345 board type by checking GPIO bits.
 * GPIO bit 7 6 5    Board type
 *          0 0 0    Undefined
 *          0 0 1    Undefined
 *          0 1 0    GW
 *          0 1 1    USB
 *          1 0 0    R 1.0
 *          1 0 1    I
 *          1 1 0    SV
 *          1 1 1    R 0.0
 */
#define BOARD_ID_BCM9634X_MASK  (GPIO_BOARD_ID_1|GPIO_BOARD_ID_2|GPIO_BOARD_ID_3) 
#define BOARD_ID_BCM96345SV     (GPIO_BOARD_ID_2|GPIO_BOARD_ID_3)
#define BOARD_ID_BCM96345R00    (GPIO_BOARD_ID_1|GPIO_BOARD_ID_2|GPIO_BOARD_ID_3)
#define BOARD_ID_BCM96345I      (GPIO_BOARD_ID_1|GPIO_BOARD_ID_3)
#define BOARD_ID_BCM96345R10    (GPIO_BOARD_ID_3)
#define BOARD_ID_BCM96345USB    (GPIO_BOARD_ID_1|GPIO_BOARD_ID_2)
#define BOARD_ID_BCM96345GW     (GPIO_BOARD_ID_2)

/* Defines. for board driver */
#define BOARD_IOCTL_MAGIC       'B'
#define BOARD_DRV_MAJOR          206

#define BOARD_IOCTL_FLASH_INIT \
    _IOWR(BOARD_IOCTL_MAGIC, 0, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_WRITE \
    _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_READ \
    _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_NR_PAGES \
    _IOWR(BOARD_IOCTL_MAGIC, 3, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_DUMP_ADDR \
    _IOWR(BOARD_IOCTL_MAGIC, 4, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MEMORY \
    _IOWR(BOARD_IOCTL_MAGIC, 5, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_MIPS_SOFT_RESET \
    _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_LED_CTRL \
    _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 8, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 9, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_RELEASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 10, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_PSI_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_SDRAM_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 12, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ENET_MODE_FLAG \
    _IOWR(BOARD_IOCTL_MAGIC, 13, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_ENET_MODE_FLAG \
    _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 15, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 16, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_BASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 17, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CHIP_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)

#endif

int sysScratchPadSet(char *tokenId, char *tokBuf, int bufLen);
int sysScratchPadGet(char *tokenId, char *tokBuf, int bufLen);
int sysPersistentGet(char *string,int strLen,int offset);
int sysPersistentSet(char *string,int strLen,int offset);
int sysNvRamSet(char *string,int strLen,int offset);
int sysNvRamGet(char *string,int strLen,int offset);
void sysFlashImageInit(void);
int sysFlashImageGet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType);
int sysFlashImageSet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType);
int sysNrPagesGet(void);
int sysDumpAddr(char *addr, int len);
int sysSetMemory(char *addr, int size, unsigned long value );
void sysMipsSoftReset(void);
int sysGetBoardIdName(char *name, int length);
int sysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId );
int sysReleaseMacAddress( unsigned char *pucaAddr );
int sysGetSdramSize( void );
int sysGetPsiSize( void );
int sysGetEnetModeFlag(void);
int sysSetEnetModeFlag(unsigned long);
int sysGetBootline(char *string,int strLen);
int sysSetBootline(char *string,int strLen);
void sysLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
int sysFlashSizeGet(void);
int sysGetBaseMacAddress(unsigned char *pucaAddr);
int sysGetChipId(void);
#if defined(__cplusplus)
}
#endif

#endif /* _BOARD_API_H_ */
