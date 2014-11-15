/* -*- linux-c -*- */
/*
 * $Id: at76c503-rfmd.c,v 1.25 2004/08/18 22:01:45 jal2 Exp $
 *
 * Driver for at76c503-based devices based on the Atmel "Fast-Vnet" reference
 * design using RFMD radio chips
 *
 * Copyright (c) 2002 - 2003 Oliver Kurth
 * Changes Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 *
 * This driver is derived from usb-skeleton.c
 *
 * This driver contains code specific to Atmel AT76C503 (USB wireless 802.11)
 * devices which use radio chips from RF Micro Devices (RFMD).  Almost
 * all of the actual driver is handled by the generic at76c503.c module, this
 * file just registers for the USB ids and passes the correct firmware to
 * at76c503.
 *
 * History:
 *
 * 2003_02_11 0.1: (alex)
 * - split board-specific code off from at76c503.c
 * - reverted to 0.90.2 firmware because 0.100.x is broken for WUSB11
 *
 * 2003_02_18 0.2: (alex)
 * - Reduced duplicated code and moved as much as possible into at76c503.c
 * - Changed default netdev name to "wlan%d"
 */

#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/init.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,23)
#include <linux/firmware.h>
#else
# ifdef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
#  error firmware download from user space unavail. with this kernel
# endif
# define FIRMWARE_NAME_MAX 30 
struct firmware {
	size_t size;
	u8 *data;
};
#endif

#include "at76c503.h"

/* Include firmware data definition: a dummy or a statically compiled-in fw */
#ifdef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
/* a dummy struct to use if at76c503-*.o shall load the firmware via hotplug */
static struct firmware static_fw = {0,NULL};
#else
# include "fw-pkg-rfmd-1.101.0-84.h"
#endif

/* Version Information */

#define DRIVER_NAME "at76c503-rfmd"
#define DRIVER_AUTHOR \
"Oliver Kurth <oku@masqmail.cx>, Joerg Albert <joerg.albert@gmx.de>, Alex <alex@foogod.com>"
#define DRIVER_DESC "Atmel at76c503 (RFMD) Wireless LAN Driver"

/* firmware name to load if above include file contains empty fw only */
#define FW_NAME "atmel_" DRIVER_NAME ".bin"

#define BOARDTYPE BOARDTYPE_503_RFMD

/* USB Device IDs supported by this driver */

#define VENDOR_ID_ATMEL               0x03eb
#define PRODUCT_ID_ATMEL_503R         0x7605 /* Generic AT76C503/RFMD device */
#define PRODUCT_ID_W_BUDDIE_WN210     0x4102 /* AirVast W-Buddie WN210 */

#define VENDOR_ID_DYNALINK            0x069a
#define PRODUCT_ID_DYNALINK_WLL013_R  0x0321 /* Dynalink/Askey WLL013 (rfmd) */

#define VENDOR_ID_LINKSYS             0x077b
#define PRODUCT_ID_LINKSYS_WUSB11_V26 0x2219 /* Linksys WUSB11 v2.6 */
#define PRODUCT_ID_NE_NWU11B          0x2227 /* Network Everywhere NWU11B */

#define VENDOR_ID_NETGEAR             0x0864
#define PRODUCT_ID_NETGEAR_MA101B     0x4102 /* Netgear MA 101 Rev. B */

#define VENDOR_ID_ACTIONTEC           0x1668
#define PRODUCT_ID_ACTIONTEC_802UAT1  0x7605 /* Actiontec 802UAT1, HWU01150-01UK */

#define VENDOR_ID_DLINK               0x2001 /* D-Link */
#define PRODUCT_ID_DLINK_DWL120       0x3200 /* DWL-120 rev. E */

#define VENDOR_ID_DICK_SMITH_ELECTR   0x1371 /* Dick Smith Electronics */
#define PRODUCT_ID_DSE_XH1153         0x5743 /* XH1153 802.11b USB adapter */
                                             /* also: CNet CNUSB611 (D) */
#define PRODUCT_ID_WL_200U            0x0002 /* WL-200U */

#define VENDOR_ID_BENQ                0x04a5 /* BenQ (Acer) */
#define PRODUCT_ID_BENQ_AWL_400       0x9001 /* BenQ AWL-400 USB stick */

#define VENDOR_ID_3COM                0x506
#define PRODUCT_ID_3COM_3CRSHEW696    0xa01 /* 3COM 3CRSHEW696 */

#define VENDOR_ID_SIEMENS             0x681
#define PRODUCT_ID_SIEMENS_SANTIS_WLL013 0x1b /* Siemens Santis ADSL WLAN 
						 USB adapter WLL 013 */

#define VENDOR_ID_BELKIN_2		0x50d
#define PRODUCT_ID_BELKIN_F5D6050_V2	0x50	/* Belkin F5D6050, version 2 */


#define VENDOR_ID_BLITZ                 0x07b8  
#define PRODUCT_ID_BLITZ_NETWAVE_BWU613 0xb000 /* iBlitzz, BWU613 (not *B or *SB !) */

#define VENDOR_ID_GIGABYTE              0x1044  
#define PRODUCT_ID_GIGABYTE_GN_WLBM101  0x8003 /* Gigabyte GN-WLBM101 */

#define VENDOR_ID_PLANEX                0x2019
#define PRODUCT_ID_PLANEX_GW_US11S      0x3220 /* Planex GW-US11S */

#define VENDOR_ID_COMPAQ                0x049f
#define PRODUCT_ID_IPAQ_INT_WLAN        0x0032 /* internal wlan adapter in h5[4,5]xx series iPAQs */

static struct usb_device_id dev_table[] = {
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_ATMEL_503R        ) },
	{ USB_DEVICE(VENDOR_ID_DYNALINK, PRODUCT_ID_DYNALINK_WLL013_R ) },
	{ USB_DEVICE(VENDOR_ID_LINKSYS,  PRODUCT_ID_LINKSYS_WUSB11_V26) },
	{ USB_DEVICE(VENDOR_ID_LINKSYS,  PRODUCT_ID_NE_NWU11B         ) },
	{ USB_DEVICE(VENDOR_ID_NETGEAR,  PRODUCT_ID_NETGEAR_MA101B    ) },
	{ USB_DEVICE(VENDOR_ID_DLINK,    PRODUCT_ID_DLINK_DWL120      ) },
	{ USB_DEVICE(VENDOR_ID_ACTIONTEC,PRODUCT_ID_ACTIONTEC_802UAT1 ) },
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_W_BUDDIE_WN210    ) },
	{ USB_DEVICE(VENDOR_ID_DICK_SMITH_ELECTR, PRODUCT_ID_DSE_XH1153) },
	{ USB_DEVICE(VENDOR_ID_DICK_SMITH_ELECTR, PRODUCT_ID_WL_200U) },
	{ USB_DEVICE(VENDOR_ID_BENQ,     PRODUCT_ID_BENQ_AWL_400) },
	{ USB_DEVICE(VENDOR_ID_3COM, PRODUCT_ID_3COM_3CRSHEW696) },
	{ USB_DEVICE(VENDOR_ID_SIEMENS,  PRODUCT_ID_SIEMENS_SANTIS_WLL013) },
	{ USB_DEVICE(VENDOR_ID_BELKIN_2, PRODUCT_ID_BELKIN_F5D6050_V2 ) },
        { USB_DEVICE(VENDOR_ID_BLITZ,    PRODUCT_ID_BLITZ_NETWAVE_BWU613 ) },
        { USB_DEVICE(VENDOR_ID_GIGABYTE, PRODUCT_ID_GIGABYTE_GN_WLBM101 ) },
        { USB_DEVICE(VENDOR_ID_PLANEX,   PRODUCT_ID_PLANEX_GW_US11S ) },
        { USB_DEVICE(VENDOR_ID_COMPAQ,   PRODUCT_ID_IPAQ_INT_WLAN) },
	{ }
};


/* jal: not really good style to include a .c file, but all but the above
   is constant in the at76c503-*.c files ... */
#include "at76c503-fw_skel.c"
