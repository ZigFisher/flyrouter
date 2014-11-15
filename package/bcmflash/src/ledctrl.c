/***************************************************************************
 * File Name  : ledctrl.c
 *
 * Description: 
 *
 *    This application control the led action by calling sysLedCtrl.
 *    smaple calls:     sysLedCtrl(kLedAdsl, kLedStateBlinkOnce);           // kLedxxx defines in board_api.h
 *
 * Created on :  11/01/2002  seanl
 *
 ***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>  /* ioctl */
#include "board.h"



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

// ******* end board_api.c/h

typedef struct
{
    char *validLedName;
    BOARD_LED_NAME ledName;
} LED_NAME_MAP, *PLED_NAME_MAP;


// LED name as the first param
const LED_NAME_MAP ledNameMap[] =
{
    {"Adsl",         kLedAdsl},
    {"Wireless",     kLedWireless},
    {"Usb",          kLedUsb},
    {"Hpna",         kLedHpna},
    {"WanData",      kLedWanData},
    {"PPP",          kLedPPP},
    {"Voip",         kLedVoip},
//    {"Diag",         kLedDiag},
//    {"Pstn",         kLedPSTN},
//    {"Power",        kLedPower},
    {NULL,           kLedEnd},             
};

typedef struct 
{
    char *validLedState;
    BOARD_LED_STATE ledState;
} LED_STATE_MAP, *PLED_STATE_MAP;

const LED_STATE_MAP ledStateMap[] =
{
    {"Off",                 kLedStateOff},               
    {"On",                  kLedStateOn},                
    {"BlinkOnce",           kLedStateBlinkOnce},         
    {"SlowBlinkContinues",  kLedStateSlowBlinkContinues},
    {"FastBlinkContinues",  kLedStateFastBlinkContinues},
    {NULL,                  -1},
};  

void showUsage()
{
    PLED_NAME_MAP namePtr = ledNameMap;
    PLED_STATE_MAP statePtr = ledStateMap;

    printf("\nUsage: ledctrl led_name led_state\n\n");
    printf("Supported led names:\n");
    while (namePtr->validLedName)
        printf("%s\n", namePtr++->validLedName);
    
    printf("\nSupported led states:\n");
    while (statePtr->validLedState)
        printf("%s\n", statePtr++->validLedState);

    exit(-1);
}

int main(int argc, char **argv)
{
    PLED_NAME_MAP namePtr = ledNameMap;
    PLED_STATE_MAP statePtr = ledStateMap;

    if (argc < 3)
        showUsage();

    while (namePtr->validLedName)
        if (strcmp(namePtr->validLedName, argv[1]) == 0)
            break;
        else
            namePtr++;
    if (!namePtr->validLedName)
        showUsage();

    while (statePtr->validLedState)
        if (strcmp(statePtr->validLedState, argv[2]) == 0)
            break;
        else
            statePtr++;

    if (!statePtr->validLedState)
        showUsage();

    sysLedCtrl(namePtr->ledName, statePtr->ledState);

	return 0;

}
