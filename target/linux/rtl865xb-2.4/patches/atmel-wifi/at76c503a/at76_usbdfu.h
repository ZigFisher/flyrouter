/* -*- linux-c -*- */
/*
 * USB Device Firmware Upgrade (DFU) handler
 *
 * Copyright (c) 2003 Oliver Kurth
 * Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the 
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 *
*
 */

/* Different to the previous version of usbdfu.c this module does not
   register itself with the USB subsystem but provides a generic
   procedure for DFU download only. We avoid giving up the device etc.
   which was flaky with 2.4.x already (and 2.6.x does not provide
   usb_scan_devices etc. !).
*/

#ifndef _USBDFU_H
#define _USBDFU_H

#include <linux/usb.h>

int usbdfu_download(struct usb_device *udev, u8 *fw_buf, u32 fw_len);

#endif /* _USBDFU_H */
