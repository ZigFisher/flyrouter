/* -*- linux-c -*- */
/*
 * at76c505-rfmd2958.c:
 *
 * Driver for at76c505-based devices based on the Atmel "Fast-Vnet" reference
 * design using RFMD radio chips
 * This file is used for the AT76C505 with RFMD 2958 radio.
 * 
 * Copyright (c) 2002 - 2003 Oliver Kurth
 * Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 *
 * This driver is derived from usb-skeleton.c
 *
 * This driver contains code specific to Atmel AT76C505 (USB wireless 802.11)
 * devices which use radio chips from RF Micro Devices (RFMD).  Almost
 * all of the actual driver is handled by the generic at76c503.c module, this
 * file mostly just deals with the initial probes and downloading the correct
 * firmware to the device before handing it off to at76c503.
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
# include "fw-pkg-505-rfmd2958-1.101.0-86.h"
#endif

/* Version Information */

#define DRIVER_NAME "at76c505-rfmd2958"
#define DRIVER_AUTHOR \
"Oliver Kurth <oku@masqmail.cx>, Joerg Albert <joerg.albert@gmx.de>, Alex <alex@foogod.com>"
#define DRIVER_DESC "Atmel at76c505 (RFMD 2958) Wireless LAN Driver"

/* firmware name to load if above include file contains empty fw only */
#define FW_NAME "atmel_" DRIVER_NAME ".bin"

#define BOARDTYPE BOARDTYPE_505_RFMD_2958

/* USB Device IDs supported by this driver */

#define VENDOR_ID_ATMEL               0x03eb
#define PRODUCT_ID_ATMEL_505R2958     0x7613 /* Generic AT76C505/RFMD device 
					       also OvisLink WL-1130USB */

#define VENDOR_ID_CNET                0x1371
#define PRODUCT_ID_CNET_CNUSB611G     0x0013 /* CNet CNUSB 611G */
#define PRODUCT_ID_FL_WL240U          0x0014 /* Fiberline WL-240U with the
                                                 CNet vendor id */
#define VENDOR_ID_LINKSYS             0x1915 
#define PRODUCT_ID_LINKSYS_WUSB11V28  0x2233 /* Linksys WUSB11 v2.8 */

#define VENDOR_ID_XTERASYS            0x12fd
#define PRODUCT_ID_XTERASYS_XN_2122B  0x1001 /* Xterasys XN-2122B, also
					        IBlitzz BWU613B / BWU613SB */

#define VENDOR_ID_COREGA               0x07aa
#define PRODUCT_ID_COREGA_USB_STICK_11_KK 0x7613 /* Corega WLAN USB Stick 11 (K.K.) */

#define VENDOR_ID_MSI                 0x0db0
#define PRODUCT_ID_MSI_MS6978_WLAN_BOX_PC2PC 0x1020

static struct usb_device_id dev_table[] = {
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_ATMEL_505R2958   ) },
	{ USB_DEVICE(VENDOR_ID_CNET,     PRODUCT_ID_FL_WL240U         ) },
	{ USB_DEVICE(VENDOR_ID_CNET,     PRODUCT_ID_CNET_CNUSB611G    ) },
	{ USB_DEVICE(VENDOR_ID_LINKSYS,  PRODUCT_ID_LINKSYS_WUSB11V28 ) },
	{ USB_DEVICE(VENDOR_ID_XTERASYS, PRODUCT_ID_XTERASYS_XN_2122B ) },
	{ USB_DEVICE(VENDOR_ID_COREGA,   PRODUCT_ID_COREGA_USB_STICK_11_KK ) },
        { USB_DEVICE(VENDOR_ID_MSI,      PRODUCT_ID_MSI_MS6978_WLAN_BOX_PC2PC) },
	{ }
};

/* jal: not really good style to include a .c file, but all but the above
   is constant in the at76c50[35]-*.c files ... */
#include "at76c503-fw_skel.c"
