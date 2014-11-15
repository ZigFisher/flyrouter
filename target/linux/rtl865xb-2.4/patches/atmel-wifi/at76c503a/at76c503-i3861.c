/* -*- linux-c -*- */
/*
 * $Id: at76c503-i3861.c,v 1.20 2004/08/18 22:01:45 jal2 Exp $
 *
 * Driver for at76c503-based devices based on the Atmel "Fast-Vnet" reference
 * design using Intersil 3861 radio chips
 *
 * Copyright (c) 2002 - 2003 Oliver Kurth
 * Changes Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 *
 * This driver is derived from usb-skeleton.c
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 *
 * This driver contains code specific to Atmel AT76C503 (USB wireless 802.11)
 * devices which use a Intersil 3861 radio chip.  Almost
 * all of the actual driver is handled by the generic at76c503.c module, this
 * file just registers for the USB ids and passes the correct firmware to
 * at76c503.
 *
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
# include "fw-pkg-i3861.h"
#endif

/* Version Information */

#define DRIVER_NAME "at76c503-i3861"
#define DRIVER_AUTHOR \
"Oliver Kurth <oku@masqmail.cx>, Joerg Albert <joerg.albert@gmx.de>, Alex <alex@foogod.com>"
#define DRIVER_DESC "Atmel at76c503 (i3861) Wireless LAN Driver"

/* firmware name to load if above include file contains empty fw only */
#define FW_NAME "atmel_" DRIVER_NAME ".bin"

#define BOARDTYPE BOARDTYPE_503_INTERSIL_3861

#define VENDOR_ID_ATMEL               0x03eb
#define PRODUCT_ID_ATMEL_503I         0x7603 /* Generic AT76C503/3861 device */

#define VENDOR_ID_LINKSYS             0x066b
#define PRODUCT_ID_LINKSYS_WUSB11_V21 0x2211 /* Linksys WUSB11 v2.1/v2.6 */

#define VENDOR_ID_NETGEAR             0x0864
#define PRODUCT_ID_NETGEAR_MA101A     0x4100 /* Netgear MA 101 Rev. A */

#define VENDOR_ID_TEKRAM              0x0b3b
#define PRODUCT_ID_TEKRAM_U300C       0x1612 /* Tekram U-300C / Allnet ALL0193 */

#define VENDOR_ID_HP                  0x03f0
#define PRODUCT_ID_HP_HN210W          0x011c /* HP HN210W PKW-J7801A */

#define VENDOR_ID_M4Y750              0x0cde /* Unknown Vendor ID! */
#define PRODUCT_ID_M4Y750             0x0001 /* Sitecom/Z-Com/Zyxel M4Y-750 */

#define VENDOR_ID_DYNALINK            0x069a
#define PRODUCT_ID_DYNALINK_WLL013_I  0x0320 /* Dynalink/Askey WLL013 (intersil) */

#define VENDOR_ID_SMC                 0x0d5c
#define PRODUCT_ID_SMC2662W_V1        0xa001 /* EZ connect 11Mpbs
Wireless USB Adapter SMC2662W (v1) */

#define VENDOR_ID_BENQ                0x4a5 /* BenQ (Acer) */
#define PRODUCT_ID_BENQ_AWL_300       0x9000 /* AWL-300 */

/* this adapter contains flash */
#define VENDOR_ID_ADDTRON             0x05dd  /* Addtron */
#define PRODUCT_ID_ADDTRON_AWU120     0xff31 /* AWU-120 */
/* also Compex WLU11 */

#define VENDOR_ID_INTEL               0x8086 /* Intel */
#define PRODUCT_ID_INTEL_AP310        0x0200 /* AP310 AnyPoint II usb */

#define VENDOR_ID_CONCEPTRONIC        0x0d8e
#define PRODUCT_ID_CONCEPTRONIC_C11U  0x7100 /* also Dynalink L11U */

#define VENDOR_ID_ARESCOM		0xd8e
#define PRODUCT_ID_WL_210		0x7110 /* Arescom WL-210, 
						  FCC id 07J-GL2411USB */
#define VENDOR_ID_IO_DATA		0x04bb
#define PRODUCT_ID_IO_DATA_WN_B11_USB   0x0919 /* IO-DATA WN-B11/USB */

#define VENDOR_ID_BT            0x069a
#define PRODUCT_ID_BT_VOYAGER_1010  0x0821 /* BT Voyager 1010 */


static struct usb_device_id dev_table[] = {
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_ATMEL_503I        ) },
	{ USB_DEVICE(VENDOR_ID_LINKSYS,  PRODUCT_ID_LINKSYS_WUSB11_V21) },
	{ USB_DEVICE(VENDOR_ID_NETGEAR,  PRODUCT_ID_NETGEAR_MA101A    ) },
	{ USB_DEVICE(VENDOR_ID_TEKRAM,   PRODUCT_ID_TEKRAM_U300C      ) },
	{ USB_DEVICE(VENDOR_ID_HP,       PRODUCT_ID_HP_HN210W         ) },
	{ USB_DEVICE(VENDOR_ID_M4Y750,   PRODUCT_ID_M4Y750            ) },
	{ USB_DEVICE(VENDOR_ID_DYNALINK, PRODUCT_ID_DYNALINK_WLL013_I ) },
	{ USB_DEVICE(VENDOR_ID_SMC,      PRODUCT_ID_SMC2662W_V1       ) },
	{ USB_DEVICE(VENDOR_ID_BENQ,     PRODUCT_ID_BENQ_AWL_300      ) },
	{ USB_DEVICE(VENDOR_ID_ADDTRON,  PRODUCT_ID_ADDTRON_AWU120    ) },
	{ USB_DEVICE(VENDOR_ID_INTEL,    PRODUCT_ID_INTEL_AP310       ) },
	{ USB_DEVICE(VENDOR_ID_CONCEPTRONIC,PRODUCT_ID_CONCEPTRONIC_C11U) },
	{ USB_DEVICE(VENDOR_ID_ARESCOM, PRODUCT_ID_WL_210) },
	{ USB_DEVICE(VENDOR_ID_IO_DATA, PRODUCT_ID_IO_DATA_WN_B11_USB) },
	{ USB_DEVICE(VENDOR_ID_BT,       PRODUCT_ID_BT_VOYAGER_1010   ) },
	{ }
};

/* jal: not really good style to include a .c file, but all but the above
   is constant in the at76c503-*.c files ... */
#include "at76c503-fw_skel.c"
