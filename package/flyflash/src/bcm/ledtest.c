#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"

#define BOARD_IOCTL_MAGIC       'B'
#define BOARD_IOCTL_LED_CTRL _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)
#define BP_GPIO_7       0x0080

typedef enum
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
} BOARD_IOCTL_ACTION;

typedef enum
{
    kLedAdsl,
    kLedWireless,
    kLedUsb,
    kLedHpna,
    kLedWanData,
    kLedPPP,
    kLedVoip,
    kLedDiag,
    kPSTN,
    kLedPower,
    kLedEnd,                // NOTE: Insert the new led name before this one.  Alway stay at the end.
} BOARD_LED_NAME;           //Michael: this sequence also has to sync with conf45/inc/board_api.h

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateBlinkOnce,                  /* blink once, ~100ms and ignore the same call during the 100ms period */
    kLedStateSlowBlinkContinues,         /* slow blink continues at ~600ms interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at ~200ms interval */
} BOARD_LED_STATE;

typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;


static int boardIoctl(int boardFd, int board_ioctl, BOARD_IOCTL_ACTION action, char *string, int strLen, int offset)
{
    BOARD_IOCTL_PARMS IoctlParms;
    IoctlParms.string = string;
    IoctlParms.strLen = strLen;
    IoctlParms.offset = offset;
    IoctlParms.action = action;
    ioctl(boardFd, board_ioctl, &IoctlParms);
    return (IoctlParms.result);
}

static void sysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    int boardFd;
    if ((boardFd = open("/dev/brcmboard", O_RDWR)) == -1)
        printf("Unable to open device /dev/brcmboard.\n");
    boardIoctl(boardFd, BOARD_IOCTL_LED_CTRL, 0, "", (int)ledName, (int)ledState);
    close(boardFd);
}

int main(int argc, char * argv[]) {

  if (argc != 2) {
        printf("Error! Wrong number of arguments! 1 means on and 0 means off.\n");
  } else {
        if (strcmp(argv[1],"1") == 0)
                sysLedCtrl(kLedUsb, kLedStateOn);
        if (strcmp(argv[1],"0") == 0)
                sysLedCtrl(kLedUsb, kLedStateOff);
  }
  return 0;
}
