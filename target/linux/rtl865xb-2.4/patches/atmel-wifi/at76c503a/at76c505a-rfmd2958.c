/* -*- linux-c -*- */
/*
 * $Id: at76c505a-rfmd2958.c,v 1.2 2004/08/18 22:01:45 jal2 Exp $
 *
 * Driver for at76c503-based devices based on the Atmel "Fast-Vnet" reference
 * design using a at76c505a (with RFMD radio chips ?)
 *
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
 * This driver contains code specific to Atmel AT76C505A (USB wireless 802.11)
 * devices which use radio chips from RF Micro Devices (RFMD).  Almost
 * all of the actual driver is handled by the generic at76c503.c module, this
 * file just registers for the USB ids and passes the correct firmware to
 * at76c503.
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
# include "fw-pkg-505a-rfmd2958-1.102.0-113.h"
#endif

/* Version Information */

#define DRIVER_NAME "at76c505a-rfmd2958"
#define DRIVER_AUTHOR \
"Joerg Albert <joerg.albert@gmx.de>"
#define DRIVER_DESC "Atmel at76c505a (RFMD2958) Wireless LAN Driver"

#define BOARDTYPE BOARDTYPE_505A_RFMD_2958

/* firmware name to load if above include file contains empty fw only */
#define FW_NAME "atmel_" DRIVER_NAME ".bin"

/* USB Device IDs supported by this driver */

#define VENDOR_ID_ATMEL               0x03eb
#define PRODUCT_ID_ATMEL_505A         0x7614 /* Generic AT76C505A device */
#define PRODUCT_ID_ATMEL_505AS        0x7617 /* Generic AT76C505AS device */

static struct usb_device_id dev_table[] = {
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_ATMEL_505A       ) },
	{ USB_DEVICE(VENDOR_ID_ATMEL,    PRODUCT_ID_ATMEL_505AS      ) },
	{ }
};

/* jal: not really good style to include a .c file, but all but the above
   is constant in the at76c503-*.c files ... */
#include "at76c503-fw_skel.c"
