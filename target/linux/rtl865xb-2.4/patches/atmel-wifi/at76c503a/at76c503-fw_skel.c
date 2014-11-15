/* -*- linux-c -*- */
/*
 * $Id: at76c503-fw_skel.c,v 1.7 2004/08/18 22:01:45 jal2 Exp $
 *
 * Driver for at76c503-based devices based on the Atmel "Fast-Vnet" reference
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
 * This file is the skeleton used in every fw-specific file.
 * It assumes to have the following defines
 *
 * DRIVER_NAME, DRIVER_AUTHOR, DRIVER_DESC, FW_NAME
 * BOARDTYPE
 *
 * and these variables:
 * struct usb_device_id dev_table
 *
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
extern inline char *fw_dev_param(struct usb_device *udev, char *buf)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 20)
        snprintf(buf, FIRMWARE_NAME_MAX, "usb-%s-%s",
                 udev->bus->bus_name, udev->devpath);
#else
	snprintf(buf, FIRMWARE_NAME_MAX, "usb-%d-%d",
                 udev->bus->busnum, udev->devnum);
#endif
        return buf;
}
#else
#define fw_dev_param(udev, buf) (&udev->dev)
#endif

MODULE_DEVICE_TABLE (usb, dev_table);

/* this is the firmware we use */
const struct firmware *fw;

/* Module paramaters */

static char netdev_name[IFNAMSIZ+1] = "wlan%d";
MODULE_PARM(netdev_name, "c" __MODULE_STRING(IFNAMSIZ));
MODULE_PARM_DESC(netdev_name,
                 "network device name (default is wlan%d)");
static int debug = 1;
MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug,
                 "debug output (default: 1)");

/* Use our own dbg macro */
#undef dbg
#define dbg(format, arg...) \
	do { \
		if (debug) \
		printk(KERN_DEBUG __FILE__ ": " format "\n" , ## arg);\
	} while (0)


/* local function prototypes */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static void *at76c50x_probe(struct usb_device *udev, unsigned int ifnum, 
			    const struct usb_device_id *id);
static void at76c50x_disconnect(struct usb_device *dev, void *ptr);
#else
static int at76c50x_probe(struct usb_interface *interface,
			  const struct usb_device_id *id);
static void at76c50x_disconnect(struct usb_interface *interface);
#endif


/* structure for registering this driver with the usb subsystem */

static struct usb_driver module_usb = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
	owner:      THIS_MODULE,
#endif
	name:	    DRIVER_NAME,
	probe:	    at76c50x_probe,
	disconnect: at76c50x_disconnect,
	id_table:   dev_table,
};

/* Module and USB entry points */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static void *at76c50x_probe(struct usb_device *udev, unsigned int ifnum, 
			    const struct usb_device_id *id)
#else
static int at76c50x_probe(struct usb_interface *interface,
			  const struct usb_device_id *id)
#endif
{
	void *devptr = NULL;
	int retval;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define RETERR(x) return NULL
#else
#define RETERR(x) return x
	struct usb_device *udev __attribute__ ((unused));
	udev = interface_to_usbdev(interface);
#endif

	/* if fw is statically compiled in, we use it */
	if (static_fw.size > 0) {
		dbg("using compiled-in firmware");
		fw = &static_fw;
	} else {
#ifdef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
		char buf[FIRMWARE_NAME_MAX] __attribute__ ((unused));
		if (fw == NULL) {
			dbg("downloading firmware " FW_NAME);
			if (request_firmware(&fw, FW_NAME, 
					     fw_dev_param(udev,buf)) == 0) {
				dbg("got it.");
			} else {
				err("firmware " FW_NAME " not found.");
				err("You may need to download the firmware from "
				    "http://www.thekelleys.org.uk/atmel or "
				    "ftp://ftp.berlios.de/pub/at76c503a/firmware/");
				RETERR(-EFAULT);
			}
		} else
			dbg("re-using previously loaded fw");
#else
                err("either configure driver for firmware loader or compile"
                    "firmware in");
                RETERR(-EFAULT);
#endif
	}

	retval = at76c503_do_probe(THIS_MODULE, udev, &module_usb, fw->data, fw->size,
				   BOARDTYPE, netdev_name, &devptr);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	return devptr;
#else
	return retval;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static void at76c50x_disconnect(struct usb_device *dev, void *ptr)
#else
static void at76c50x_disconnect(struct usb_interface *interface)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	struct at76c503 *ptr;

	ptr = usb_get_intfdata (interface);
	usb_set_intfdata(interface, NULL);
#endif

	info("%s disconnecting", ((struct at76c503 *)ptr)->netdev->name);
	at76c503_delete_device(ptr);
	info(DRIVER_NAME " disconnected");
}

static int __init mod_init(void)
{
	int result;

	info(DRIVER_DESC " " DRIVER_VERSION " loading");

	/* register this driver with the USB subsystem */
	result = usb_register(&module_usb);
	if (result < 0) {
		err("usb_register failed (status %d)", result);
		return -1;
	}

	fw = NULL;
	return 0;
}

static void __exit mod_exit(void)
{
	info(DRIVER_DESC " " DRIVER_VERSION " unloading");
	usb_deregister(&module_usb);
#ifdef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
	if (static_fw.size == 0 && fw != NULL)
		/* we had loaded and allocated the buffer before */
		release_firmware(fw);
#endif
}

module_init (mod_init);
module_exit (mod_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
