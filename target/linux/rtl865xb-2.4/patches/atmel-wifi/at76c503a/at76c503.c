/* -*- linux-c -*- */
/* $Id: at76c503.c,v 1.66 2004/08/18 22:01:45 jal2 Exp $
 *
 * USB at76c503/at76c505 driver
 *
 * Copyright (c) 2002 - 2003 Oliver Kurth
 * Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 * Copyright (c) 2004 Nick Jones
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 *
 * Some iw_handler code was taken from airo.c, (C) 1999 Benjamin Reed
 *
 * History:
 *
 * 2002_12_31:
 * - first ping, ah-hoc mode works, fw version 0.90.2 only
 *
 * 2003_01_07 0.1:
 * - first release
 *
 * 2003_01_08 0.2:
 * - moved rx code into tasklet
 * - added locking in keventd handler
 * - support big endian in ieee802_11.h
 * - external firmware downloader now in this module
 *
 * 2003_01_10 0.3:
 * - now using 8 spaces for tab indentations
 * - added tx rate settings (patch from Joerg Albert (jal))
 * - created functions for mib settings
 *
 * 2003_01_19 0.4:
 * - use usbdfu for the internal firmware
 *
 * 2003_01_27 0.5:
 * - implemented WEP. Thanks to jal
 * - added frag and rts ioctl calls (jal again)
 * - module parameter to give names other than eth
 *
 * 2003_01_28 0.6:
 * - make it compile with kernel < 2.4.20 (there is no owner field
 *   in struct usb_driver)
 * - fixed a small bug for the module param eth_name
 * - do not use GFP_DMA, GFP_KERNEL is enough
 * - no down() in _tx() because that's in interrupt. Use
 *   spin_lock_irq() instead
 * - should not stop net queue on urb errors
 * - cleanup in ioctl(): locked it altogether, this makes it easier
 *   to maintain
 * - tried to implement promisc. mode: does not work with this device
 * - tried to implement setting mac address: does not
 *   seem to work with this device
 * - now use fw version 0.90.2 #140 (prev. was #93). Does not help...
 *
 * 2003_01_30 0.7:
 * - now works with fw 0.100.2 (solution was: wait for completion
 *   of commands)
 * - setting MAC address now works (thx to a small fix by jal)
 * - it turned out that promisc. mode is not possible. The firmware
 *   does not allow it. I hope that it will be implemented in a future
 *   version.
 *
 * 2003_02_13 0.8:
 * - scan mode implemented by jal
 * - infra structure mode by jal
 * - some small cleanups (removed dead code)
 *
 * history can now be found in the cvs log at http://at76c503a.berlios.de
 * 
 * TODO:
 * - monitor mode
 *
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/smp_lock.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/usb.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <asm/uaccess.h>
#include <linux/wireless.h>

#if WIRELESS_EXT > 12

#include <net/iw_handler.h>
#define IW_REQUEST_INFO				struct iw_request_info

#else

#define EIWCOMMIT				EINPROGRESS
#define IW_REQUEST_INFO				void

#endif // #if WIRELESS_EXT > 12

#include <linux/rtnetlink.h>  /* for rtnl_lock() */

#include "at76c503.h"
#include "at76_ieee802_11.h"
#include "at76_usbdfu.h"

/* timeout in seconds for the usb_control_msg in get_cmd_status
 * and set_card_command
 */
#ifndef USB_CTRL_GET_TIMEOUT
# define USB_CTRL_GET_TIMEOUT 5
#endif

/* try to make it compile for both 2.4.x and 2.6.x kernels */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)

/* number of endpoints of an interface */
#define NUM_EP(intf) (intf)->altsetting[0].desc.bNumEndpoints
#define EP(intf,nr) (intf)->altsetting[0].endpoint[(nr)].desc
#define GET_DEV(udev) usb_get_dev((udev))
#define PUT_DEV(udev) usb_put_dev((udev))
#define SET_NETDEV_OWNER(ndev,owner) /* not needed anymore ??? */

static inline int submit_urb(struct urb *urb, int mem_flags) {
	return usb_submit_urb(urb, mem_flags);
}
static inline struct urb *alloc_urb(int iso_pk, int mem_flags) {
	return usb_alloc_urb(iso_pk, mem_flags);
}

#else

/* 2.4.x kernels */

#define NUM_EP(intf) (intf)->altsetting[0].bNumEndpoints
#define EP(intf,nr) (intf)->altsetting[0].endpoint[(nr)]

#define GET_DEV(udev) usb_inc_dev_use((udev))
#define PUT_DEV(udev) usb_dec_dev_use((udev))

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 25)
// this macro is defined from 2.4.25 onward
#define SET_NETDEV_DEV(x,y)
#endif

#define SET_NETDEV_OWNER(ndev,owner) ndev->owner = owner

static inline int submit_urb(struct urb *urb, int mem_flags) {
	return usb_submit_urb(urb);
}
static inline struct urb *alloc_urb(int iso_pk, int mem_flags) {
	return usb_alloc_urb(iso_pk);
}

static inline void usb_set_intfdata(struct usb_interface *intf, void *data) {}

#endif //#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)

/* wireless extension level this source currently supports */
#define WIRELESS_EXT_SUPPORTED	16

#ifndef USB_ST_URB_PENDING
#define USB_ST_URB_PENDING	(-EINPROGRESS)
#define USB_ST_STALL		(-EPIPE)
#endif

#ifndef USB_ASYNC_UNLINK
#define USB_ASYNC_UNLINK	URB_ASYNC_UNLINK
#endif

#ifndef FILL_BULK_URB
#define FILL_BULK_URB(a,b,c,d,e,f,g) usb_fill_bulk_urb(a,b,c,d,e,f,g)
#endif

/* debug bits */
#define DBG_PROGRESS        0x000001 /* progress of scan-join-(auth-assoc)-connected */
#define DBG_BSS_TABLE       0x000002 /* show the bss table after scans */
#define DBG_IOCTL           0x000004 /* ioctl calls / settings */
#define DBG_KEVENT          0x000008 /* kevents */
#define DBG_TX_DATA         0x000010 /* tx header */
#define DBG_TX_DATA_CONTENT 0x000020 /* tx content */
#define DBG_TX_MGMT         0x000040
#define DBG_RX_DATA         0x000080 /* rx data header */
#define DBG_RX_DATA_CONTENT 0x000100 /* rx data content */
#define DBG_RX_MGMT         0x000200 /* rx mgmt header except beacon and probe responses */
#define DBG_RX_BEACON       0x000400 /* rx beacon */
#define DBG_RX_CTRL         0x000800 /* rx control */
#define DBG_RX_MGMT_CONTENT 0x001000 /* rx mgmt content */
#define DBG_RX_FRAGS        0x002000 /* rx data fragment handling */
#define DBG_DEVSTART        0x004000 /* fw download, device start */
#define DBG_URB             0x008000 /* rx urb status, ... */
#define DBG_RX_ATMEL_HDR    0x010000 /* the atmel specific header of each rx packet */
#define DBG_PROC_ENTRY      0x020000 /* procedure entries and exits */
#define DBG_PM              0x040000 /* power management settings */
#define DBG_BSS_MATCH       0x080000 /* show why a certain bss did not match */
#define DBG_PARAMS          0x100000 /* show the configured parameters */
#define DBG_WAIT_COMPLETE   0x200000 /* show the wait_completion progress */
#define DBG_RX_FRAGS_SKB    0x400000 /* show skb header for incoming rx fragments */
#define DBG_BSS_TABLE_RM    0x800000 /* inform on removal of old bss table entries */

#define DBG_DEFAULTS 0
static int debug = DBG_DEFAULTS;

static const u8 zeros[32];

/* Use our own dbg macro */
#undef dbg
#define dbg(bits, format, arg...) \
	do { \
		if (debug & (bits)) \
		printk(KERN_DEBUG __FILE__ ": " format "\n" , ## arg);\
	} while (0)

/* uncond. debug output */
#define dbg_uc(format, arg...) \
  printk(KERN_DEBUG __FILE__ ": " format "\n" , ## arg)

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

#define assert(x) \
  do {\
   if (!(x)) \
     err(__FILE__ ":%d assertion " #x " failed", __LINE__);\
  } while (0)

/* how often do we re-try these packets ? */
#define AUTH_RETRIES  3
#define ASSOC_RETRIES 3
#define DISASSOC_RETRIES 3

#define NEW_STATE(dev,newstate) \
  do {\
    dbg(DBG_PROGRESS, "%s: state %d -> %d (" #newstate ")",\
        dev->netdev->name, dev->istate, newstate);\
    dev->istate = newstate;\
  } while (0)

/* the beacon timeout in infra mode when we are connected (in seconds) */
#define BEACON_TIMEOUT 10

/* after how many seconds do we re-scan the channels in infra mode */
#define RESCAN_TIME 10

/* Version Information */
#define DRIVER_DESC "Generic Atmel at76c503/at76c505 routines"

/* Module paramaters */
MODULE_PARM(debug, "i");
#define DRIVER_AUTHOR \
"Oliver Kurth, Joerg Albert <joerg.albert@gmx.de>, Alex, Nick Jones"
MODULE_PARM_DESC(debug, "Debugging level");

static int rx_copybreak = 200;
MODULE_PARM(rx_copybreak, "i");
MODULE_PARM_DESC(rx_copybreak, "rx packet copy threshold");

static int scan_min_time = 10;
MODULE_PARM(scan_min_time, "i");
MODULE_PARM_DESC(scan_min_time, "scan min channel time (default: 10)");

static int scan_max_time = 120;
MODULE_PARM(scan_max_time, "i");
MODULE_PARM_DESC(scan_max_time, "scan max channel time (default: 120)");

static int scan_mode = SCAN_TYPE_ACTIVE;
MODULE_PARM(scan_mode, "i");
MODULE_PARM_DESC(scan_mode, "scan mode: 0 active (with ProbeReq, default), 1 passive");

static int preamble_type = PREAMBLE_TYPE_LONG;
MODULE_PARM(preamble_type, "i");
MODULE_PARM_DESC(preamble_type, "preamble type: 0 long (default), 1 short");

static int auth_mode = 0;
MODULE_PARM(auth_mode, "i");
MODULE_PARM_DESC(auth_mode, "authentication mode: 0 open system (default), "
		 "1 shared secret");

static int pm_mode = PM_ACTIVE;
MODULE_PARM(pm_mode, "i");
MODULE_PARM_DESC(pm_mode, "power management mode: 1 active (def.), 2 powersave, 3 smart save");

static int pm_period = 0;
MODULE_PARM(pm_period, "i");
MODULE_PARM_DESC(pm_period, "period of waking up the device in usec");

#define DEF_RTS_THRESHOLD 1536
#define DEF_FRAG_THRESHOLD 1536
#define DEF_SHORT_RETRY_LIMIT 8
//#define DEF_LONG_RETRY_LIMIT 4
#define DEF_ESSID "okuwlan"
#define DEF_ESSID_LEN 7
#define DEF_CHANNEL 10

#define MAX_RTS_THRESHOLD 2347
#define MAX_FRAG_THRESHOLD 2346
#define MIN_FRAG_THRESHOLD 256

/* The frequency of each channel in MHz */
const long channel_frequency[] = {
        2412, 2417, 2422, 2427, 2432, 2437, 2442,
        2447, 2452, 2457, 2462, 2467, 2472, 2484
};
#define NUM_CHANNELS ( sizeof(channel_frequency) / sizeof(channel_frequency[0]) )

/* the broadcast address */
const u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

/* the supported rates of this hardware, bit7 marks a mandantory rate */
const u8 hw_rates[4] = {0x82,0x84,0x0b,0x16};

/* the max padding size for tx in bytes (see calc_padding)*/
#define MAX_PADDING_SIZE 53

/* a ieee820.11 frame header without addr4 */
struct ieee802_11_mgmt {
	u16 frame_ctl;
	u16 duration_id;
	u8 addr1[ETH_ALEN]; /* destination addr */
	u8 addr2[ETH_ALEN]; /* source addr */
	u8 addr3[ETH_ALEN]; /* BSSID */
	u16 seq_ctl;
	u8 data[1508];
	u32 fcs;
} __attribute__ ((packed));

/* the size of the ieee802.11 header (excl. the at76c503 tx header) */
#define IEEE802_11_MGMT_HEADER_SIZE offsetof(struct ieee802_11_mgmt, data)

#define BEACON_MAX_DATA_LENGTH 1500
/* beacon in ieee802_11_mgmt.data */
struct ieee802_11_beacon_data {
	u8   timestamp[8];           // TSFTIMER
	u16  beacon_interval;         // Kms between TBTTs (Target Beacon Transmission Times)
	u16  capability_information;
	u8   data[BEACON_MAX_DATA_LENGTH]; /* contains: SSID (tag,length,value), 
					  Supported Rates (tlv), channel */
} __attribute__ ((packed));

/* disassoc frame in ieee802_11_mgmt.data */
struct ieee802_11_disassoc_frame {
	u16 reason;
} __attribute__ ((packed));
#define DISASSOC_FRAME_SIZE \
  (AT76C503_TX_HDRLEN + IEEE802_11_MGMT_HEADER_SIZE +\
   sizeof(struct ieee802_11_disassoc_frame))

/* assoc request in ieee802_11_mgmt.data */
struct ieee802_11_assoc_req {
	u16  capability;
	u16  listen_interval;
	u8   data[1]; /* variable number of bytes for SSID 
			 and supported rates (tlv coded) */
};
/* the maximum size of an AssocReq packet */
#define ASSOCREQ_MAX_SIZE \
  (AT76C503_TX_HDRLEN + IEEE802_11_MGMT_HEADER_SIZE +\
   offsetof(struct ieee802_11_assoc_req,data) +\
   1+1+IW_ESSID_MAX_SIZE + 1+1+4)

/* reassoc request in ieee802_11_mgmt.data */
struct ieee802_11_reassoc_req {
	u16  capability;
	u16  listen_interval;
	u8   curr_ap[ETH_ALEN]; /* the bssid of the AP we are
				   currently associated to */
	u8   data[1]; /* variable number of bytes for SSID 
			 and supported rates (tlv coded) */
} __attribute__ ((packed));

/* the maximum size of an AssocReq packet */
#define REASSOCREQ_MAX_SIZE \
  (AT76C503_TX_HDRLEN + IEEE802_11_MGMT_HEADER_SIZE +\
   offsetof(struct ieee802_11_reassoc_req,data) +\
   1+1+IW_ESSID_MAX_SIZE + 1+1+4)


/* assoc/reassoc response */
struct ieee802_11_assoc_resp {
	u16  capability;
	u16  status;
	u16  assoc_id;
	u8   data[1]; /* variable number of bytes for 
			 supported rates (tlv coded) */
} __attribute__ ((packed));

/* auth. request/response in ieee802_11_mgmt.data */
struct ieee802_11_auth_frame {
	u16 algorithm;
	u16 seq_nr;
	u16 status;
	u8 challenge[0];
} __attribute__ ((packed));
/* for shared secret auth, add the challenge text size */
#define AUTH_FRAME_SIZE \
  (AT76C503_TX_HDRLEN + IEEE802_11_MGMT_HEADER_SIZE +\
   sizeof(struct ieee802_11_auth_frame))

/* deauth frame in ieee802_11_mgmt.data */
struct ieee802_11_deauth_frame {
	u16 reason;
} __attribute__ ((packed));
#define DEAUTH_FRAME_SIZE \
  (AT76C503_TX_HDRLEN + IEEE802_11_MGMT_HEADER_SIZE +\
   sizeof(struct ieee802_11_disauth_frame))


#define KEVENT_CTRL_HALT 1
#define KEVENT_NEW_BSS 2
#define KEVENT_SET_PROMISC 3
#define KEVENT_MGMT_TIMEOUT 4
#define KEVENT_SCAN 5 
#define KEVENT_JOIN 6
#define KEVENT_STARTIBSS 7
#define KEVENT_SUBMIT_RX 8
#define KEVENT_RESTART 9 /* restart the device */
#define KEVENT_ASSOC_DONE  10 /* execute the power save settings:
			     listen interval, pm mode, assoc id */

#define KEVENT_EXTERNAL_FW  11
#define KEVENT_INTERNAL_FW  12
#define KEVENT_RESET_DEVICE 13

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);

static u8 snapsig[] = {0xaa, 0xaa, 0x03};
//#ifdef COLLAPSE_RFC1042
/* RFC 1042 encapsulates Ethernet frames in 802.2 SNAP (0xaa, 0xaa, 0x03) with
 * a SNAP OID of 0 (0x00, 0x00, 0x00) */
static u8 rfc1042sig[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
//#endif /* COLLAPSE_RFC1042 */

/* local function prototypes */
static void iwspy_update(struct at76c503 *dev, struct at76c503_rx_buffer *buf);

static void at76c503_read_bulk_callback (struct urb *urb);
static void at76c503_write_bulk_callback(struct urb *urb);
static void defer_kevent (struct at76c503 *dev, int flag);
static struct bss_info *find_matching_bss(struct at76c503 *dev,
					  struct bss_info *curr);
static int auth_req(struct at76c503 *dev, struct bss_info *bss, int seq_nr,
		    u8 *challenge);
static int disassoc_req(struct at76c503 *dev, struct bss_info *bss);
static int assoc_req(struct at76c503 *dev, struct bss_info *bss);
static int reassoc_req(struct at76c503 *dev, struct bss_info *curr,
		       struct bss_info *new);
static void dump_bss_table(struct at76c503 *dev, int force_output);
static int submit_rx_urb(struct at76c503 *dev);
static int startup_device(struct at76c503 *dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
static int update_usb_intf_descr(struct at76c503 *dev);
#endif

/* disassemble the firmware image */
static int at76c503_get_fw_info(u8 *fw_data, int fw_size,
				u32 *board, u32 *version, char **str,
				u8 **ext_fw, u32 *ext_fw_size,
				u8 **int_fw, u32 *int_fw_size);

/* second step of initialisation (after fw download) */
static int init_new_device(struct at76c503 *dev);

/* some abbrev. for wireless events */
#if WIRELESS_EXT > 13
static inline void iwevent_scan_complete(struct net_device *dev)
{
	union iwreq_data wrqu;
	wrqu.data.length = 0;
	wrqu.data.flags = 0;
	wireless_send_event(dev, SIOCGIWSCAN, &wrqu, NULL);
}
static inline void iwevent_bss_connect(struct net_device *dev, u8 *bssid)
{
	union iwreq_data wrqu;
	wrqu.data.length = 0;
	wrqu.data.flags = 0;
	memcpy(wrqu.ap_addr.sa_data, bssid, ETH_ALEN);
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	wireless_send_event(dev, SIOCGIWAP, &wrqu, NULL);
}

static inline void iwevent_bss_disconnect(struct net_device *dev)
{
	union iwreq_data wrqu;
	wrqu.data.length = 0;
	wrqu.data.flags = 0;
	memset(wrqu.ap_addr.sa_data, '\0', ETH_ALEN);
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	wireless_send_event(dev, SIOCGIWAP, &wrqu, NULL);
}

#else
static inline void iwevent_scan_complete(struct net_device *dev) {}
static inline void iwevent_bss_connect(struct net_device *dev, u8 *bssid) {}
static inline void iwevent_bss_disconnect(struct net_device *dev) {}
#endif /* #if WIRELESS_EXT > 13 */

/* hexdump len many bytes from buf into obuf, separated by delim,
   add a trailing \0 into obuf */
static char *hex2str(char *obuf, u8 *buf, int len, char delim)
{
#define BIN2HEX(x) ((x) < 10 ? '0'+(x) : (x)+'A'-10)

  char *ret = obuf;
  while (len--) {
    *obuf++ = BIN2HEX(*buf>>4);
    *obuf++ = BIN2HEX(*buf&0xf);
    if (delim != '\0')
      *obuf++ = delim;
    buf++;
  }
  if (delim != '\0' && obuf > ret)
	  obuf--; // remove last inserted delimiter
  *obuf = '\0';

  return ret;
}

/* == PROC is_cloaked_ssid ==
   returns != 0, if the given SSID is a cloaked one:
   - length 0
   - length > 0, all bytes are \0
   - length == 1, SSID ' '
*/
static inline int is_cloaked_ssid(u8 *ssid, int length)
{
	return (length == 0) || 
		(length == 1 && *ssid == ' ') ||
		(length > 0 && !memcmp(ssid,zeros,length));
}

static inline void free_bss_list(struct at76c503 *dev)
{
	struct list_head *next, *ptr;
	unsigned long flags;

	spin_lock_irqsave(&dev->bss_list_spinlock, flags);

	dev->curr_bss = dev->new_bss = NULL;

	list_for_each_safe(ptr, next, &dev->bss_list) {
		list_del(ptr);
		kfree(list_entry(ptr, struct bss_info, list));
	}

	spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);
}

static inline char *mac2str(u8 *mac)
{
	static char str [6*3];
  
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return str;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)

/* == PROC analyze_usb_config == 
   This procedure analyzes the configuration after the
   USB device got reset and find the start of the interface and the
   two endpoint descriptors.
   Returns < 0 if the descriptors seems to be wrong. */
static int analyze_usb_config(u8 *cfgd, int cfgd_len,
				int *intf_idx, int *ep0_idx, int *ep1_idx)
{
	u8 *cfgd_start = cfgd;
	u8 *cfgd_end = cfgd + cfgd_len; /* first byte after config descriptor */
	int nr_intf=0, nr_ep=0; /* number of interface, number of endpoint descr.
			       found */

	assert(cfgd_len >= 2);
	if (cfgd_len < 2)
		return -1;

	if (*(cfgd+1) != USB_DT_CONFIG) {
		err("not a config descriptor");
		return -2;
	}

	if (*cfgd != USB_DT_CONFIG_SIZE) {
		err("invalid length for config descriptor: %d", *cfgd);
		return -3;
	}

	/* scan the config descr */
	while ((cfgd+1) < cfgd_end) {

		switch (*(cfgd+1)) {

		case USB_DT_INTERFACE:
			nr_intf++;
			if (nr_intf == 1)
				*intf_idx = cfgd - cfgd_start;
			break;

		case USB_DT_ENDPOINT:
			nr_ep++;
			if (nr_ep == 1)
				*ep0_idx = cfgd - cfgd_start;
			else
				if (nr_ep == 2)
					*ep1_idx = cfgd - cfgd_start;
			break;
		default:
			;
		}
		cfgd += *cfgd;
	} /* while ((cfgd+1) < cfgd_end) */

	if (nr_ep != 2 || nr_intf != 1) {
		err("unexpected nr of intf (%d) or endpoints (%d)",
		    nr_intf, nr_ep);
		return -4;
	}

	return 0;
} /* end of analyze_usb_config */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
/* == PROC update_usb_intf_descr ==
   currently (2.6.0-test2) usb_reset_device() does not recognize that
   the interface descr. are changed.
   This procedure reads the configuration and does a limited parsing of
   the interface and endpoint descriptors.
   This is IMHO needed until usb_reset_device() is changed inside the
   kernel's USB subsystem.
   Copied from usb/core/config.c:usb_get_configuration()

   THIS IS VERY UGLY CODE - DO NOT COPY IT ! */

#define AT76C503A_USB_CONFDESCR_LEN 0x20
/* the short configuration descriptor before reset */
//#define AT76C503A_USB_SHORT_CONFDESCR_LEN 0x19

static int update_usb_intf_descr(struct at76c503 *dev)
{
	int intf0; /* begin of intf descriptor in configuration */ 
	int ep0, ep1; /* begin of endpoint descriptors */

	struct usb_device *udev = dev->udev;
	struct usb_config_descriptor *cfg_desc;
	int result = 0, size;
	u8 *buffer;
	struct usb_host_interface *ifp;
	int i;

	dbg(DBG_DEVSTART, "%s: ENTER", __FUNCTION__);

	cfg_desc = (struct usb_config_descriptor *)
		kmalloc(AT76C503A_USB_CONFDESCR_LEN, GFP_KERNEL);
	if (!cfg_desc) {
		err("cannot kmalloc config desc");
		return -ENOMEM;
	}

	result = usb_get_descriptor(udev, USB_DT_CONFIG, 0,
				    cfg_desc, AT76C503A_USB_CONFDESCR_LEN);
	if (result < AT76C503A_USB_CONFDESCR_LEN) {
		if (result < 0)
			err("unable to get descriptor");
		else {
			err("config descriptor too short (expected >= %i, got %i)",
			    AT76C503A_USB_CONFDESCR_LEN, result);
			result = -EINVAL;
		}
		goto err;
	}

	/* now check the config descriptor */
	le16_to_cpus(&cfg_desc->wTotalLength);
	size = cfg_desc->wTotalLength;
	buffer = (u8 *)cfg_desc;
	
	if (cfg_desc->bNumInterfaces > 1) {
		err("found %d interfaces", cfg_desc->bNumInterfaces);
		result = - EINVAL;
		goto err;
	}

	if ((result=analyze_usb_config(buffer, size, &intf0, &ep0, &ep1))) {

		err("analyze_usb_config returned %d for config desc %s",
		    result,
		    hex2str(dev->obuf, (u8 *)cfg_desc, 
			    min((int)(sizeof(dev->obuf)-1)/2,size), '\0'));
		result=-EINVAL;
		goto err;
	}

	/* we got the correct config descriptor - update the interface's endpoints */
	ifp = &udev->actconfig->interface[0]->altsetting[0];

	if (ifp->endpoint)
		kfree(ifp->endpoint);

	memcpy(&ifp->desc, buffer+intf0, USB_DT_INTERFACE_SIZE);

	if (!(ifp->endpoint = kmalloc(2 * sizeof(struct usb_host_endpoint), 
				      GFP_KERNEL))) {
		result = -ENOMEM;
		goto err;
	}
	memset(ifp->endpoint, 0, 2 * sizeof(struct usb_host_endpoint));
	memcpy(&ifp->endpoint[0].desc, buffer+ep0, USB_DT_ENDPOINT_SIZE);
	le16_to_cpus(&ifp->endpoint[0].desc.wMaxPacketSize);
	memcpy(&ifp->endpoint[1].desc, buffer+ep1, USB_DT_ENDPOINT_SIZE);
	le16_to_cpus(&ifp->endpoint[1].desc.wMaxPacketSize);

	/* we must set the max packet for the new ep (see usb_set_maxpacket() ) */

#define usb_endpoint_out(ep_dir)	(!((ep_dir) & USB_DIR_IN))
	for(i=0; i < ifp->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor	*d = &ifp->endpoint[i].desc;		
		int b = d->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
		if (usb_endpoint_out(d->bEndpointAddress)) {
			if (d->wMaxPacketSize > udev->epmaxpacketout[b])
				udev->epmaxpacketout[b] = d->wMaxPacketSize;
		} else {
			if (d->wMaxPacketSize > udev->epmaxpacketin[b])
				udev->epmaxpacketin[b] = d->wMaxPacketSize;
		}
	}
			     
	dbg(DBG_DEVSTART, "%s: ifp %p num_altsetting %d "
	    "endpoint addr x%x, x%x", __FUNCTION__,
	    ifp, udev->actconfig->interface[0]->num_altsetting,
	    ifp->endpoint[0].desc.bEndpointAddress,
	    ifp->endpoint[1].desc.bEndpointAddress);
	result = 0;
err:
	kfree(cfg_desc);
	dbg(DBG_DEVSTART, "%s: EXIT with %d", __FUNCTION__, result);
	return result;
} /* update_usb_intf_descr */

#else

/* 2.4.x version */
/* == PROC update_usb_intf_descr ==
   currently (2.4.23) usb_reset_device() does not recognize that
   the interface descr. are changed.
   This procedure reads the configuration and does a limited parsing of
   the interface and endpoint descriptors.
   This is IMHO needed until usb_reset_device() is changed inside the
   kernel's USB subsystem.
   Copied from usb/core/config.c:usb_get_configuration()

   THIS IS VERY UGLY CODE - DO NOT COPY IT ! */

#define AT76C503A_USB_CONFDESCR_LEN 0x20
/* the short configuration descriptor before reset */
//#define AT76C503A_USB_SHORT_CONFDESCR_LEN 0x19

int update_usb_intf_descr(struct at76c503 *dev)
{
	int intf0; /* begin of intf descriptor in configuration */ 
	int ep0, ep1; /* begin of endpoint descriptors */

	struct usb_device *udev = dev->udev;
	struct usb_config_descriptor *cfg_desc;
	int result = 0, size;
	u8 *buffer;
	struct usb_interface_descriptor *ifp;
	int i;

	dbg(DBG_DEVSTART, "%s: ENTER", __FUNCTION__);

	cfg_desc = (struct usb_config_descriptor *)
		kmalloc(AT76C503A_USB_CONFDESCR_LEN, GFP_KERNEL);
	if (!cfg_desc) {
		err("cannot kmalloc config desc");
		return -ENOMEM;
	}

	result = usb_get_descriptor(udev, USB_DT_CONFIG, 0,
				    cfg_desc, AT76C503A_USB_CONFDESCR_LEN);
	if (result < AT76C503A_USB_CONFDESCR_LEN) {
		if (result < 0)
			err("unable to get descriptor");
		else {
			err("config descriptor too short (expected >= %i, got %i)",
			    AT76C503A_USB_CONFDESCR_LEN, result);
			result = -EINVAL;
		}
		goto err;
	}

	/* now check the config descriptor */
	le16_to_cpus(&cfg_desc->wTotalLength);
	size = cfg_desc->wTotalLength;
	buffer = (u8 *)cfg_desc;
	
	if (cfg_desc->bNumInterfaces > 1) {
		err("found %d interfaces", cfg_desc->bNumInterfaces);
		result = - EINVAL;
		goto err;
	}

	if ((result=analyze_usb_config(buffer, size, &intf0, &ep0, &ep1))) {
		err("analyze_usb_config returned %d for config desc %s",
		    result,
		    hex2str(dev->obuf, (u8 *)cfg_desc, 
			    min((int)(sizeof(dev->obuf)-1)/2, size), '\0'));
		result=-EINVAL;
		goto err;
	}

	/* we got the correct config descriptor - update the interface's endpoints */
	ifp = &udev->actconfig->interface[0].altsetting[0];

	if (ifp->endpoint)
		kfree(ifp->endpoint);

	//memcpy(&ifp->desc, expected_cfg_descr+intf0, USB_DT_INTERFACE_SIZE);

	if (!(ifp->endpoint = kmalloc(2 * sizeof(struct usb_endpoint_descriptor), 
				      GFP_KERNEL))) {
		result = -ENOMEM;
		goto err;
	}

	memset(ifp->endpoint, 0, 2 * sizeof(struct usb_endpoint_descriptor));
	memcpy(&ifp->endpoint[0], buffer+ep0, USB_DT_ENDPOINT_SIZE);
	le16_to_cpus(&ifp->endpoint[0].wMaxPacketSize);
	memcpy(&ifp->endpoint[1], buffer+ep1, USB_DT_ENDPOINT_SIZE);
	le16_to_cpus(&ifp->endpoint[1].wMaxPacketSize);

	/* fill the interface data */
	memcpy(ifp, buffer+intf0, USB_DT_INTERFACE_SIZE);

	/* we must set the max packet for the new ep (see usb_set_maxpacket() ) */

	for(i=0; i < ifp->bNumEndpoints; i++) {
		struct usb_endpoint_descriptor	*d = &ifp->endpoint[i];		
		int b = d->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
		if (usb_endpoint_out(d->bEndpointAddress)) {
			if (d->wMaxPacketSize > udev->epmaxpacketout[b])
				udev->epmaxpacketout[b] = d->wMaxPacketSize;
		} else {
			if (d->wMaxPacketSize > udev->epmaxpacketin[b])
				udev->epmaxpacketin[b] = d->wMaxPacketSize;
		}
	}
			     
	dbg(DBG_DEVSTART, "%s: ifp %p num_altsetting %d "
	    "endpoint addr x%x, x%x", __FUNCTION__,
	    ifp, udev->actconfig->interface[0].num_altsetting,
	    ifp->endpoint[0].bEndpointAddress,
	    ifp->endpoint[1].bEndpointAddress);
	result = 0;
err:
	kfree(cfg_desc);
	dbg(DBG_DEVSTART, "%s: EXIT with %d", __FUNCTION__, result);
	return result;
} /* update_usb_intf_descr */
# endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0) ... #else ... */
#endif	/* #if  LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8) */


int at76c503_remap(struct usb_device *udev)
{
	int ret;
	ret = usb_control_msg(udev, usb_sndctrlpipe(udev,0),
			      0x0a, INTERFACE_VENDOR_REQUEST_OUT,
			      0, 0,
			      NULL, 0, HZ * USB_CTRL_GET_TIMEOUT);
	if (ret < 0)
		return ret;

	return 0;
}


static int get_op_mode(struct usb_device *udev)
{
	int ret;
	u8 op_mode;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0x33, INTERFACE_VENDOR_REQUEST_IN,
			      0x01, 0,
			      &op_mode, 1, HZ * USB_CTRL_GET_TIMEOUT);
	if(ret < 0)
		return ret;
	return op_mode;
}

/* this loads a block of the second part of the firmware */
static inline
int load_ext_fw_block(struct usb_device *udev,
		      int i, unsigned char *buf, int bsize)
{
	return usb_control_msg(udev, usb_sndctrlpipe(udev,0),
			       0x0e, DEVICE_VENDOR_REQUEST_OUT,
			       0x0802, i,
			       buf, bsize, HZ * USB_CTRL_GET_TIMEOUT);
}

static inline
int get_hw_cfg_rfmd(struct usb_device *udev,
	       unsigned char *buf, int buf_size)
{
	return usb_control_msg(udev, usb_rcvctrlpipe(udev,0),
			       0x33, INTERFACE_VENDOR_REQUEST_IN,
			       ((0x0a << 8) | 0x02), 0,
			       buf, buf_size, HZ * USB_CTRL_GET_TIMEOUT);
}

/* Intersil boards use a different "value" for GetHWConfig requests */
static inline
int get_hw_cfg_intersil(struct usb_device *udev,
	       unsigned char *buf, int buf_size)
{
	return usb_control_msg(udev, usb_rcvctrlpipe(udev,0),
			       0x33, INTERFACE_VENDOR_REQUEST_IN,
			       ((0x09 << 8) | 0x02), 0,
			       buf, buf_size, HZ * USB_CTRL_GET_TIMEOUT);
}

/* Get the hardware configuration for the adapter and place the appropriate
 * data in the appropriate fields of 'dev' (the GetHWConfig request and
 * interpretation of the result depends on the type of board we're dealing
 * with) */
static int get_hw_config(struct at76c503 *dev)
{
	int ret;
	union {
		struct hwcfg_intersil i;
		struct hwcfg_rfmd     r3;
		struct hwcfg_r505     r5;
	} *hwcfg = kmalloc(sizeof(*hwcfg), GFP_KERNEL);

	if (!hwcfg)
		return -ENOMEM;

	switch (dev->board_type) {

	  case BOARDTYPE_503_INTERSIL_3861:
	  case BOARDTYPE_503_INTERSIL_3863:
		ret = get_hw_cfg_intersil(dev->udev, (unsigned char *)&hwcfg->i, sizeof(hwcfg->i));
		if (ret < 0) break;
		memcpy(dev->mac_addr, hwcfg->i.mac_addr, ETH_ALEN);
		memcpy(dev->cr31_values, hwcfg->i.cr31_values, 14);
		memcpy(dev->cr58_values, hwcfg->i.cr58_values, 14);
		memcpy(dev->pidvid, hwcfg->i.pidvid, 4);
		dev->regulatory_domain = hwcfg->i.regulatory_domain;
		break;

	  case BOARDTYPE_503_RFMD:
	  case BOARDTYPE_503_RFMD_ACC:
		ret = get_hw_cfg_rfmd(dev->udev, (unsigned char *)&hwcfg->r3, sizeof(hwcfg->r3));
		if (ret < 0) break;
		memcpy(dev->cr20_values, hwcfg->r3.cr20_values, 14);
		memcpy(dev->cr21_values, hwcfg->r3.cr21_values, 14);
		memcpy(dev->bb_cr, hwcfg->r3.bb_cr, 14);
		memcpy(dev->pidvid, hwcfg->r3.pidvid, 4);
		memcpy(dev->mac_addr, hwcfg->r3.mac_addr, ETH_ALEN);
		dev->regulatory_domain = hwcfg->r3.regulatory_domain;
		memcpy(dev->low_power_values, hwcfg->r3.low_power_values, 14);
		memcpy(dev->normal_power_values, hwcfg->r3.normal_power_values, 14);
		break;

	  case BOARDTYPE_505_RFMD:
	  case BOARDTYPE_505_RFMD_2958:
	  case BOARDTYPE_505A_RFMD_2958:
		ret = get_hw_cfg_rfmd(dev->udev, (unsigned char *)&hwcfg->r5, sizeof(hwcfg->r5));
		if (ret < 0) break;
		memcpy(dev->cr39_values, hwcfg->r5.cr39_values, 14);
		memcpy(dev->bb_cr, hwcfg->r5.bb_cr, 14);
		memcpy(dev->pidvid, hwcfg->r5.pidvid, 4);
		memcpy(dev->mac_addr, hwcfg->r5.mac_addr, ETH_ALEN);
		dev->regulatory_domain = hwcfg->r5.regulatory_domain;
		memcpy(dev->cr15_values, hwcfg->r5.cr15_values, 14);
		break;

	  default:
		err("Bad board type set (%d).  Unable to get hardware config.", dev->board_type);
		ret = -EINVAL;
	}

	kfree(hwcfg);

	if (ret < 0) {
		err("Get HW Config failed (%d)", ret);
	}
	return ret;
}

/* == PROC getRegDomain == */
struct reg_domain const *getRegDomain(u16 code)
{
	static struct reg_domain const fd_tab[] = {
		{0x10, "FCC (U.S)", 0x7ff}, /* ch 1-11 */
		{0x20, "IC (Canada)", 0x7ff}, /* ch 1-11 */
		{0x30, "ETSI (Europe - (Spain+France)", 0x1fff},  /* ch 1-13 */
		{0x31, "Spain", 0x600},    /* ch 10,11 */
		{0x32, "France", 0x1e00},  /* ch 10-13 */
		{0x40, "MKK (Japan)", 0x2000},  /* ch 14 */
		{0x41, "MKK1 (Japan)", 0x3fff},  /* ch 1-14 */
		{0x50, "Israel", 0x3fc},  /* ch 3-9 */
	};
	static int const tab_len = sizeof(fd_tab) / sizeof(struct reg_domain);

	/* use this if an unknown code comes in */
	static struct reg_domain const unknown = 
		{0, "<unknown>", 0xffffffff};
  
	int i;

	for(i=0; i < tab_len; i++)
		if (code == fd_tab[i].code)
			break;
  
	return (i >= tab_len) ? &unknown : &fd_tab[i];
} /* getFreqDomain */

static inline
int get_mib(struct usb_device *udev,
	    u16 mib, u8 *buf, int buf_size)
{
	return usb_control_msg(udev, usb_rcvctrlpipe(udev,0),
			       0x33, INTERFACE_VENDOR_REQUEST_IN,
			       mib << 8, 0,
			       buf, buf_size, HZ * USB_CTRL_GET_TIMEOUT);
}

static inline
int get_cmd_status(struct usb_device *udev,
		   u8 cmd, u8 *cmd_status)
{
	return usb_control_msg(udev, usb_rcvctrlpipe(udev,0),
			       0x22, INTERFACE_VENDOR_REQUEST_IN,
			       cmd, 0,
			       cmd_status, 40, HZ * USB_CTRL_GET_TIMEOUT);
}

#define EXT_FW_BLOCK_SIZE 1024
static int download_external_fw(struct usb_device *udev, u8 *buf, int size)
{
	int i = 0, ret = 0;
	u8 *block;

	if (size < 0) return -EINVAL;
	if ((size > 0) && (buf == NULL)) return -EFAULT;

	block = kmalloc(EXT_FW_BLOCK_SIZE, GFP_KERNEL);
	if (block == NULL) return -ENOMEM;

	dbg(DBG_DEVSTART, "downloading external firmware");

	while(size > 0){
		int bsize = size > EXT_FW_BLOCK_SIZE ? EXT_FW_BLOCK_SIZE : size;

		memcpy(block, buf, bsize);
		dbg(DBG_DEVSTART,
		    "ext fw, size left = %5d, bsize = %4d, i = %2d", size, bsize, i);
		if((ret = load_ext_fw_block(udev, i, block, bsize)) < 0){
			err("load_ext_fw_block failed: %d, i = %d", ret, i);
			goto exit;
		}
		buf += bsize;
		size -= bsize;
		i++;
	}

	/* for fw >= 0.100, the device needs
	   an extra empty block: */
	if((ret = load_ext_fw_block(udev, i, block, 0)) < 0){
		err("load_ext_fw_block failed: %d, i = %d", ret, i);
		goto exit;
	}

 exit:
	kfree(block);
	return ret;
}

static
int set_card_command(struct usb_device *udev, int cmd,
		    unsigned char *buf, int buf_size)
{
	int ret;
	struct at76c503_command *cmd_buf =
		(struct at76c503_command *)kmalloc(
			sizeof(struct at76c503_command) + buf_size,
			GFP_KERNEL);

	if(cmd_buf){
		cmd_buf->cmd = cmd;
		cmd_buf->reserved = 0;
		cmd_buf->size = cpu_to_le16(buf_size);
		if(buf_size > 0)
			memcpy(&(cmd_buf[1]), buf, buf_size);
		ret = usb_control_msg(udev, usb_sndctrlpipe(udev,0),
				      0x0e, DEVICE_VENDOR_REQUEST_OUT,
				      0, 0,
				      cmd_buf,
				      sizeof(struct at76c503_command) + buf_size,
				      HZ * USB_CTRL_GET_TIMEOUT);
		kfree(cmd_buf);
		return ret;
	}

	return -ENOMEM;
}

/* TODO: should timeout */
int wait_completion(struct at76c503 *dev, int cmd)
{
	u8 *cmd_status = kmalloc(40, GFP_KERNEL);
	struct net_device *netdev = dev->netdev;
	int ret = 0;

	do{
		ret = get_cmd_status(dev->udev, cmd, cmd_status);
		if(ret < 0){
			err("%s: get_cmd_status failed: %d", netdev->name, ret);
			break;
		}

		dbg(DBG_WAIT_COMPLETE, "%s: cmd %d,cmd_status[5] = %d",
		    dev->netdev->name, cmd, cmd_status[5]);

		if(cmd_status[5] == CMD_STATUS_IN_PROGRESS ||
		   cmd_status[5] == CMD_STATUS_IDLE){
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(HZ/10); // 100 ms
		}else break;
	}while(1);

	if (ret >= 0)
		/* if get_cmd_status did not fail, return the status
		   retrieved */
		ret = cmd_status[5];
	kfree(cmd_status);
	return ret;
}

static
int set_mib(struct at76c503 *dev, struct set_mib_buffer *buf)
{
	struct usb_device *udev = dev->udev;
	int ret;
	struct at76c503_command *cmd_buf =
		(struct at76c503_command *)kmalloc(
			sizeof(struct at76c503_command) + buf->size + 4,
			GFP_KERNEL);

	if(cmd_buf){
		cmd_buf->cmd = CMD_SET_MIB;
		cmd_buf->reserved = 0;
		cmd_buf->size = cpu_to_le16(buf->size + 4);
		memcpy(&(cmd_buf[1]), buf, buf->size + 4);
		ret = usb_control_msg(udev, usb_sndctrlpipe(udev,0),
				      0x0e, DEVICE_VENDOR_REQUEST_OUT,
				      0, 0,
				      cmd_buf,
				      sizeof(struct at76c503_command) + buf->size + 4,
				      HZ * USB_CTRL_GET_TIMEOUT);
		if (ret >= 0)
			if ((ret=wait_completion(dev, CMD_SET_MIB)) != 
			    CMD_STATUS_COMPLETE) {
				info("%s: set_mib: wait_completion failed with %d",
				     dev->netdev->name, ret);
				ret = -156; /* ??? */
			}
		kfree(cmd_buf);
		return ret;
	}

	return -ENOMEM;
}

/* return < 0 on error, == 0 if no command sent, == 1 if cmd sent */
static
int set_radio(struct at76c503 *dev, int on_off)
{
	int ret;

	if(dev->radio_on != on_off){
		ret = set_card_command(dev->udev, CMD_RADIO, NULL, 0);
		if(ret < 0){
			err("%s: set_card_command(CMD_RADIO) failed: %d", dev->netdev->name, ret);
		} else
			ret = 1;
		dev->radio_on = on_off;
	} else
		ret = 0;
	return ret;
}


/* == PROC set_pm_mode ==
   sets power save modi (PM_ACTIVE/PM_SAVE/PM_SMART_SAVE) */
static int set_pm_mode(struct at76c503 *dev, u8 mode) __attribute__ ((unused));
static int set_pm_mode(struct at76c503 *dev, u8 mode)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_MAC_MGMT;
	dev->mib_buf.size = 1;
	dev->mib_buf.index = POWER_MGMT_MODE_OFFSET;

	dev->mib_buf.data[0] = mode;

	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (pm_mode) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

/* == PROC set_associd ==
   sets the assoc id for power save mode */
static int set_associd(struct at76c503 *dev, u16 id) __attribute__ ((unused));
static int set_associd(struct at76c503 *dev, u16 id)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_MAC_MGMT;
	dev->mib_buf.size = 2;
	dev->mib_buf.index = STATION_ID_OFFSET;

	dev->mib_buf.data[0] = id & 0xff;
	dev->mib_buf.data[1] = id >> 8;

	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (associd) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

/* == PROC set_listen_interval ==
   sets the listen interval for power save mode.
   really needed, as we have a similar parameter in the assocreq ??? */
static int set_listen_interval(struct at76c503 *dev, u16 interval) __attribute__ ((unused));
static int set_listen_interval(struct at76c503 *dev, u16 interval)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_MAC;
	dev->mib_buf.size = 2;
	dev->mib_buf.index = STATION_ID_OFFSET;

	dev->mib_buf.data[0] = interval & 0xff;
	dev->mib_buf.data[1] = interval >> 8;

	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (listen_interval) failed: %d",
		    dev->netdev->name, ret);
	}
	return ret;
}

static
int set_preamble(struct at76c503 *dev, u8 type)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_LOCAL;
	dev->mib_buf.size = 1;
	dev->mib_buf.index = PREAMBLE_TYPE_OFFSET;
	dev->mib_buf.data[0] = type;
	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (preamble) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

static
int set_frag(struct at76c503 *dev, u16 size)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_MAC;
	dev->mib_buf.size = 2;
	dev->mib_buf.index = FRAGMENTATION_OFFSET;
	*(u16*)dev->mib_buf.data = cpu_to_le16(size);
	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (frag threshold) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

static
int set_rts(struct at76c503 *dev, u16 size)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_MAC;
	dev->mib_buf.size = 2;
	dev->mib_buf.index = RTS_OFFSET;
	*(u16*)dev->mib_buf.data = cpu_to_le16(size);
	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (rts) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

static
int set_autorate_fallback(struct at76c503 *dev, int onoff)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_LOCAL;
	dev->mib_buf.size = 1;
	dev->mib_buf.index = TX_AUTORATE_FALLBACK_OFFSET;
	dev->mib_buf.data[0] = onoff;
	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (autorate fallback) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

static int set_mac_address(struct at76c503 *dev, void *addr)
{
        int ret = 0;

        memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
        dev->mib_buf.type = MIB_MAC_ADD;
        dev->mib_buf.size = ETH_ALEN;
        dev->mib_buf.index = offsetof(struct mib_mac_addr, mac_addr);
        memcpy(dev->mib_buf.data, addr, ETH_ALEN);
        ret = set_mib(dev, &dev->mib_buf);
        if(ret < 0){
                err("%s: set_mib (MAC_ADDR, mac_addr) failed: %d",
                    dev->netdev->name, ret);
        }
        return ret;
}

#if 0
/* implemented to get promisc. mode working, but does not help.
   May still be useful for multicast eventually. */
static int set_group_address(struct at76c503 *dev, u8 *addr, int n)
{
        int ret = 0;

        memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
        dev->mib_buf.type = MIB_MAC_ADD;
        dev->mib_buf.size = ETH_ALEN;
        dev->mib_buf.index = offsetof(struct mib_mac_addr, group_addr) + n*ETH_ALEN;
        memcpy(dev->mib_buf.data, addr, ETH_ALEN);
        ret = set_mib(dev, &dev->mib_buf);
        if(ret < 0){
                err("%s: set_mib (MIB_MAC_ADD, group_addr) failed: %d",
                    dev->netdev->name, ret);
        }

#if 1
	/* I do not know anything about the group_addr_status field... (oku)*/
        memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
        dev->mib_buf.type = MIB_MAC_ADD;
        dev->mib_buf.size = 1;
        dev->mib_buf.index = offsetof(struct mib_mac_addr, group_addr_status) + n;
        dev->mib_buf.data[0] = 1;
        ret = set_mib(dev, &dev->mib_buf);
        if(ret < 0){
                err("%s: set_mib (MIB_MAC_ADD, group_addr_status) failed: %d",
                    dev->netdev->name, ret);
        }
#endif
        return ret;
}
#endif

static
int set_promisc(struct at76c503 *dev, int onoff)
{
	int ret = 0;

	memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
	dev->mib_buf.type = MIB_LOCAL;
	dev->mib_buf.size = 1;
	dev->mib_buf.index = offsetof(struct mib_local, promiscuous_mode);
	dev->mib_buf.data[0] = onoff ? 1 : 0;
	ret = set_mib(dev, &dev->mib_buf);
	if(ret < 0){
		err("%s: set_mib (promiscous_mode) failed: %d", dev->netdev->name, ret);
	}
	return ret;
}

static int dump_mib_mac_addr(struct at76c503 *dev) __attribute__ ((unused));
static int dump_mib_mac_addr(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_mac_addr *mac_addr =
		kmalloc(sizeof(struct mib_mac_addr), GFP_KERNEL);

	if(!mac_addr){
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = get_mib(dev->udev, MIB_MAC_ADD,
		      (u8*)mac_addr, sizeof(struct mib_mac_addr));
	if(ret < 0){
		err("%s: get_mib (MAC_ADDR) failed: %d", dev->netdev->name, ret);
		goto err;
	}

	dbg_uc("%s: MIB MAC_ADDR: mac_addr %s group_addr %s status %d %d %d %d", 
	       dev->netdev->name, mac2str(mac_addr->mac_addr),
	       hex2str(dev->obuf, (u8 *)mac_addr->group_addr, 
		       min((int)(sizeof(dev->obuf)-1)/2, 4*ETH_ALEN), '\0'),
	       mac_addr->group_addr_status[0], mac_addr->group_addr_status[1],
	       mac_addr->group_addr_status[2], mac_addr->group_addr_status[3]);

 err:
	kfree(mac_addr);
 exit:
	return ret;
}

static int dump_mib_mac_wep(struct at76c503 *dev) __attribute__ ((unused));
static int dump_mib_mac_wep(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_mac_wep *mac_wep =
		kmalloc(sizeof(struct mib_mac_wep), GFP_KERNEL);

	if(!mac_wep){
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = get_mib(dev->udev, MIB_MAC_WEP,
		      (u8*)mac_wep, sizeof(struct mib_mac_wep));
	if(ret < 0){
		err("%s: get_mib (MAC_WEP) failed: %d", dev->netdev->name, ret);
		goto err;
	}

	dbg_uc("%s: MIB MAC_WEP: priv_invoked %u def_key_id %u key_len %u "
	    "excl_unencr %u wep_icv_err %u wep_excluded %u encr_level %u key %d: %s",
	    dev->netdev->name, mac_wep->privacy_invoked,
	    mac_wep->wep_default_key_id, mac_wep->wep_key_mapping_len,
	    mac_wep->exclude_unencrypted,le32_to_cpu( mac_wep->wep_icv_error_count),
	    le32_to_cpu(mac_wep->wep_excluded_count),
	    mac_wep->encryption_level, mac_wep->wep_default_key_id,
	    mac_wep->wep_default_key_id < 4 ?
	    hex2str(dev->obuf,
		    mac_wep->wep_default_keyvalue[mac_wep->wep_default_key_id],
		    min((int)(sizeof(dev->obuf)-1)/2,
			mac_wep->encryption_level == 2 ? 13 : 5), '\0') :
	       "<invalid key id>");

 err:
	kfree(mac_wep);
 exit:
	return ret;
}

static int dump_mib_mac_mgmt(struct at76c503 *dev) __attribute__ ((unused));
static int dump_mib_mac_mgmt(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_mac_mgmt *mac_mgmt =
		kmalloc(sizeof(struct mib_mac_mgmt), GFP_KERNEL);

	if(!mac_mgmt){
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = get_mib(dev->udev, MIB_MAC_MGMT,
		      (u8*)mac_mgmt, sizeof(struct mib_mac_mgmt));
	if(ret < 0){
		err("%s: get_mib failed: %d", dev->netdev->name, ret);
		goto err;
	}

	dbg_uc("%s: MIB MAC_MGMT: station_id x%x pm_mode %d\n",
	    dev->netdev->name, le16_to_cpu(mac_mgmt->station_id),
	    mac_mgmt->power_mgmt_mode);
 err:
	kfree(mac_mgmt);
 exit:
	return ret;
}

static int dump_mib_mac(struct at76c503 *dev) __attribute__ ((unused));
static int dump_mib_mac(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_mac *mac =
		kmalloc(sizeof(struct mib_mac), GFP_KERNEL);

	if(!mac){
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = get_mib(dev->udev, MIB_MAC,
		      (u8*)mac, sizeof(struct mib_mac));
	if(ret < 0){
		err("%s: get_mib failed: %d", dev->netdev->name, ret);
		goto err;
	}

	dbg_uc("%s: MIB MAC: listen_int %d",
	    dev->netdev->name, le16_to_cpu(mac->listen_interval));
 err:
	kfree(mac);
 exit:
	return ret;
}

static
int get_current_bssid(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_mac_mgmt *mac_mgmt =
		kmalloc(sizeof(struct mib_mac_mgmt), GFP_KERNEL);

	if(!mac_mgmt){
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = get_mib(dev->udev, MIB_MAC_MGMT,
		      (u8*)mac_mgmt, sizeof(struct mib_mac_mgmt));
	if(ret < 0){
		err("%s: get_mib failed: %d", dev->netdev->name, ret);
		goto err;
	}
	memcpy(dev->bssid, mac_mgmt->current_bssid, ETH_ALEN);
	info("using BSSID %s", mac2str(dev->bssid));
 err:
	kfree(mac_mgmt);
 exit:
	return ret;
}

static
int get_current_channel(struct at76c503 *dev)
{
	int ret = 0;
	struct mib_phy *phy =
		kmalloc(sizeof(struct mib_phy), GFP_KERNEL);

	if(!phy){
		ret = -ENOMEM;
		goto exit;
	}
	ret = get_mib(dev->udev, MIB_PHY, (u8*)phy,
		      sizeof(struct mib_phy));
	if(ret < 0){
		err("%s: get_mib(MIB_PHY) failed: %d", dev->netdev->name, ret);
		goto err;
	}
	dev->channel = phy->channel_id;
 err:
	kfree(phy);
 exit:
	return ret;
}

static
int start_scan(struct at76c503 *dev, int use_essid)
{
	struct at76c503_start_scan scan;

	memset(&scan, 0, sizeof(struct at76c503_start_scan));
	memset(scan.bssid, 0xff, ETH_ALEN);

	if (use_essid) {
		memcpy(scan.essid, dev->essid, IW_ESSID_MAX_SIZE);
		scan.essid_size = dev->essid_size;
	} else
		scan.essid_size = 0;

	scan.probe_delay = cpu_to_le16(10000);
	//jal: why should we start at a certain channel? we do scan the whole range
	//allowed by reg domain.
	scan.channel = dev->channel;

	/* atmelwlandriver differs between scan type 0 and 1 (active/passive)
	   For ad-hoc mode, it uses type 0 only.*/
	scan.scan_type = dev->scan_mode;
	scan.min_channel_time = cpu_to_le16(dev->scan_min_time);
	scan.max_channel_time = cpu_to_le16(dev->scan_max_time);
	/* other values are set to 0 for type 0 */

	return set_card_command(dev->udev, CMD_SCAN,
				(unsigned char*)&scan, sizeof(scan));
}

static
int start_ibss(struct at76c503 *dev)
{
	struct at76c503_start_bss bss;

	memset(&bss, 0, sizeof(struct at76c503_start_bss));
	memset(bss.bssid, 0xff, ETH_ALEN);
	memcpy(bss.essid, dev->essid, IW_ESSID_MAX_SIZE);
	bss.essid_size = dev->essid_size;
	bss.bss_type = ADHOC_MODE;
	bss.channel = dev->channel;

	return set_card_command(dev->udev, CMD_START_IBSS,
				(unsigned char*)&bss, sizeof(struct at76c503_start_bss));
}

/* idx points into dev->bss */
static
int join_bss(struct at76c503 *dev, struct bss_info *ptr)
{
	struct at76c503_join join;

	assert(ptr != NULL);

	memset(&join, 0, sizeof(struct at76c503_join));
	memcpy(join.bssid, ptr->bssid, ETH_ALEN);
	memcpy(join.essid, ptr->ssid, ptr->ssid_len);
	join.essid_size = ptr->ssid_len;
	join.bss_type = (dev->iw_mode == IW_MODE_ADHOC ? 1 : 2);
	join.channel = ptr->channel;
	join.timeout = cpu_to_le16(2000);

	dbg(DBG_PROGRESS, "%s join addr %s ssid %s type %d ch %d timeout %d",
	    dev->netdev->name, mac2str(join.bssid), 
	    join.essid, join.bss_type, join.channel, le16_to_cpu(join.timeout));
	return set_card_command(dev->udev, CMD_JOIN,
				(unsigned char*)&join,
				sizeof(struct at76c503_join));
} /* join_bss */

/* the firmware download timeout (after remap) */
void fw_dl_timeout(unsigned long par)
{
	struct at76c503 *dev = (struct at76c503 *)par;
	defer_kevent(dev, KEVENT_RESET_DEVICE);
}


/* the restart timer timed out */
void restart_timeout(unsigned long par)
{
	struct at76c503 *dev = (struct at76c503 *)par;
	defer_kevent(dev, KEVENT_RESTART);
}

/* we got to check the bss_list for old entries */
void bss_list_timeout(unsigned long par)
{
	struct at76c503 *dev = (struct at76c503 *)par;
	unsigned long flags;
	struct list_head *lptr, *nptr;
	struct bss_info *ptr;

	spin_lock_irqsave(&dev->bss_list_spinlock, flags);

	list_for_each_safe(lptr, nptr, &dev->bss_list) {

		ptr = list_entry(lptr, struct bss_info, list);

		if (ptr != dev->curr_bss && ptr != dev->new_bss &&
		    time_after(jiffies, ptr->last_rx+BSS_LIST_TIMEOUT)) {
			dbg(DBG_BSS_TABLE_RM,
			    "%s: bss_list: removing old BSS %s ch %d",
			    dev->netdev->name, mac2str(ptr->bssid), ptr->channel);
			list_del(&ptr->list);
			kfree(ptr);
		}
	}
	spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);
	/* restart the timer */
	mod_timer(&dev->bss_list_timer, jiffies+BSS_LIST_TIMEOUT);
	
}

/* we got a timeout for a infrastructure mgmt packet */
void mgmt_timeout(unsigned long par)
{
	struct at76c503 *dev = (struct at76c503 *)par;
	defer_kevent(dev, KEVENT_MGMT_TIMEOUT);
}

/* the deferred procedure called from kevent() */
void handle_mgmt_timeout(struct at76c503 *dev)
{

	if (dev->istate != SCANNING)
		/* this is normal behaviour in state SCANNING ... */
		dbg(DBG_PROGRESS, "%s: timeout, state %d", dev->netdev->name,
		    dev->istate);

	switch(dev->istate) {

	case SCANNING: /* we use the mgmt_timer to delay the next scan for some time */
		defer_kevent(dev, KEVENT_SCAN);
		break;

	case JOINING:
		assert(0);
		break;

	case CONNECTED: /* we haven't received the beacon of this BSS for 
			   BEACON_TIMEOUT seconds */
		info("%s: lost beacon bssid %s",
		     dev->netdev->name, mac2str(dev->curr_bss->bssid));
		/* jal: starting mgmt_timer in adhoc mode is questionable, 
		   but I'll leave it here to track down another lockup problem */
		if (dev->iw_mode != IW_MODE_ADHOC) {
			netif_carrier_off(dev->netdev);
			netif_stop_queue(dev->netdev);
			iwevent_bss_disconnect(dev->netdev);
			NEW_STATE(dev,SCANNING);
			defer_kevent(dev,KEVENT_SCAN);
		}
		break;

	case AUTHENTICATING:
		if (dev->retries-- >= 0) {
			auth_req(dev, dev->curr_bss, 1, NULL);
			mod_timer(&dev->mgmt_timer, jiffies+HZ);
		} else {
			/* try to get next matching BSS */
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
		}
		break;

	case ASSOCIATING:
		if (dev->retries-- >= 0) {
			assoc_req(dev,dev->curr_bss);
			mod_timer(&dev->mgmt_timer, jiffies+HZ);
		} else {
			/* jal: TODO: we may be authenticated to several
			   BSS and may try to associate to the next of them here
			   in the future ... */

			/* try to get next matching BSS */
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
		}
		break;

	case REASSOCIATING:
		if (dev->retries-- >= 0)
			reassoc_req(dev, dev->curr_bss, dev->new_bss);
		else {
			/* we disassociate from the curr_bss and
			   scan again ... */
			NEW_STATE(dev,DISASSOCIATING);
			dev->retries = DISASSOC_RETRIES;
			disassoc_req(dev, dev->curr_bss);
		}
		mod_timer(&dev->mgmt_timer, jiffies+HZ);
		break;

	case DISASSOCIATING:
		if (dev->retries-- >= 0) {
			disassoc_req(dev, dev->curr_bss);
			mod_timer(&dev->mgmt_timer,jiffies+HZ);
		} else {
			/* we scan again ... */
			NEW_STATE(dev,SCANNING);
			defer_kevent(dev,KEVENT_SCAN);
		}
		break;

	case INIT:
		break;

	default:
		assert(0);
	} /* switch (dev->istate) */

}/* handle_mgmt_timeout */

/* calc. the padding from txbuf->wlength (which excludes the USB TX header) 
   guess this is needed to compensate a flaw in the AT76C503A USB part ... */
static inline
int calc_padding(int wlen)
{
	/* add the USB TX header */
	wlen += AT76C503_TX_HDRLEN;

	wlen = wlen % 64;

	if (wlen < 50)
		return 50 - wlen;

	if (wlen >=61)
		return 64 + 50 - wlen;

	return 0;
}

/* send a management frame on bulk-out.
   txbuf->wlength must be set (in LE format !) */
static
int send_mgmt_bulk(struct at76c503 *dev, struct at76c503_tx_buffer *txbuf)
{
	unsigned long flags;
	int ret = 0;
	int urb_status;
	void *oldbuf = NULL;

	netif_carrier_off(dev->netdev); /* disable running netdev watchdog */
	netif_stop_queue(dev->netdev); /* stop tx data packets */

	spin_lock_irqsave(&dev->mgmt_spinlock, flags);

	if ((urb_status=dev->write_urb->status) == USB_ST_URB_PENDING) {
		oldbuf=dev->next_mgmt_bulk; /* to kfree below */
		dev->next_mgmt_bulk = txbuf;
		txbuf = NULL;
	}
	spin_unlock_irqrestore(&dev->mgmt_spinlock, flags);

	if (oldbuf) {
		/* a data/mgmt tx is already pending in the URB -
		   if this is no error in some situations we must
		   implement a queue or silently modify the old msg */
		err("%s: %s removed pending mgmt buffer %s",
		    dev->netdev->name, __FUNCTION__,
		    hex2str(dev->obuf, (u8 *)dev->next_mgmt_bulk,
			    min((int)(sizeof(dev->obuf))/3, 64),' '));
		kfree(dev->next_mgmt_bulk);
	}

	if (txbuf) {

		txbuf->tx_rate = 0;
//		txbuf->padding = 0;
		txbuf->padding = 
		   cpu_to_le16(calc_padding(le16_to_cpu(txbuf->wlength)));

		if (dev->next_mgmt_bulk) {
			err("%s: %s URB status %d, but mgmt is pending",
			    dev->netdev->name, __FUNCTION__, urb_status);
		}

		dbg(DBG_TX_MGMT, "%s: tx mgmt: wlen %d tx_rate %d pad %d %s",
		    dev->netdev->name, le16_to_cpu(txbuf->wlength),
		    txbuf->tx_rate, le16_to_cpu(txbuf->padding),
		    hex2str(dev->obuf, txbuf->packet,
			    min((sizeof(dev->obuf)-1)/2,
				(size_t)le16_to_cpu(txbuf->wlength)),'\0'));

		/* txbuf was not consumed above -> send mgmt msg immediately */
		memcpy(dev->bulk_out_buffer, txbuf,
		       le16_to_cpu(txbuf->wlength) + AT76C503_TX_HDRLEN);
		FILL_BULK_URB(dev->write_urb, dev->udev,
			      usb_sndbulkpipe(dev->udev, 
					      dev->bulk_out_endpointAddr),
			      dev->bulk_out_buffer,
			      le16_to_cpu(txbuf->wlength) + 
			      le16_to_cpu(txbuf->padding) +
			      AT76C503_TX_HDRLEN,
			      (usb_complete_t)at76c503_write_bulk_callback, dev);
		ret = submit_urb(dev->write_urb, GFP_ATOMIC);
		if (ret) {
			err("%s: %s error in tx submit urb: %d",
			    dev->netdev->name, __FUNCTION__, ret);
		}
		kfree(txbuf);
	} /* if (txbuf) */

	return ret;

} /* send_mgmt_bulk */

static
int disassoc_req(struct at76c503 *dev, struct bss_info *bss)
{
	struct at76c503_tx_buffer *tx_buffer;
	struct ieee802_11_mgmt *mgmt;
	struct ieee802_11_disassoc_frame *req;

	assert(bss != NULL);
	if (bss == NULL)
		return -EFAULT;

	tx_buffer = kmalloc(DISASSOC_FRAME_SIZE + MAX_PADDING_SIZE,
			    GFP_ATOMIC);
	if (!tx_buffer)
		return -ENOMEM;

	mgmt = (struct ieee802_11_mgmt *)&(tx_buffer->packet);
	req  = (struct ieee802_11_disassoc_frame *)&(mgmt->data);

	/* make wireless header */
	mgmt->frame_ctl = cpu_to_le16(IEEE802_11_FTYPE_MGMT|IEEE802_11_STYPE_AUTH);
	mgmt->duration_id = cpu_to_le16(0x8000);
	memcpy(mgmt->addr1, bss->bssid, ETH_ALEN);
	memcpy(mgmt->addr2, dev->netdev->dev_addr, ETH_ALEN);
	memcpy(mgmt->addr3, bss->bssid, ETH_ALEN);
	mgmt->seq_ctl = cpu_to_le16(0);

	req->reason = 0;

	/* init. at76c503 tx header */
	tx_buffer->wlength = cpu_to_le16(DISASSOC_FRAME_SIZE -
		AT76C503_TX_HDRLEN);
	
	dbg(DBG_TX_MGMT, "%s: DisAssocReq bssid %s",
	    dev->netdev->name, mac2str(mgmt->addr3));

	/* either send immediately (if no data tx is pending
	   or put it in pending list */
	return send_mgmt_bulk(dev, tx_buffer); 

} /* disassoc_req */

/* challenge is the challenge string (in TLV format) 
   we got with seq_nr 2 for shared secret authentication only and
   send in seq_nr 3 WEP encrypted to prove we have the correct WEP key;
   otherwise it is NULL */
static
int auth_req(struct at76c503 *dev, struct bss_info *bss, int seq_nr, u8 *challenge)
{
	struct at76c503_tx_buffer *tx_buffer;
	struct ieee802_11_mgmt *mgmt;
	struct ieee802_11_auth_frame *req;
	
	int buf_len = (seq_nr != 3 ? AUTH_FRAME_SIZE : 
		       AUTH_FRAME_SIZE + 1 + 1 + challenge[1]);

	assert(bss != NULL);
	assert(seq_nr != 3 || challenge != NULL);
	
	tx_buffer = kmalloc(buf_len + MAX_PADDING_SIZE, GFP_ATOMIC);
	if (!tx_buffer)
		return -ENOMEM;

	mgmt = (struct ieee802_11_mgmt *)&(tx_buffer->packet);
	req  = (struct ieee802_11_auth_frame *)&(mgmt->data);

	/* make wireless header */
	/* first auth msg is not encrypted, only the second (seq_nr == 3) */
	mgmt->frame_ctl = cpu_to_le16(IEEE802_11_FTYPE_MGMT | IEEE802_11_STYPE_AUTH |
		(seq_nr == 3 ? IEEE802_11_FCTL_WEP : 0));

	mgmt->duration_id = cpu_to_le16(0x8000);
	memcpy(mgmt->addr1, bss->bssid, ETH_ALEN);
	memcpy(mgmt->addr2, dev->netdev->dev_addr, ETH_ALEN);
	memcpy(mgmt->addr3, bss->bssid, ETH_ALEN);
	mgmt->seq_ctl = cpu_to_le16(0);

	req->algorithm = cpu_to_le16(dev->auth_mode);
	req->seq_nr = cpu_to_le16(seq_nr);
	req->status = cpu_to_le16(0);

	if (seq_nr == 3)
		memcpy(req->challenge, challenge, 1+1+challenge[1]);

	/* init. at76c503 tx header */
	tx_buffer->wlength = cpu_to_le16(buf_len - AT76C503_TX_HDRLEN);
	
	dbg(DBG_TX_MGMT, "%s: AuthReq bssid %s alg %d seq_nr %d",
	    dev->netdev->name, mac2str(mgmt->addr3),
	    le16_to_cpu(req->algorithm), le16_to_cpu(req->seq_nr));
	if (seq_nr == 3) {
		dbg(DBG_TX_MGMT, "%s: AuthReq challenge: %s ...",
		    dev->netdev->name,
		    hex2str(dev->obuf, req->challenge, 
			    min((int)sizeof(dev->obuf)/3, 18),' '));
	}

	/* either send immediately (if no data tx is pending
	   or put it in pending list */
	return send_mgmt_bulk(dev, tx_buffer); 

} /* auth_req */

static
int assoc_req(struct at76c503 *dev, struct bss_info *bss)
{
	struct at76c503_tx_buffer *tx_buffer;
	struct ieee802_11_mgmt *mgmt;
	struct ieee802_11_assoc_req *req;
	u8 *tlv;

	assert(bss != NULL);

	tx_buffer = kmalloc(ASSOCREQ_MAX_SIZE + MAX_PADDING_SIZE,
			    GFP_ATOMIC);
	if (!tx_buffer)
		return -ENOMEM;

	mgmt = (struct ieee802_11_mgmt *)&(tx_buffer->packet);
	req  = (struct ieee802_11_assoc_req *)&(mgmt->data);
	tlv = req->data;

	/* make wireless header */
	mgmt->frame_ctl = cpu_to_le16(IEEE802_11_FTYPE_MGMT|IEEE802_11_STYPE_ASSOC_REQ);

	mgmt->duration_id = cpu_to_le16(0x8000);
	memcpy(mgmt->addr1, bss->bssid, ETH_ALEN);
	memcpy(mgmt->addr2, dev->netdev->dev_addr, ETH_ALEN);
	memcpy(mgmt->addr3, bss->bssid, ETH_ALEN);
	mgmt->seq_ctl = cpu_to_le16(0);

	/* we must set the Privacy bit in the capabilites to assure an
	   Agere-based AP with optional WEP transmits encrypted frames
	   to us.  AP only set the Privacy bit in their capabilities
	   if WEP is mandatory in the BSS! */
	req->capability = cpu_to_le16(bss->capa | 
				      (dev->wep_enabled ? IEEE802_11_CAPA_PRIVACY : 0) |
				      (dev->preamble_type == PREAMBLE_TYPE_SHORT ?
				       IEEE802_11_CAPA_SHORT_PREAMBLE : 0));

	req->listen_interval = cpu_to_le16(2 * bss->beacon_interval);
	
	/* write TLV data elements */

	*tlv++ = IE_ID_SSID;
	*tlv++ = bss->ssid_len;
	memcpy(tlv, bss->ssid, bss->ssid_len);
	tlv += bss->ssid_len;

	*tlv++ = IE_ID_SUPPORTED_RATES;
	*tlv++ = sizeof(hw_rates);
	memcpy(tlv, hw_rates, sizeof(hw_rates));
	tlv += sizeof(hw_rates); /* tlv points behind the supp_rates field */

	/* init. at76c503 tx header */
	tx_buffer->wlength = cpu_to_le16(tlv-(u8 *)mgmt);
	
	{
		/* output buffer for ssid and rates */
		char orates[4*2+1] __attribute__ ((unused));
		int len;

		tlv = req->data;
		len = min(IW_ESSID_MAX_SIZE, (int)*(tlv+1));
		memcpy(dev->obuf, tlv+2, len);
		dev->obuf[len] = '\0';
		tlv += (1 + 1 + *(tlv+1)); /* points to IE of rates now */
		dbg(DBG_TX_MGMT, "%s: AssocReq bssid %s capa x%04x ssid %s rates %s",
		    dev->netdev->name, mac2str(mgmt->addr3),
		    le16_to_cpu(req->capability), dev->obuf,
		    hex2str(orates,tlv+2,min((sizeof(orates)-1)/2,(size_t)*(tlv+1)),
			    '\0'));
	}

	/* either send immediately (if no data tx is pending
	   or put it in pending list */
	return send_mgmt_bulk(dev, tx_buffer); 

} /* assoc_req */

/* we are currently associated to curr_bss and
   want to reassoc to new_bss */
static
int reassoc_req(struct at76c503 *dev, struct bss_info *curr_bss,
		struct bss_info *new_bss)
{
	struct at76c503_tx_buffer *tx_buffer;
	struct ieee802_11_mgmt *mgmt;
	struct ieee802_11_reassoc_req *req;
	
	u8 *tlv;

	assert(curr_bss != NULL);
	assert(new_bss != NULL);
	if (curr_bss == NULL || new_bss == NULL)
		return -EFAULT;

	tx_buffer = kmalloc(REASSOCREQ_MAX_SIZE + MAX_PADDING_SIZE,
			    GFP_ATOMIC);
	if (!tx_buffer)
		return -ENOMEM;

	mgmt = (struct ieee802_11_mgmt *)&(tx_buffer->packet);
	req  = (struct ieee802_11_reassoc_req *)&(mgmt->data);
	tlv = req->data;

	/* make wireless header */
	/* jal: encrypt this packet if wep_enabled is TRUE ??? */
	mgmt->frame_ctl = cpu_to_le16(IEEE802_11_FTYPE_MGMT|IEEE802_11_STYPE_REASSOC_REQ);
	mgmt->duration_id = cpu_to_le16(0x8000);
	memcpy(mgmt->addr1, new_bss->bssid, ETH_ALEN);
	memcpy(mgmt->addr2, dev->netdev->dev_addr, ETH_ALEN);
	memcpy(mgmt->addr3, new_bss->bssid, ETH_ALEN);
	mgmt->seq_ctl = cpu_to_le16(0);

	/* we must set the Privacy bit in the capabilites to assure an
	   Agere-based AP with optional WEP transmits encrypted frames
	   to us.  AP only set the Privacy bit in their capabilities
	   if WEP is mandatory in the BSS! */
	req->capability = cpu_to_le16(new_bss->capa | 
				      (dev->wep_enabled ? IEEE802_11_CAPA_PRIVACY : 0) |
				      (dev->preamble_type == PREAMBLE_TYPE_SHORT ?
				       IEEE802_11_CAPA_SHORT_PREAMBLE : 0));

	req->listen_interval = cpu_to_le16(2 * new_bss->beacon_interval);
	
	memcpy(req->curr_ap, curr_bss->bssid, ETH_ALEN);

	/* write TLV data elements */

	*tlv++ = IE_ID_SSID;
	*tlv++ = new_bss->ssid_len;
	memcpy(tlv,new_bss->ssid, new_bss->ssid_len);
	tlv += new_bss->ssid_len;

	*tlv++ = IE_ID_SUPPORTED_RATES;
	*tlv++ = sizeof(hw_rates);
	memcpy(tlv, hw_rates, sizeof(hw_rates));
	tlv += sizeof(hw_rates); /* tlv points behind the supp_rates field */

	/* init. at76c503 tx header */
	tx_buffer->wlength = cpu_to_le16(tlv-(u8 *)mgmt);
	
	{
		/* output buffer for rates and bssid */
		char orates[4*2+1] __attribute__ ((unused));
		char ocurr[6*3+1] __attribute__ ((unused));
		tlv = req->data;
		memcpy(dev->obuf, tlv+2, min(sizeof(dev->obuf),(size_t)*(tlv+1)));
		dev->obuf[IW_ESSID_MAX_SIZE] = '\0';
		tlv += (1 + 1 + *(tlv+1)); /* points to IE of rates now */
		dbg(DBG_TX_MGMT, "%s: ReAssocReq curr %s new %s capa x%04x ssid %s rates %s",
		    dev->netdev->name,
		    hex2str(ocurr, req->curr_ap, ETH_ALEN, ':'),
		    mac2str(mgmt->addr3), le16_to_cpu(req->capability), dev->obuf,
		    hex2str(orates,tlv+2,min((sizeof(orates)-1)/2,(size_t)*(tlv+1)),
			    '\0'));
	}

	/* either send immediately (if no data tx is pending
	   or put it in pending list */
	return send_mgmt_bulk(dev, tx_buffer); 

} /* reassoc_req */

static
int handle_scan(struct at76c503 *dev)
{
	int ret;
	
	if (dev->istate == INIT) {
		ret = -EPERM;
		goto end_scan;
	}
	assert(dev->istate == SCANNING);
	
	/* empty the driver's bss list */
	free_bss_list(dev);
	
	/* scan twice: first run with ProbeReq containing the
	   empty SSID, the second run with the real SSID.
	   APs in cloaked mode (e.g. Agere) will answer
	   in the second run with their real SSID. */
	
	if ((ret = start_scan(dev, 0)) < 0) {
		err("%s: start_scan failed with %d", dev->netdev->name, ret);
		goto end_scan;
	}
	if ((ret = wait_completion(dev,CMD_SCAN)) != CMD_STATUS_COMPLETE) {
		err("%s start_scan completed with %d",
		    dev->netdev->name, ret);
		goto end_scan;
	}
	
	/* dump the results of the scan with ANY ssid */
	dump_bss_table(dev, 0);
	
	if ((ret = start_scan(dev, 1)) < 0) {
		err("%s: 2.start_scan failed with %d", dev->netdev->name, ret);
			goto end_scan;
	}
	if ((ret = wait_completion(dev,CMD_SCAN)) != CMD_STATUS_COMPLETE) {
		err("%s 2.start_scan completed with %d", 
			dev->netdev->name, ret);
		goto end_scan;
	}
	
	/* dump the results of the scan with real ssid */
	dump_bss_table(dev, 0);
	
	iwevent_scan_complete(dev->netdev); /* report the end of scan to user space */

end_scan:
	return (ret < 0);
}

#if 0
/* == PROC re_register_intf ==
   re-register the interfaces with the driver core. Taken from
   usb.c:usb_new_device() after the call to usb_get_configuration() */
int re_register_intf(struct usb_device *dev)
{
	int i;

	dbg(DBG_DEVSTART, "%s: ENTER", __FUNCTION__);

	/* Register all of the interfaces for this device with the driver core.
	 * Remember, interfaces get bound to drivers, not devices. */
	for (i = 0; i < dev->actconfig->desc.bNumInterfaces; i++) {
		struct usb_interface *interface = &dev->actconfig->interface[i];
		struct usb_interface_descriptor *desc;

		desc = &interface->altsetting [interface->act_altsetting].desc;
		interface->dev.parent = &dev->dev;
		interface->dev.driver = NULL;
		interface->dev.bus = dev->dev.bus; /* &usb_bus_type */
		interface->dev.dma_mask = dev->dev.parent->dma_mask;
		sprintf (&interface->dev.bus_id[0], "%d-%s:%d",
			 dev->bus->busnum, dev->devpath,
			 desc->bInterfaceNumber);
		if (!desc->iInterface
		    || usb_string (dev, desc->iInterface,
				   interface->dev.name,
				   sizeof interface->dev.name) <= 0) {
			/* typically devices won't bother with interface
			 * descriptions; this is the normal case.  an
			 * interface's driver might describe it better.
			 * (also: iInterface is per-altsetting ...)
			 */
			sprintf (&interface->dev.name[0],
				 "usb-%s-%s interface %d",
				 dev->bus->bus_name, dev->devpath,
				 desc->bInterfaceNumber);
		}
		dev_dbg (&dev->dev, "%s - registering interface %s\n", __FUNCTION__,
			 interface->dev.bus_id);
		device_add (&interface->dev);
//		usb_create_driverfs_intf_files (interface);
	}

	dbg(DBG_DEVSTART, "%s: EXIT", __FUNCTION__);
	return 0;
} /* re_register_intf */
#endif

/* shamelessly copied from usbnet.c (oku) */
static void defer_kevent (struct at76c503 *dev, int flag)
{
	set_bit (flag, &dev->kevent_flags);
	if (!schedule_work (&dev->kevent))
		dbg(DBG_KEVENT, "%s: kevent %d may have been dropped",
		     dev->netdev->name, flag);
	else
		dbg(DBG_KEVENT, "%s: kevent %d scheduled",
		    dev->netdev->name, flag);
}

static void
kevent(void *data)
{
	struct at76c503 *dev = data;
	int ret;
	unsigned long flags;

	/* on errors, bits aren't cleared, but no reschedule
	   is done. So work will be done next time something
	   else has to be done. This is ugly. TODO! (oku) */

	dbg(DBG_KEVENT, "%s: kevent entry flags=x%lx", dev->netdev->name,
	    dev->kevent_flags);

	down(&dev->sem);

	if(test_bit(KEVENT_CTRL_HALT, &dev->kevent_flags)){
		/* this never worked... but it seems
		   that it's rarely necessary, if at all (oku) */
		ret = usb_clear_halt(dev->udev,
				     usb_sndctrlpipe (dev->udev, 0));
		if(ret < 0)
			err("usb_clear_halt() failed: %d", ret);
		else{
			clear_bit(KEVENT_CTRL_HALT, &dev->kevent_flags);
			info("usb_clear_halt() succesful");
		}
	}
	if(test_bit(KEVENT_NEW_BSS, &dev->kevent_flags)){
		struct net_device *netdev = dev->netdev;
		struct mib_mac_mgmt *mac_mgmt = kmalloc(sizeof(struct mib_mac_mgmt), GFP_KERNEL);

		ret = get_mib(dev->udev, MIB_MAC_MGMT, (u8*)mac_mgmt,
			      sizeof(struct mib_mac_mgmt));
		if(ret < 0){
			err("%s: get_mib failed: %d", netdev->name, ret);
			goto new_bss_clean;
		}

		dbg(DBG_PROGRESS, "ibss_change = 0x%2x", mac_mgmt->ibss_change);
		memcpy(dev->bssid, mac_mgmt->current_bssid, ETH_ALEN);
		dbg(DBG_PROGRESS, "using BSSID %s", mac2str(dev->bssid));
    
		iwevent_bss_connect(dev->netdev, dev->bssid);

		memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
		dev->mib_buf.type = MIB_MAC_MGMT;
		dev->mib_buf.size = 1;
		dev->mib_buf.index = IBSS_CHANGE_OK_OFFSET;
		ret = set_mib(dev, &dev->mib_buf);
		if(ret < 0){
			err("%s: set_mib (ibss change ok) failed: %d", netdev->name, ret);
			goto new_bss_clean;
		}
		clear_bit(KEVENT_NEW_BSS, &dev->kevent_flags);
	new_bss_clean:
		kfree(mac_mgmt);
	}
	if(test_bit(KEVENT_SET_PROMISC, &dev->kevent_flags)){
		info("%s: KEVENT_SET_PROMISC", dev->netdev->name);

		set_promisc(dev, dev->promisc);
		clear_bit(KEVENT_SET_PROMISC, &dev->kevent_flags);
	}

	if(test_bit(KEVENT_MGMT_TIMEOUT, &dev->kevent_flags)){
		clear_bit(KEVENT_MGMT_TIMEOUT, &dev->kevent_flags);
		handle_mgmt_timeout(dev);
	}

	/* check this _before_ KEVENT_JOIN, 'cause _JOIN sets _STARTIBSS bit */
	if (test_bit(KEVENT_STARTIBSS, &dev->kevent_flags)) {
		clear_bit(KEVENT_STARTIBSS, &dev->kevent_flags);
		assert(dev->istate == STARTIBSS);
		ret = start_ibss(dev);
		if(ret < 0){
			err("%s: start_ibss failed: %d", dev->netdev->name, ret);
			goto end_startibss;
		}

		ret = wait_completion(dev, CMD_START_IBSS);
		if (ret != CMD_STATUS_COMPLETE) {
			err("%s start_ibss failed to complete,%d",
			    dev->netdev->name, ret);
			goto end_startibss;
		}

		ret = get_current_bssid(dev);
		if(ret < 0) goto end_startibss;

		ret = get_current_channel(dev);
		if(ret < 0) goto end_startibss;

		/* not sure what this is good for ??? */
		memset(&dev->mib_buf, 0, sizeof(struct set_mib_buffer));
		dev->mib_buf.type = MIB_MAC_MGMT;
		dev->mib_buf.size = 1;
		dev->mib_buf.index = IBSS_CHANGE_OK_OFFSET;
		ret = set_mib(dev, &dev->mib_buf);
		if(ret < 0){
			err("%s: set_mib (ibss change ok) failed: %d", dev->netdev->name, ret);
			goto end_startibss;
		}

		netif_start_queue(dev->netdev);
	}
end_startibss:

	/* check this _before_ KEVENT_SCAN, 'cause _SCAN sets _JOIN bit */
	if (test_bit(KEVENT_JOIN, &dev->kevent_flags)) {
		clear_bit(KEVENT_JOIN, &dev->kevent_flags);
		if (dev->istate == INIT)
			goto end_join;
		assert(dev->istate == JOINING);

		/* dev->curr_bss == NULL signals a new round,
		   starting with list_entry(dev->bss_list.next, ...) */

		/* secure the access to dev->curr_bss ! */
		spin_lock_irqsave(&dev->bss_list_spinlock, flags);
		dev->curr_bss=find_matching_bss(dev, dev->curr_bss);
		spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);

		if (dev->curr_bss != NULL) {
			if ((ret=join_bss(dev,dev->curr_bss)) < 0) {
				err("%s: join_bss failed with %d",
				    dev->netdev->name, ret);
				goto end_join;
			}
			
			ret=wait_completion(dev,CMD_JOIN);
			if (ret != CMD_STATUS_COMPLETE) {
				if (ret != CMD_STATUS_TIME_OUT)
					err("%s join_bss completed with %d",
					    dev->netdev->name, ret);
				else
					info("%s join_bss ssid %s timed out",
						     dev->netdev->name,
					     mac2str(dev->curr_bss->bssid));

				/* retry next BSS immediately */
				defer_kevent(dev,KEVENT_JOIN);
				goto end_join;
			}

			/* here we have joined the (I)BSS */
			if (dev->iw_mode == IW_MODE_ADHOC) {
				struct bss_info *bptr = dev->curr_bss;
				NEW_STATE(dev,CONNECTED);
				/* get ESSID, BSSID and channel for dev->curr_bss */
				dev->essid_size = bptr->ssid_len;
				memcpy(dev->essid, bptr->ssid, bptr->ssid_len);
				memcpy(dev->bssid, bptr->bssid, ETH_ALEN);
				dev->channel = bptr->channel;
				iwevent_bss_connect(dev->netdev,bptr->bssid);
				netif_start_queue(dev->netdev);
				/* just to be sure */
				del_timer_sync(&dev->mgmt_timer);
			} else {
				/* send auth req */
				NEW_STATE(dev,AUTHENTICATING);
				auth_req(dev, dev->curr_bss, 1, NULL);
				mod_timer(&dev->mgmt_timer, jiffies+HZ);
			}
			goto end_join;
		} /* if (dev->curr_bss != NULL) */

		/* here we haven't found a matching (i)bss ... */
		if (dev->iw_mode == IW_MODE_ADHOC) {
			NEW_STATE(dev,STARTIBSS);
			defer_kevent(dev,KEVENT_STARTIBSS);
			goto end_join;
		}
		/* haven't found a matching BSS
		   in infra mode - use timer to try again in 10 seconds */
		NEW_STATE(dev,SCANNING);
		mod_timer(&dev->mgmt_timer, jiffies+RESCAN_TIME*HZ);
	} /* if (test_bit(KEVENT_JOIN, &dev->kevent_flags)) */
end_join:

	if (test_bit(KEVENT_SCAN, &dev->kevent_flags)) {
		clear_bit(KEVENT_SCAN, &dev->kevent_flags);
		
		if (handle_scan(dev)) {
			goto end_scan;
		}
		
		NEW_STATE(dev,JOINING);
		assert(dev->curr_bss == NULL); /* done in free_bss_list, 
						  find_bss will start with first bss */
		/* call join_bss immediately after
		   re-run of all other threads in kevent */
		defer_kevent(dev,KEVENT_JOIN);
	} /* if (test_bit(KEVENT_SCAN, &dev->kevent_flags)) */
end_scan:

	if (test_bit(KEVENT_SUBMIT_RX, &dev->kevent_flags)) {
		clear_bit(KEVENT_SUBMIT_RX, &dev->kevent_flags);
		submit_rx_urb(dev);
	}


	if (test_bit(KEVENT_RESTART, &dev->kevent_flags)) {
		clear_bit(KEVENT_RESTART, &dev->kevent_flags);
		assert(dev->istate == INIT);
		startup_device(dev);
		/* scan again in a second */
		NEW_STATE(dev,SCANNING);
		mod_timer(&dev->mgmt_timer, jiffies+HZ);
	}

	if (test_bit(KEVENT_ASSOC_DONE, &dev->kevent_flags)) {
		clear_bit(KEVENT_ASSOC_DONE, &dev->kevent_flags);
		assert(dev->istate == ASSOCIATING ||
		       dev->istate == REASSOCIATING);

		if (dev->iw_mode == IW_MODE_INFRA) {
			assert(dev->curr_bss != NULL);
			if (dev->curr_bss != NULL && 
			    dev->pm_mode != PM_ACTIVE) {
				/* calc the listen interval in units of
				   beacon intervals of the curr_bss */
			       dev->pm_period_beacon = (dev->pm_period_us >> 10) / 
					dev->curr_bss->beacon_interval;

#if 0 /* only to check if we need to set the listen interval here
         or could do it in the (re)assoc_req parameter */
				dump_mib_mac(dev);
#endif

				if (dev->pm_period_beacon < 2)
					dev->pm_period_beacon = 2;
				else
					if ( dev->pm_period_beacon > 0xffff)
						dev->pm_period_beacon = 0xffff;

				dbg(DBG_PM, "%s: pm_mode %d assoc id x%x listen int %d",
				    dev->netdev->name, dev->pm_mode,
				    dev->curr_bss->assoc_id, dev->pm_period_beacon);

				set_associd(dev, dev->curr_bss->assoc_id);
				set_listen_interval(dev, (u16)dev->pm_period_beacon);
				set_pm_mode(dev, dev->pm_mode);
#if 0
				dump_mib_mac(dev);
				dump_mib_mac_mgmt(dev);
#endif
			}
		}

		netif_carrier_on(dev->netdev);
		netif_wake_queue(dev->netdev); /* _start_queue ??? */
		NEW_STATE(dev,CONNECTED);
		iwevent_bss_connect(dev->netdev,dev->curr_bss->bssid);
		dbg(DBG_PROGRESS, "%s: connected to BSSID %s",
		    dev->netdev->name, mac2str(dev->curr_bss->bssid));
	}


	if (test_bit(KEVENT_RESET_DEVICE, &dev->kevent_flags)) {

		clear_bit(KEVENT_RESET_DEVICE, &dev->kevent_flags);

		dbg(DBG_DEVSTART, "resetting the device");

		usb_reset_device(dev->udev);

/* with 2.6.8 the reset above will cause a disconnect as the USB subsys
   recognizes the change in the config descriptors. Subsequently the device
   will be registered again. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
		//jal: patch the state (patch by Dmitri)
		dev->udev->state = USB_STATE_CONFIGURED;
#endif
		/* jal: currently (2.6.0-test2 and 2.4.23) 
		   usb_reset_device() does not recognize that
		   the interface descr. are changed.
		   This procedure reads the configuration and does a limited parsing of
		   the interface and endpoint descriptors */
		update_usb_intf_descr(dev);

		/* continue immediately with external fw download */
		set_bit (KEVENT_EXTERNAL_FW, &dev->kevent_flags);
#else
		/* kernel >= 2.6.8 */
		NEW_STATE(dev, WAIT_FOR_DISCONNECT);
#endif
	}

	if (test_bit(KEVENT_EXTERNAL_FW, &dev->kevent_flags)) {
		u8 op_mode;

		clear_bit(KEVENT_EXTERNAL_FW, &dev->kevent_flags);

		op_mode = get_op_mode(dev->udev);
		dbg(DBG_DEVSTART, "opmode %d", op_mode);
	
		if (op_mode != OPMODE_NORMAL_NIC_WITHOUT_FLASH) {
			err("unexpected opmode %d", op_mode);
			goto end_external_fw;
		}

		if (dev->extfw && dev->extfw_size) {
			ret = download_external_fw(dev->udev, dev->extfw,
						   dev->extfw_size);
			if (ret < 0) {
				err("Downloading external firmware failed: %d", ret);
				goto end_external_fw;
			}
		}
		NEW_STATE(dev,INIT);
		init_new_device(dev);
	}
end_external_fw:

	if (test_bit(KEVENT_INTERNAL_FW, &dev->kevent_flags)) {
		clear_bit(KEVENT_INTERNAL_FW, &dev->kevent_flags);

		dbg(DBG_DEVSTART, "downloading internal firmware");

		ret=usbdfu_download(dev->udev, dev->intfw,
				    dev->intfw_size);

		if (ret < 0) {
			err("downloading internal fw failed with %d",ret);
			goto end_internal_fw;
		}
 
		dbg(DBG_DEVSTART, "sending REMAP");

		if ((ret=at76c503_remap(dev->udev)) < 0) {
			err("sending REMAP failed with %d",ret);
			goto end_internal_fw;
		}

		dbg(DBG_DEVSTART, "sleeping for 2 seconds");
		NEW_STATE(dev,EXTFW_DOWNLOAD);
		mod_timer(&dev->fw_dl_timer, jiffies+2*HZ+1);
	}
end_internal_fw:

	up(&dev->sem);

	dbg(DBG_KEVENT, "%s: kevent exit flags=x%lx", dev->netdev->name,
	    dev->kevent_flags);

	return;
}

static
int essid_matched(struct at76c503 *dev, struct bss_info *ptr)
{
	/* common criteria for both modi */

	int retval = (dev->essid_size == 0  /* ANY ssid */ ||
		      (dev->essid_size == ptr->ssid_len &&
		       !memcmp(dev->essid, ptr->ssid, ptr->ssid_len)));
	if (!retval)
		dbg(DBG_BSS_MATCH, "%s bss table entry %p: essid didn't match",
		    dev->netdev->name, ptr);
	return retval;
}

static inline
int mode_matched(struct at76c503 *dev, struct bss_info *ptr)
{
	int retval;

	if (dev->iw_mode == IW_MODE_ADHOC)
		retval =  ptr->capa & IEEE802_11_CAPA_IBSS;
	else
		retval =  ptr->capa & IEEE802_11_CAPA_ESS;
	if (!retval)
		dbg(DBG_BSS_MATCH, "%s bss table entry %p: mode didn't match",
		    dev->netdev->name, ptr);
	return retval;
}

static
int rates_matched(struct at76c503 *dev, struct bss_info *ptr)
{
	int i;
	u8 *rate;

	for(i=0,rate=ptr->rates; i < ptr->rates_len; i++,rate++)
		if (*rate & 0x80) {
			/* this is a basic rate we have to support
			   (see IEEE802.11, ch. 7.3.2.2) */
			if (*rate != (0x80|hw_rates[0]) && *rate != (0x80|hw_rates[1]) &&
			    *rate != (0x80|hw_rates[2]) && *rate != (0x80|hw_rates[3])) {
				dbg(DBG_BSS_MATCH,
				    "%s: bss table entry %p: basic rate %02x not supported",
				     dev->netdev->name, ptr, *rate);
				return 0;
			}
		}
	/* if we use short preamble, the bss must support it */
	if (dev->preamble_type == PREAMBLE_TYPE_SHORT &&
	    !(ptr->capa & IEEE802_11_CAPA_SHORT_PREAMBLE)) {
		dbg(DBG_BSS_MATCH, "%s: %p does not support short preamble",
		    dev->netdev->name, ptr);
		return 0;
	} else
		return 1;
}

static inline
int wep_matched(struct at76c503 *dev, struct bss_info *ptr)
{
	if (!dev->wep_enabled && 
	    ptr->capa & IEEE802_11_CAPA_PRIVACY) {
		/* we have disabled WEP, but the BSS signals privacy */
		dbg(DBG_BSS_MATCH, "%s: bss table entry %p: requires encryption",
		    dev->netdev->name, ptr);
		return 0;
	}
	/* otherwise if the BSS does not signal privacy it may well
	   accept encrypted packets from us ... */
	return 1;
}

static inline
int bssid_matched(struct at76c503 *dev, struct bss_info *ptr)
{
	if (!dev->wanted_bssid_valid ||
		!memcmp(ptr->bssid, dev->wanted_bssid, ETH_ALEN)) {
		return 1;
	} else {
		if (debug & DBG_BSS_MATCH) {
			dbg_uc("%s: requested bssid - %s does not match", 
				dev->netdev->name, mac2str(dev->wanted_bssid));
			dbg_uc("       AP bssid - %s of bss table entry %p", 
				mac2str(ptr->bssid), ptr);
		}
		return 0;
	}
}

static void dump_bss_table(struct at76c503 *dev, int force_output)
{
	struct bss_info *ptr;
	/* hex dump output buffer for debug */
	unsigned long flags;
	struct list_head *lptr;

	if ((debug & DBG_BSS_TABLE) || (force_output)) {
		spin_lock_irqsave(&dev->bss_list_spinlock, flags);

		dbg_uc("%s BSS table (curr=%p, new=%p):", dev->netdev->name,
		       dev->curr_bss, dev->new_bss);

		list_for_each(lptr, &dev->bss_list) {
			ptr = list_entry(lptr, struct bss_info, list);
			dbg_uc("0x%p: bssid %s channel %d ssid %s (%s)"
			    " capa x%04x rates %s rssi %d link %d noise %d",
			    ptr, mac2str(ptr->bssid),
			    ptr->channel,
			    ptr->ssid,
			    hex2str(dev->obuf, ptr->ssid,
				    min((sizeof(dev->obuf)-1)/2,
					(size_t)ptr->ssid_len),'\0'),
			    le16_to_cpu(ptr->capa),
			    hex2str(dev->obuf_s, ptr->rates, 
				    min(sizeof(dev->obuf_s)/3,
					(size_t)ptr->rates_len), ' '),
			       ptr->rssi, ptr->link_qual, ptr->noise_level);
		}

		spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);
	}
}

/* try to find a matching bss in dev->bss, starting at position start.
   returns the ptr to a matching bss in the list or
   NULL if none found */
/* last is the last bss tried, last == NULL signals a new round,
   starting with list_entry(dev->bss_list.next, ...) */
/* this proc must be called inside an aquired dev->bss_list_spinlock
   otherwise the timeout on bss may remove the newly chosen entry ! */
static struct bss_info *find_matching_bss(struct at76c503 *dev,
					  struct bss_info *last)
{
	struct bss_info *ptr = NULL;
	struct list_head *curr;

	curr  = last != NULL ? last->list.next : dev->bss_list.next;
	while (curr != &dev->bss_list) {
		ptr = list_entry(curr, struct bss_info, list);
		if (essid_matched(dev,ptr) &&
		    mode_matched(dev,ptr)  &&
		    wep_matched(dev,ptr)   &&
		    rates_matched(dev,ptr) &&
		    bssid_matched(dev,ptr))
			break;
		curr = curr->next;
	}

	if (curr == &dev->bss_list)
		ptr = NULL;
	/* otherwise ptr points to the struct bss_info we have chosen */

	dbg(DBG_BSS_TABLE, "%s %s: returned %p", dev->netdev->name,
	    __FUNCTION__, ptr);
	return ptr;
} /* find_matching_bss */


/* we got an association response */
static void rx_mgmt_assoc(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;
	struct ieee802_11_assoc_resp *resp = 
		(struct ieee802_11_assoc_resp *)mgmt->data;
	u16 assoc_id = le16_to_cpu(resp->assoc_id);
	u16 status = le16_to_cpu(resp->status);
	u16 capa = le16_to_cpu(resp->capability); 
	dbg(DBG_RX_MGMT, "%s: rx AssocResp bssid %s capa x%04x status x%04x "
	    "assoc_id x%04x rates %s",
	    dev->netdev->name, mac2str(mgmt->addr3), capa, status, assoc_id,
	    hex2str(dev->obuf, resp->data+2,
		    min((size_t)*(resp->data+1),(sizeof(dev->obuf)-1)/2), '\0'));

	if (dev->istate == ASSOCIATING) {

		assert(dev->curr_bss != NULL);
		if (dev->curr_bss == NULL)
			return;

		if (status == IEEE802_11_STATUS_SUCCESS) {
			struct bss_info *ptr = dev->curr_bss;
			ptr->assoc_id = assoc_id & 0x3fff;
			/* update iwconfig params */
			memcpy(dev->bssid, ptr->bssid, ETH_ALEN);
			memcpy(dev->essid, ptr->ssid, ptr->ssid_len);
			dev->essid_size = ptr->ssid_len;
			dev->channel = ptr->channel;
			defer_kevent(dev,KEVENT_ASSOC_DONE);
		} else {
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
		}
		del_timer_sync(&dev->mgmt_timer);
	} else
		info("%s: AssocResp in state %d ignored",
		     dev->netdev->name, dev->istate);
} /* rx_mgmt_assoc */

static void rx_mgmt_reassoc(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;
	struct ieee802_11_assoc_resp *resp = 
		(struct ieee802_11_assoc_resp *)mgmt->data;
	unsigned long flags;
	u16 capa = le16_to_cpu(resp->capability);
	u16 status = le16_to_cpu(resp->status);
	u16 assoc_id = le16_to_cpu(resp->assoc_id);

	dbg(DBG_RX_MGMT, "%s: rx ReAssocResp bssid %s capa x%04x status x%04x "
	    "assoc_id x%04x rates %s",
	    dev->netdev->name, mac2str(mgmt->addr3), capa, status, assoc_id,
	    hex2str(dev->obuf, resp->data+2,
		    min((size_t)*(resp->data+1),(sizeof(dev->obuf)-1)/2), '\0'));

	if (dev->istate == REASSOCIATING) {

		assert(dev->new_bss != NULL);
		if (dev->new_bss == NULL)
			return;

		if (status == IEEE802_11_STATUS_SUCCESS) {
			struct bss_info *bptr = dev->new_bss;
			bptr->assoc_id = assoc_id;
			NEW_STATE(dev,CONNECTED);

			iwevent_bss_connect(dev->netdev,bptr->bssid);

			spin_lock_irqsave(&dev->bss_list_spinlock, flags);
			dev->curr_bss = dev->new_bss;
			dev->new_bss = NULL;
			spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);

			/* get ESSID, BSSID and channel for dev->curr_bss */
			dev->essid_size = bptr->ssid_len;
			memcpy(dev->essid, bptr->ssid, bptr->ssid_len);
			memcpy(dev->bssid, bptr->bssid, ETH_ALEN);
			dev->channel = bptr->channel;
			dbg(DBG_PROGRESS, "%s: reassociated to BSSID %s",
			    dev->netdev->name, mac2str(dev->bssid));
			defer_kevent(dev, KEVENT_ASSOC_DONE);
		} else {
			del_timer_sync(&dev->mgmt_timer);
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
		}
	} else
		info("%s: ReAssocResp in state %d ignored",
		     dev->netdev->name, dev->istate);
} /* rx_mgmt_reassoc */

static void rx_mgmt_disassoc(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;
	struct ieee802_11_disassoc_frame *resp = 
		(struct ieee802_11_disassoc_frame *)mgmt->data;

	dbg(DBG_RX_MGMT, "%s: rx DisAssoc bssid %s reason x%04x destination %s",
	    dev->netdev->name, mac2str(mgmt->addr3),
	    le16_to_cpu(resp->reason),
	    hex2str(dev->obuf, mgmt->addr1, 
		    min((int)sizeof(dev->obuf)/3, ETH_ALEN), ':'));

	if (dev->istate == SCANNING || dev->istate == INIT)
		return;

	assert(dev->curr_bss != NULL);
	if (dev->curr_bss == NULL)
		return;

	if (dev->istate == REASSOCIATING) {
		assert(dev->new_bss != NULL);
		if (dev->new_bss == NULL)
			return;
	}

	if (!memcmp(mgmt->addr3, dev->curr_bss->bssid, ETH_ALEN) &&
		(!memcmp(dev->netdev->dev_addr, mgmt->addr1, ETH_ALEN) ||
			!memcmp(bc_addr, mgmt->addr1, ETH_ALEN))) {
		/* this is a DisAssoc from the BSS we are connected or
		   trying to connect to, directed to us or broadcasted */
		/* jal: TODO: can the Disassoc also come from the BSS
		   we've sent a ReAssocReq to (i.e. from dev->new_bss) ? */
		if (dev->istate == DISASSOCIATING ||
		    dev->istate == ASSOCIATING  ||
		    dev->istate == REASSOCIATING  ||
		    dev->istate == CONNECTED  ||
		    dev->istate == JOINING)
		{
			if (dev->istate == CONNECTED) {
				netif_carrier_off(dev->netdev);
				netif_stop_queue(dev->netdev);
				iwevent_bss_disconnect(dev->netdev);
			}
			del_timer_sync(&dev->mgmt_timer);
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
		} else
		/* ignore DisAssoc in states AUTH, ASSOC */
			info("%s: DisAssoc in state %d ignored",
			     dev->netdev->name, dev->istate);
	}
	/* ignore DisAssoc to other STA or from other BSSID */
} /* rx_mgmt_disassoc */

static void rx_mgmt_auth(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;
	struct ieee802_11_auth_frame *resp = 
		(struct ieee802_11_auth_frame *)mgmt->data;
	int seq_nr = le16_to_cpu(resp->seq_nr);
	int alg = le16_to_cpu(resp->algorithm);
	int status = le16_to_cpu(resp->status);

	dbg(DBG_RX_MGMT, "%s: rx AuthFrame bssid %s alg %d seq_nr %d status %d " 
	    "destination %s",
	    dev->netdev->name, mac2str(mgmt->addr3),
	    alg, seq_nr, status,
	    hex2str(dev->obuf, mgmt->addr1,
		    min((int)sizeof(dev->obuf)/3, ETH_ALEN), ':'));

	if (alg == IEEE802_11_AUTH_ALG_SHARED_SECRET &&
	    seq_nr == 2) {
		dbg(DBG_RX_MGMT, "%s: AuthFrame challenge %s ...",
		    dev->netdev->name,
		    hex2str(dev->obuf, resp->challenge,
			    min((int)sizeof(dev->obuf)/3,18), ' '));
	}

	if (dev->istate != AUTHENTICATING) {
		info("%s: ignored AuthFrame in state %d",
		     dev->netdev->name, dev->istate);
		return;
	}
	if (dev->auth_mode != alg) {
		info("%s: ignored AuthFrame for alg %d",
		     dev->netdev->name, alg);
		return;
	}

	assert(dev->curr_bss != NULL);
	if (dev->curr_bss == NULL)
		return;

	if (!memcmp(mgmt->addr3, dev->curr_bss->bssid, ETH_ALEN) &&
	    !memcmp(dev->netdev->dev_addr, mgmt->addr1, ETH_ALEN)) {
		/* this is a AuthFrame from the BSS we are connected or
		   trying to connect to, directed to us */
		if (status != IEEE802_11_STATUS_SUCCESS) {
			del_timer_sync(&dev->mgmt_timer);
			/* try to join next bss */
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
			return;
		}

		if (dev->auth_mode == IEEE802_11_AUTH_ALG_OPEN_SYSTEM ||
			seq_nr == 4) {
			dev->retries = ASSOC_RETRIES;
			NEW_STATE(dev,ASSOCIATING);
			assoc_req(dev, dev->curr_bss);
			mod_timer(&dev->mgmt_timer,jiffies+HZ);
			return;
		}

		assert(seq_nr == 2);
		auth_req(dev, dev->curr_bss, seq_nr+1, resp->challenge);
		mod_timer(&dev->mgmt_timer,jiffies+HZ);
	}
	/* else: ignore AuthFrames to other receipients */
} /* rx_mgmt_auth */

static void rx_mgmt_deauth(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;
	struct ieee802_11_deauth_frame *resp = 
		(struct ieee802_11_deauth_frame *)mgmt->data;

	dbg(DBG_RX_MGMT|DBG_PROGRESS,
	    "%s: rx DeAuth bssid %s reason x%04x destination %s",
	    dev->netdev->name, mac2str(mgmt->addr3),
	    le16_to_cpu(resp->reason),
	    hex2str(dev->obuf, mgmt->addr1,
		    min((int)sizeof(dev->obuf)/3,ETH_ALEN), ':'));

	if (dev->istate == DISASSOCIATING ||
	    dev->istate == AUTHENTICATING ||
	    dev->istate == ASSOCIATING ||
	    dev->istate == REASSOCIATING  ||
	    dev->istate == CONNECTED) {

		assert(dev->curr_bss != NULL);
		if (dev->curr_bss == NULL)
			return;

		if (!memcmp(mgmt->addr3, dev->curr_bss->bssid, ETH_ALEN) &&
		(!memcmp(dev->netdev->dev_addr, mgmt->addr1, ETH_ALEN) ||
		 !memcmp(bc_addr, mgmt->addr1, ETH_ALEN))) {
			/* this is a DeAuth from the BSS we are connected or
			   trying to connect to, directed to us or broadcasted */
			if (dev->istate == CONNECTED)
				iwevent_bss_disconnect(dev->netdev);
			NEW_STATE(dev,JOINING);
			defer_kevent(dev,KEVENT_JOIN);
			del_timer_sync(&dev->mgmt_timer);
		}
		/* ignore DeAuth to other STA or from other BSSID */
	} else {
		/* ignore DeAuth in states SCANNING */
		info("%s: DeAuth in state %d ignored",
		     dev->netdev->name, dev->istate);
	}
} /* rx_mgmt_deauth */

static void rx_mgmt_beacon(struct at76c503 *dev,
			   struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = (struct ieee802_11_mgmt *)buf->packet;

	/* beacon content */
	struct ieee802_11_beacon_data *bdata = 
		(struct ieee802_11_beacon_data *)mgmt->data;

	/* length of var length beacon parameters */
	int varpar_len = min((int)buf->wlength -
			     (int)(offsetof(struct ieee802_11_mgmt, data) +
			      offsetof(struct ieee802_11_beacon_data, data)),
			     BEACON_MAX_DATA_LENGTH);

	struct list_head *lptr;
	struct bss_info *match; /* entry matching addr3 with its bssid */
	int new_entry = 0;
	int len;
	struct data_element {
		u8 type;
		u8 length;
		u8 data_head;
	} *element;
	int have_ssid = 0;
	int have_rates = 0;
	int have_channel = 0;
	int keep_going = 1;
	unsigned long flags;
	
	spin_lock_irqsave(&dev->bss_list_spinlock, flags);
	
	if (dev->istate == CONNECTED) {
		/* in state CONNECTED we use the mgmt_timer to control
		   the beacon of the BSS */
		assert(dev->curr_bss != NULL);
		if (dev->curr_bss == NULL)
			goto rx_mgmt_beacon_end;
		if (!memcmp(dev->curr_bss->bssid, mgmt->addr3, ETH_ALEN)) {
			mod_timer(&dev->mgmt_timer, jiffies+BEACON_TIMEOUT*HZ);
			dev->curr_bss->rssi = buf->rssi;
			goto rx_mgmt_beacon_end;
		}
	}

	/* look if we have this BSS already in the list */
	match = NULL;

	if (!list_empty(&dev->bss_list)) {
		list_for_each(lptr, &dev->bss_list) {
			struct bss_info *bss_ptr = 
				list_entry(lptr, struct bss_info, list);
			if (!memcmp(bss_ptr->bssid, mgmt->addr3, ETH_ALEN)) {
				match = bss_ptr;
				break;
			}
		}
	}

	if (match == NULL) {
		/* haven't found the bss in the list */
		if ((match=kmalloc(sizeof(struct bss_info), GFP_ATOMIC)) == NULL) {
			dbg(DBG_BSS_TABLE, "%s: cannot kmalloc new bss info (%d byte)",
			    dev->netdev->name, sizeof(struct bss_info));
			goto rx_mgmt_beacon_end;
		}
		memset(match,0,sizeof(*match));
		new_entry = 1;
		/* append new struct into list */
		list_add_tail(&match->list, &dev->bss_list);
	}
	
	/* we either overwrite an existing entry or append a new one
	   match points to the entry in both cases */
	
	match->capa = le16_to_cpu(bdata->capability_information);
	
	/* while beacon_interval is not (!) */
	match->beacon_interval = le16_to_cpu(bdata->beacon_interval);
	
	match->rssi = buf->rssi;
	match->link_qual = buf->link_quality;
	match->noise_level = buf->noise_level;
	
	memcpy(match->mac,mgmt->addr2,ETH_ALEN); //just for info
	memcpy(match->bssid,mgmt->addr3,ETH_ALEN);
	dbg(DBG_RX_BEACON, "%s: bssid %s", dev->netdev->name, 
			mac2str(match->bssid));
	
	element = (struct data_element*)bdata->data;
	
#define data_end(element) (&(element->data_head) + element->length)
	
	// This routine steps through the bdata->data array to try and get 
	// some useful information about the access point.
	// Currently, this implementation supports receipt of: SSID, 
	// supported transfer rates and channel, in any order, with some 
	// tolerance for intermittent unknown codes (although this 
	// functionality may not be necessary as the useful information will 
	// usually arrive in consecutively, but there have been some 
	// reports of some of the useful information fields arriving in a 
	// different order).
	// It does not support any more IE_ID types as although IE_ID_TIM may 
	// be supported (on my AP at least).  
	// The bdata->data array is about 1500 bytes long but only ~36 of those 
	// bytes are useful, hence the have_ssid etc optimistaions.

	while (keep_going &&
	       ((int)(data_end(element) - bdata->data) <= varpar_len)) {

		switch (element->type) {
		
		case IE_ID_SSID:
			len = min(IW_ESSID_MAX_SIZE, (int)element->length);
			if (!have_ssid && ((new_entry) || 
                                           !is_cloaked_ssid(&(element->data_head), len))) {
			/* we copy only if this is a new entry,
			   or the incoming SSID is not a cloaked SSID. This 
			   will protect us from overwriting a real SSID read 
			   in a ProbeResponse with a cloaked one from a 
			   following beacon. */
			
				match->ssid_len = len;
				memcpy(match->ssid, &(element->data_head), len);
				match->ssid[len] = '\0'; /* terminate the 
							    string for 
							    printing */
				dbg(DBG_RX_BEACON, "%s: SSID - %s", 
					dev->netdev->name, match->ssid);
			}
			have_ssid = 1;
			break;
		
		case IE_ID_SUPPORTED_RATES:
			if (!have_rates) {
				match->rates_len = 
					min((int)sizeof(match->rates), 
						(int)element->length);
				memcpy(match->rates, &(element->data_head), 
					match->rates_len);
				have_rates = 1;
				dbg(DBG_RX_BEACON, 
				    "%s: SUPPORTED RATES %s", 
				    dev->netdev->name,
				    hex2str(dev->obuf, &(element->data_head),
					    min((sizeof(dev->obuf)-1)/2,
						(size_t)element->length), '\0'));
			}
			break;
		
		case IE_ID_DS_PARAM_SET:
			if (!have_channel) {
				match->channel = element->data_head;
				have_channel = 1;
				dbg(DBG_RX_BEACON, "%s: CHANNEL - %d", 
					dev->netdev->name, match->channel);
			}
			break;
		
		case IE_ID_CF_PARAM_SET:
		case IE_ID_TIM:
		case IE_ID_IBSS_PARAM_SET:
		default:
		{
			dbg(DBG_RX_BEACON, "%s: beacon IE id %d len %d %s",
			    dev->netdev->name, element->type, element->length,
			    hex2str(dev->obuf,&(element->data_head),
				    min((sizeof(dev->obuf)-1)/2,
					(size_t)element->length),'\0'));
			break;
		}

		} // switch (element->type)
		
		// advance to the next 'element' of data
		element = (struct data_element*)data_end(element);

		// Optimisation: after all, the bdata->data array is  
		// varpar_len bytes long, whereas we get all of the useful 
		// information after only ~36 bytes, this saves us a lot of 
		// time (and trouble as the remaining portion of the array 
		// could be full of junk)
		// Comment this out if you want to see what other information
		// comes from the AP - although little of it may be useful

		//if (have_ssid && have_rates && have_channel)
		//	keep_going = 0;
	}
	
	dbg(DBG_RX_BEACON, "%s: Finished processing beacon data", 
		dev->netdev->name);
	
	match->last_rx = jiffies; /* record last rx of beacon */
	
rx_mgmt_beacon_end:
	spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);
} /* rx_mgmt_beacon */


static void rx_mgmt(struct at76c503 *dev, struct at76c503_rx_buffer *buf)
{
	struct ieee802_11_mgmt *mgmt = ( struct ieee802_11_mgmt *)buf->packet;
	struct iw_statistics *wstats = &dev->wstats;
	u16 lev_dbm;
	u16 subtype = le16_to_cpu(mgmt->frame_ctl) & IEEE802_11_FCTL_STYPE;

	/* update wstats */
	if (dev->istate != INIT && dev->istate != SCANNING) {
		/* jal: this is a dirty hack needed by Tim in adhoc mode */
		if (dev->iw_mode == IW_MODE_ADHOC ||
		    (dev->curr_bss != NULL &&
		     !memcmp(mgmt->addr3, dev->curr_bss->bssid, ETH_ALEN))) {
			/* Data packets always seem to have a 0 link level, so we
			   only read link quality info from management packets.
			   Atmel driver actually averages the present, and previous
			   values, we just present the raw value at the moment - TJS */
			
			if (buf->rssi > 1) {
				lev_dbm = (buf->rssi * 5 / 2);
				if (lev_dbm > 255)
					lev_dbm = 255;
				wstats->qual.qual    = buf->link_quality;
				wstats->qual.level   = lev_dbm;
				wstats->qual.noise   = buf->noise_level;
				wstats->qual.updated = 7;
			}
		}
	}

	if (debug & DBG_RX_MGMT_CONTENT) {
		dbg_uc("%s rx mgmt subtype x%x %s",
		       dev->netdev->name, subtype,
		       hex2str(dev->obuf, (u8 *)mgmt, 
			       min((sizeof(dev->obuf)-1)/2,
				   (size_t)le16_to_cpu(buf->wlength)), '\0'));
	}

	switch (subtype) {
	case IEEE802_11_STYPE_BEACON:
	case IEEE802_11_STYPE_PROBE_RESP:
		rx_mgmt_beacon(dev,buf);
		break;

	case IEEE802_11_STYPE_ASSOC_RESP:
		rx_mgmt_assoc(dev,buf);
		break;

	case IEEE802_11_STYPE_REASSOC_RESP:
		rx_mgmt_reassoc(dev,buf);
		break;

	case IEEE802_11_STYPE_DISASSOC:
		rx_mgmt_disassoc(dev,buf);
		break;

	case IEEE802_11_STYPE_AUTH:
		rx_mgmt_auth(dev,buf);
		break;

	case IEEE802_11_STYPE_DEAUTH:
		rx_mgmt_deauth(dev,buf);
		break;

	default:
		info("%s: mgmt, but not beacon, subtype = %x",
		     dev->netdev->name, subtype);
	}

	return;
}

static void dbg_dumpbuf(const char *tag, const u8 *buf, int size)
{
	int i;

	if (!debug) return;

	for (i=0; i<size; i++) {
		if ((i % 8) == 0) {
			if (i) printk("\n");
			printk(KERN_DEBUG __FILE__ ": %s: ", tag);
		}
		printk("%02x ", buf[i]);
	}
	printk("\n");
}

/* A short overview on Ethernet-II, 802.2, 802.3 and SNAP
   (taken from http://www.geocities.com/billalexander/ethernet.html):

Ethernet Frame Formats:

Ethernet (a.k.a. Ethernet II)

        +---------+---------+---------+----------
        |   Dst   |   Src   |  Type   |  Data... 
        +---------+---------+---------+----------

         <-- 6 --> <-- 6 --> <-- 2 --> <-46-1500->

         Type 0x80 0x00 = TCP/IP
         Type 0x06 0x00 = XNS
         Type 0x81 0x37 = Novell NetWare
         

802.3

        +---------+---------+---------+----------
        |   Dst   |   Src   | Length  | Data...  
        +---------+---------+---------+----------

         <-- 6 --> <-- 6 --> <-- 2 --> <-46-1500->

802.2 (802.3 with 802.2 header)

        +---------+---------+---------+-------+-------+-------+----------
        |   Dst   |   Src   | Length  | DSAP  | SSAP  |Control| Data...  
        +---------+---------+---------+-------+-------+-------+----------

                                       <- 1 -> <- 1 -> <- 1 -> <-43-1497->

SNAP (802.3 with 802.2 and SNAP headers) 

        +---------+---------+---------+-------+-------+-------+-----------+---------+-----------
        |   Dst   |   Src   | Length  | 0xAA  | 0xAA  | 0x03  |  Org Code |   Type  | Data...   
        +---------+---------+---------+-------+-------+-------+-----------+---------+-----------

                                                               <--  3  --> <-- 2 --> <-38-1492->

*/

/* Convert the 802.11 header on a packet into an ethernet-style header
 * (basically, pretend we're an ethernet card receiving ethernet packets)
 *
 * This routine returns with the skbuff pointing to the actual data (just past
 * the end of the newly-created ethernet header).
 */
static void ieee80211_to_eth(struct sk_buff *skb, int iw_mode)
{
	struct ieee802_11_hdr *i802_11_hdr;
	struct ethhdr *eth_hdr;
	u8 *src_addr;
	u8 *dest_addr;
	unsigned short proto = 0;
	int build_ethhdr = 1;

	i802_11_hdr = (struct ieee802_11_hdr *)skb->data;

#if 0
	{
		dbg_uc("%s: ENTRY skb len %d data %s", __FUNCTION__,
		       skb->len, hex2str(dev->obuf, skb->data,
					 min((int)sizeof(dev->obuf)/3,64), ' '));
	}
#endif

	skb_pull(skb, sizeof(struct ieee802_11_hdr));
//	skb_trim(skb, skb->len - 4); /* Trim CRC */

	src_addr = iw_mode == IW_MODE_ADHOC ? i802_11_hdr->addr2
	       				    : i802_11_hdr->addr3;
	dest_addr = i802_11_hdr->addr1;

	eth_hdr = (struct ethhdr *)skb->data;
	if (!memcmp(eth_hdr->h_source, src_addr, ETH_ALEN) &&
	    !memcmp(eth_hdr->h_dest, dest_addr, ETH_ALEN)) {
		/* An ethernet frame is encapsulated within the data portion.
		 * Just use its header instead. */
		skb_pull(skb, sizeof(struct ethhdr));
		build_ethhdr = 0;
	} else if (!memcmp(skb->data, snapsig, sizeof(snapsig))) {
		/* SNAP frame - collapse it */
		skb_pull(skb, sizeof(rfc1042sig)+2);
		proto = *(unsigned short *)(skb->data - 2);
	} else {
#ifdef IEEE_STANDARD
		/* According to all standards, we should assume the data
		 * portion contains 802.2 LLC information, so we should give it
		 * an 802.3 header (which has the same implications) */
		proto = htons(skb->len);
#else /* IEEE_STANDARD */
		/* Unfortunately, it appears no actual 802.11 implementations
		 * follow any standards specs.  They all appear to put a
		 * 16-bit ethertype after the 802.11 header instead, so we take
		 * that value and make it into an Ethernet-II packet. */
		/* Note that this means we can never support non-SNAP 802.2
		 * frames (because we can't tell when we get one) */

		/* jal: This isn't true. My WRT54G happily sends SNAP.
		   Difficult to speak for all APs, so I don't dare to define
		   IEEE_STANDARD ... */
		proto = *(unsigned short *)(skb->data);
		skb_pull(skb, 2);
#endif /* IEEE_STANDARD */
	}

	eth_hdr = (struct ethhdr *)(skb->data-sizeof(struct ethhdr));
	skb->mac.ethernet = eth_hdr;
	if (build_ethhdr) {
		/* This needs to be done in this order (eth_hdr->h_dest may
		 * overlap src_addr) */
		memcpy(eth_hdr->h_source, src_addr, ETH_ALEN);
		memcpy(eth_hdr->h_dest, dest_addr, ETH_ALEN);
		/* make an 802.3 header (proto = length) */
		eth_hdr->h_proto = proto;
	}

	if (ntohs(eth_hdr->h_proto) > 1518) {
		skb->protocol = eth_hdr->h_proto;
	} else if (*(unsigned short *)skb->data == 0xFFFF) {
		/* Magic hack for Novell IPX-in-802.3 packets */
		skb->protocol = htons(ETH_P_802_3);
	} else {
		/* Assume it's an 802.2 packet (it should be, and we have no
		 * good way to tell if it isn't) */
		skb->protocol = htons(ETH_P_802_2);
	}

#if 0
	{
		char da[3*ETH_ALEN], sa[3*ETH_ALEN];
		dbg_uc("%s: EXIT skb da %s sa %s proto x%04x len %d data %s", __FUNCTION__,
		       hex2str(da, skb->mac.ethernet->h_dest, ETH_ALEN, ':'),
		       hex2str(sa, skb->mac.ethernet->h_source, ETH_ALEN, ':'),
		       ntohs(skb->protocol), skb->len,
		       hex2str(dev->obuf, skb->data, 
			       min((int)sizeof(dev->obuf)/3,64), ' '));
	}
#endif

}

/* Adjust the skb to trim the hardware header and CRC, and set up skb->mac,
 * skb->protocol, etc.
 */ 
static void ieee80211_fixup(struct sk_buff *skb, int iw_mode)
{
	struct ieee802_11_hdr *i802_11_hdr;
	struct ethhdr *eth_hdr;
	u8 *src_addr;
	u8 *dest_addr;
	unsigned short proto = 0;

	i802_11_hdr = (struct ieee802_11_hdr *)skb->data;
	skb_pull(skb, sizeof(struct ieee802_11_hdr));
//	skb_trim(skb, skb->len - 4); /* Trim CRC */

	src_addr = iw_mode == IW_MODE_ADHOC ? i802_11_hdr->addr2
	       				    : i802_11_hdr->addr3;
	dest_addr = i802_11_hdr->addr1;

	skb->mac.raw = (unsigned char *)i802_11_hdr;

	eth_hdr = (struct ethhdr *)skb->data;
	if (!memcmp(eth_hdr->h_source, src_addr, ETH_ALEN) &&
	    !memcmp(eth_hdr->h_dest, dest_addr, ETH_ALEN)) {
		/* There's an ethernet header encapsulated within the data
		 * portion, count it as part of the hardware header */
		skb_pull(skb, sizeof(struct ethhdr));
		proto = eth_hdr->h_proto;
	} else if (!memcmp(skb->data, snapsig, sizeof(snapsig))) {
		/* SNAP frame - collapse it */
		/* RFC1042/802.1h encapsulated packet.  Treat the SNAP header
		 * as part of the HW header and note the protocol. */
		/* NOTE: prism2 doesn't collapse Appletalk frames (why?). */
		skb_pull(skb, sizeof(rfc1042sig) + 2);
		proto = *(unsigned short *)(skb->data - 2);
	}

	if (ntohs(proto) > 1518) {
		skb->protocol = proto;
	} else {
#ifdef IEEE_STANDARD
		/* According to all standards, we should assume the data
		 * portion contains 802.2 LLC information */
		skb->protocol = htons(ETH_P_802_2);
#else /* IEEE_STANDARD */
		/* Unfortunately, it appears no actual 802.11 implementations
		 * follow any standards specs.  They all appear to put a
		 * 16-bit ethertype after the 802.11 header instead, so we'll
		 * use that (and consider it part of the hardware header). */
		/* Note that this means we can never support non-SNAP 802.2
		 * frames (because we can't tell when we get one) */
		skb->protocol = *(unsigned short *)skb->data;
		skb_pull(skb, 2);
#endif /* IEEE_STANDARD */
	}
}

/* check for fragmented data in dev->rx_skb. If the packet was no fragment
   or it was the last of a fragment set a skb containing the whole packet
   is returned for further processing. Otherwise we get NULL and are
   done and the packet is either stored inside the fragment buffer
   or thrown away. The check for rx_copybreak is moved here.
   Every returned skb starts with the ieee802_11 header and contains
   _no_ FCS at the end */
static struct sk_buff *check_for_rx_frags(struct at76c503 *dev)
{	
	struct sk_buff *skb = (struct sk_buff *)dev->rx_skb;
	struct at76c503_rx_buffer *buf = (struct at76c503_rx_buffer *)skb->data;
	struct ieee802_11_hdr *i802_11_hdr =
		(struct ieee802_11_hdr *)buf->packet;
	/* seq_ctrl, fragment_number, sequence number of new packet */
	u16 sctl = le16_to_cpu(i802_11_hdr->seq_ctl);
	u16 fragnr = sctl & 0xf;
	u16 seqnr = sctl>>4;
	u16 frame_ctl = le16_to_cpu(i802_11_hdr->frame_ctl);

	/* length including the IEEE802.11 header, excl. the trailing FCS,
	   excl. the struct at76c503_rx_buffer */
	int length = le16_to_cpu(buf->wlength) - dev->rx_data_fcs_len;
	
	/* where does the data payload start in skb->data ? */
	u8 *data = (u8 *)i802_11_hdr + sizeof(struct ieee802_11_hdr);

	/* length of payload, excl. the trailing FCS */
	int data_len = length - (data - (u8 *)i802_11_hdr);

	int i;
	struct rx_data_buf *bptr, *optr;
	unsigned long oldest = ~0UL;

	dbg(DBG_RX_FRAGS, "%s: rx data frame_ctl %04x addr2 %s seq/frag %d/%d "
	    "length %d data %d: %s ...",
	    dev->netdev->name, frame_ctl,
	    mac2str(i802_11_hdr->addr2),
	    seqnr, fragnr, length, data_len,
	    hex2str(dev->obuf, data,
		    min((int)(sizeof(dev->obuf)-1)/2,32), '\0'));

	dbg(DBG_RX_FRAGS_SKB, "%s: incoming skb: head %p data %p "
	    "tail %p end %p len %d",
	    dev->netdev->name, skb->head, skb->data, skb->tail,
	    skb->end, skb->len);

	if (data_len <= 0) {
		/* buffers contains no data */
		info("%s: rx skb without data", dev->netdev->name);
		return NULL;
	}

	if (fragnr == 0 && !(frame_ctl & IEEE802_11_FCTL_MOREFRAGS)) {
		/* unfragmented packet received */
		if (length < rx_copybreak && (skb = dev_alloc_skb(length)) != NULL) {
			memcpy(skb_put(skb, length),
			       dev->rx_skb->data + AT76C503_RX_HDRLEN, length);
		} else {
			skb_pull(skb, AT76C503_RX_HDRLEN);
			skb_trim(skb, length);
			/* Use a new skb for the next receive */
			dev->rx_skb = NULL;
		}

		dbg(DBG_RX_FRAGS, "%s: unfragmented", dev->netdev->name);

		return skb;
	}

	/* remove the at76c503_rx_buffer header - we don't need it anymore */
	/* we need the IEEE802.11 header (for the addresses) if this packet
	   is the first of a chain */

	assert(length  > AT76C503_RX_HDRLEN);
	skb_pull(skb, AT76C503_RX_HDRLEN);
	/* remove FCS at end */
	skb_trim(skb, length);

	dbg(DBG_RX_FRAGS_SKB, "%s: trimmed skb: head %p data %p tail %p "
	    "end %p len %d data %p data_len %d",
	    dev->netdev->name, skb->head, skb->data, skb->tail,
	    skb->end, skb->len, data, data_len);

	/* look if we've got a chain for the sender address.
	   afterwards optr points to first free or the oldest entry,
	   or, if i < NR_RX_DATA_BUF, bptr points to the entry for the
	   sender address */
	/* determining the oldest entry doesn't cope with jiffies wrapping
	   but I don't care to delete a young entry at these rare moments ... */

	for(i=0,bptr=dev->rx_data,optr=NULL; i < NR_RX_DATA_BUF; i++,bptr++) {
		if (bptr->skb != NULL) {
			if (!memcmp(i802_11_hdr->addr2, bptr->sender,ETH_ALEN))
				break;
			else
				if (optr == NULL) {
					optr = bptr;
					oldest = bptr->last_rx;
				} else {
					if (bptr->last_rx < oldest)
						optr = bptr;
				}
		} else {
			optr = bptr;
			oldest = 0UL;
		}
	}
	
	if (i < NR_RX_DATA_BUF) {

		dbg(DBG_RX_FRAGS, "%s: %d. cacheentry (seq/frag=%d/%d) "
		    "matched sender addr",
		    dev->netdev->name, i, bptr->seqnr, bptr->fragnr);

		/* bptr points to an entry for the sender address */
		if (bptr->seqnr == seqnr) {
			int left;
			/* the fragment has the current sequence number */
			if (((bptr->fragnr+1)&0xf) == fragnr) {
				bptr->last_rx = jiffies;
				/* the next following fragment number ->
				    add the data at the end */
				/* is & 0xf necessary above ??? */

				// for test only ???
				if ((left=skb_tailroom(bptr->skb)) < data_len) {
					info("%s: only %d byte free (need %d)",
					    dev->netdev->name, left, data_len);
				} else 
					memcpy(skb_put(bptr->skb, data_len),
					       data, data_len);
				bptr->fragnr = fragnr;
				if (!(frame_ctl &
				      IEEE802_11_FCTL_MOREFRAGS)) {
					/* this was the last fragment - send it */
					skb = bptr->skb;
					bptr->skb = NULL; /* free the entry */
					dbg(DBG_RX_FRAGS, "%s: last frag of seq %d",
					    dev->netdev->name, seqnr);
					return skb;
				} else
					return NULL;
			} else {
				/* wrong fragment number -> ignore it */
				dbg(DBG_RX_FRAGS, "%s: frag nr does not match: %d+1 != %d",
				    dev->netdev->name, bptr->fragnr, fragnr);
				return NULL;
			}
		} else {
			/* got another sequence number */
			if (fragnr == 0) {
				/* it's the start of a new chain - replace the
				   old one by this */
				/* bptr->sender has the correct value already */
				dbg(DBG_RX_FRAGS, "%s: start of new seq %d, "
				    "removing old seq %d", dev->netdev->name,
				    seqnr, bptr->seqnr);
				bptr->seqnr = seqnr;
				bptr->fragnr = 0;
				bptr->last_rx = jiffies;
				/* swap bptr->skb and dev->rx_skb */
				skb = bptr->skb;
				bptr->skb = dev->rx_skb;
				dev->rx_skb = skb;
			} else {
				/* it from the middle of a new chain ->
				   delete the old entry and skip the new one */
				dbg(DBG_RX_FRAGS, "%s: middle of new seq %d (%d) "
				    "removing old seq %d", dev->netdev->name,
				    seqnr, fragnr, bptr->seqnr);
				dev_kfree_skb(bptr->skb);
				bptr->skb = NULL;
			}
			return NULL;
		}
	} else {
		/* if we didn't find a chain for the sender address optr
		   points either to the first free or the oldest entry */

		if (fragnr != 0) {
			/* this is not the begin of a fragment chain ... */
			dbg(DBG_RX_FRAGS, "%s: no chain for non-first fragment (%d)",
			    dev->netdev->name, fragnr);
			return NULL;
		}
		assert(optr != NULL);
		if (optr == NULL)
			return NULL;

		if (optr->skb != NULL) {
			/* swap the skb's */
			skb = optr->skb;
			optr->skb = dev->rx_skb;
			dev->rx_skb = skb;

			dbg(DBG_RX_FRAGS, "%s: free old contents: sender %s seq/frag %d/%d",
			    dev->netdev->name, mac2str(optr->sender),
			    optr->seqnr, optr->fragnr); 

		} else {
			/* take the skb from dev->rx_skb */
			optr->skb = dev->rx_skb;
			dev->rx_skb = NULL; /* let submit_rx_urb() allocate a new skb */

			dbg(DBG_RX_FRAGS, "%s: use a free entry", dev->netdev->name);
		}
		memcpy(optr->sender, i802_11_hdr->addr2, ETH_ALEN);
		optr->seqnr = seqnr;
		optr->fragnr = 0;
		optr->last_rx = jiffies;
		
		return NULL;
	}
} /* check_for_rx_frags */

/* rx interrupt: we expect the complete data buffer in dev->rx_skb */
static void rx_data(struct at76c503 *dev)
{
	struct net_device *netdev = (struct net_device *)dev->netdev;
	struct net_device_stats *stats = &dev->stats;
	struct sk_buff *skb = dev->rx_skb;
	struct at76c503_rx_buffer *buf = (struct at76c503_rx_buffer *)skb->data;
	struct ieee802_11_hdr *i802_11_hdr;
	int length = le16_to_cpu(buf->wlength);

	if (debug & DBG_RX_DATA) {
		dbg_uc("%s received data packet:", netdev->name);
		dbg_dumpbuf(" rxhdr", skb->data, AT76C503_RX_HDRLEN);
	}
	if (debug & DBG_RX_DATA_CONTENT)
		dbg_dumpbuf("packet", skb->data + AT76C503_RX_HDRLEN,
			    length);

	if ((skb=check_for_rx_frags(dev)) == NULL)
		return;

	/* if an skb is returned, the at76c503a_rx_header and the FCS is already removed */
	i802_11_hdr = (struct ieee802_11_hdr *)skb->data;

	skb->dev = netdev;
	skb->ip_summed = CHECKSUM_NONE; /* TODO: should check CRC */

	if (i802_11_hdr->addr1[0] & 1) {
		if (!memcmp(i802_11_hdr->addr1, netdev->broadcast, ETH_ALEN))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	} else if (memcmp(i802_11_hdr->addr1, netdev->dev_addr, ETH_ALEN)) {
		skb->pkt_type=PACKET_OTHERHOST;
	}

	if (netdev->type == ARPHRD_ETHER) {
		ieee80211_to_eth(skb, dev->iw_mode);
	} else {
		ieee80211_fixup(skb, dev->iw_mode);
	}

	netdev->last_rx = jiffies;
	netif_rx(skb);
	stats->rx_packets++;
	stats->rx_bytes += length;

	return;
}

static int submit_rx_urb(struct at76c503 *dev)
{
	int ret, size;
	struct sk_buff *skb = dev->rx_skb;
	
	if (dev->read_urb == NULL) {
		err("%s: dev->read_urb is NULL", __FUNCTION__);
		return -EFAULT;
	}

	if (skb == NULL) {
		skb = dev_alloc_skb(sizeof(struct at76c503_rx_buffer));
		if (skb == NULL) {
			err("%s: unable to allocate rx skbuff.", dev->netdev->name);
			ret = -ENOMEM;
			goto exit;
		}
		dev->rx_skb = skb;
	} else {
		skb_push(skb, skb_headroom(skb));
		skb_trim(skb, 0);
	}

	size = skb_tailroom(skb);
	FILL_BULK_URB(dev->read_urb, dev->udev,
		usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
		skb_put(skb, size), size,
		(usb_complete_t)at76c503_read_bulk_callback, dev);
	ret = submit_urb(dev->read_urb, GFP_ATOMIC);
	if (ret < 0) { 
		if (ret == -ENODEV) 
			dbg(DBG_DEVSTART, "usb_submit_urb returned -ENODEV");
		else
			err("%s: rx, usb_submit_urb failed: %d", dev->netdev->name, ret);
	}

exit:
	if (ret < 0) {
		if (ret != -ENODEV) {
			/* If we can't submit the URB, the adapter becomes completely
		 	* useless, so try again later */
			if (--dev->nr_submit_rx_tries > 0)
				defer_kevent(dev, KEVENT_SUBMIT_RX);
			else {
				err("%s: giving up to submit rx urb after %d failures -"
			    	    " please unload the driver and/or power cycle the device",
			    	    dev->netdev->name, NR_SUBMIT_RX_TRIES);
			}
		}
	} else
		/* reset counter to initial value */
		dev->nr_submit_rx_tries = NR_SUBMIT_RX_TRIES;
	return ret;
}


/* we are doing a lot of things here in an interrupt. Need
   a bh handler (Watching TV with a TV card is probably
   a good test: if you see flickers, we are doing too much.
   Currently I do see flickers... even with our tasklet :-( )
   Maybe because the bttv driver and usb-uhci use the same interrupt
*/
/* Or maybe because our BH handler is preempting bttv's BH handler.. BHs don't
 * solve everything.. (alex) */
static void at76c503_read_bulk_callback (struct urb *urb)
{
	struct at76c503 *dev = (struct at76c503 *)urb->context;

	dev->rx_urb = urb;
	tasklet_schedule(&dev->tasklet);

	return;
}

static void rx_tasklet(unsigned long param)
{
	struct at76c503 *dev = (struct at76c503 *)param;
	struct urb *urb;
	struct net_device *netdev;
	struct at76c503_rx_buffer *buf;
	struct ieee802_11_hdr *i802_11_hdr;
	u16 frame_ctl;

	if (!dev) return;
	urb = dev->rx_urb;
	netdev = (struct net_device *)dev->netdev;

	if (dev->flags & AT76C503A_UNPLUG) {
		dbg(DBG_DEVSTART, "flag UNPLUG set");
		if (urb)
			dbg(DBG_DEVSTART, "urb status %d", urb->status);
		return;
	}


	if(!urb || !dev->rx_skb || !netdev || !dev->rx_skb->data) return;

	buf = (struct at76c503_rx_buffer *)dev->rx_skb->data;

	if (!buf) return;

	i802_11_hdr = (struct ieee802_11_hdr *)buf->packet;
	if (!i802_11_hdr) return;

	frame_ctl = le16_to_cpu(i802_11_hdr->frame_ctl);

	if(urb->status != 0){
		if ((urb->status != -ENOENT) && 
		    (urb->status != -ECONNRESET)) {
			dbg(DBG_URB,"%s %s: - nonzero read bulk status received: %d",
			    __FUNCTION__, netdev->name, urb->status);
			goto no_more_urb;
		}
		return;
	}

	/* there is a new bssid around, accept it: */
	if(buf->newbss && dev->iw_mode == IW_MODE_ADHOC){
		dbg(DBG_PROGRESS, "%s: rx newbss", netdev->name);
		defer_kevent(dev, KEVENT_NEW_BSS);
	}

	if (debug & DBG_RX_ATMEL_HDR) {
		dbg_uc("%s: rx frame: rate %d rssi %d noise %d link %d %s",
		    dev->netdev->name,
		    buf->rx_rate, buf->rssi, buf->noise_level,
		    buf->link_quality,
		    hex2str(dev->obuf,(u8 *)i802_11_hdr,
			    min((int)(sizeof(dev->obuf)-1)/2,48),'\0'));
	}

	switch (frame_ctl & IEEE802_11_FCTL_FTYPE) {
	case IEEE802_11_FTYPE_DATA:
		rx_data(dev);
		break;

	case IEEE802_11_FTYPE_MGMT:
		/* jal: TODO: find out if we can update iwspy also on
		   other frames than management (might depend on the
		   radio chip / firmware version !) */

		iwspy_update(dev, buf);

		rx_mgmt(dev, buf);
		break;

	case IEEE802_11_FTYPE_CTL:
		dbg(DBG_RX_CTRL, "%s: ignored ctrl frame: %04x", dev->netdev->name,
		    frame_ctl);
		break;

	default:
		info("%s: it's a frame from mars: %2x", dev->netdev->name,
		     frame_ctl);
	} /* switch (frame_ctl & IEEE802_11_FCTL_FTYPE) */

	submit_rx_urb(dev);
 no_more_urb:
	return;
}

static void at76c503_write_bulk_callback (struct urb *urb)
{
	struct at76c503 *dev = (struct at76c503 *)urb->context;
	struct net_device_stats *stats = &dev->stats;
	unsigned long flags;
	struct at76c503_tx_buffer *mgmt_buf;
	int ret;

	if(urb->status != 0){
		if((urb->status != -ENOENT) && 
		   (urb->status != -ECONNRESET)) {
			dbg(DBG_URB, "%s - nonzero write bulk status received: %d",
			    __FUNCTION__, urb->status);
		}else
			return; /* urb has been unlinked */
		stats->tx_errors++;
	}else
		stats->tx_packets++;

	spin_lock_irqsave(&dev->mgmt_spinlock, flags);
	mgmt_buf = dev->next_mgmt_bulk;
	dev->next_mgmt_bulk = NULL;
	spin_unlock_irqrestore(&dev->mgmt_spinlock, flags);

	if (mgmt_buf) {
		/* we don't copy the padding bytes, but add them
		   to the length */
		memcpy(dev->bulk_out_buffer, mgmt_buf,
		       le16_to_cpu(mgmt_buf->wlength) +
		       offsetof(struct at76c503_tx_buffer,packet));
		FILL_BULK_URB(dev->write_urb, dev->udev,
			      usb_sndbulkpipe(dev->udev, 
					      dev->bulk_out_endpointAddr),
			      dev->bulk_out_buffer,
			      le16_to_cpu(mgmt_buf->wlength) + 
			      le16_to_cpu(mgmt_buf->padding) +
			      AT76C503_TX_HDRLEN,
			      (usb_complete_t)at76c503_write_bulk_callback, dev);
		ret = submit_urb(dev->write_urb, GFP_ATOMIC);
		if (ret) {
			err("%s: %s error in tx submit urb: %d",
			    dev->netdev->name, __FUNCTION__, ret);
		}
		kfree(mgmt_buf);
	} else
		netif_wake_queue(dev->netdev);

}

static int
at76c503_tx(struct sk_buff *skb, struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)(netdev->priv);
	struct net_device_stats *stats = &dev->stats;
	int ret = 0;
	int wlen;
	int submit_len;
	struct at76c503_tx_buffer *tx_buffer =
		(struct at76c503_tx_buffer *)dev->bulk_out_buffer;
	struct ieee802_11_hdr *i802_11_hdr =
		(struct ieee802_11_hdr *)&(tx_buffer->packet);
	u8 *payload = tx_buffer->packet + sizeof(struct ieee802_11_hdr);

	if (netif_queue_stopped(netdev)) {
		err("%s: %s called while netdev is stopped", netdev->name,
		    __FUNCTION__);
		//skip this packet
		dev_kfree_skb(skb);
		return 0;
	}

	if (dev->write_urb->status == USB_ST_URB_PENDING) {
		err("%s: %s called while dev->write_urb is pending for tx",
		    netdev->name, __FUNCTION__);
		//skip this packet
		dev_kfree_skb(skb);
		return 0;
	}

	if (skb->len < 2*ETH_ALEN) {
		err("%s: %s: skb too short (%d)", dev->netdev->name,
		    __FUNCTION__, skb->len);
			dev_kfree_skb(skb);
			return 0;
	}

	/* we can get rid of memcpy, if we set netdev->hard_header_len
	   to 8 + sizeof(struct ieee802_11_hdr), because then we have
	   enough space */
	//  dbg(DBG_TX, "skb->data - skb->head = %d", skb->data - skb->head);

	if (ntohs(*(u16 *)(skb->data + 2*ETH_ALEN)) <= 1518) {
		/* this is a 802.3 packet */
		if (skb->data[2*ETH_ALEN+2] == rfc1042sig[0] &&
		    skb->data[2*ETH_ALEN+2+1] == rfc1042sig[1]) {
			/* higher layer delivered SNAP header - keep it */
			memcpy(payload, skb->data + 2*ETH_ALEN+2, skb->len - 2*ETH_ALEN -2);
			wlen = sizeof(struct ieee802_11_hdr) + skb->len - 2*ETH_ALEN -2;
		} else {
			err("%s: %s: no support for non-SNAP 802.2 packets "
			    "(DSAP x%02x SSAP x%02x cntrl x%02x)",
			    dev->netdev->name, __FUNCTION__,
			    skb->data[2*ETH_ALEN+2], skb->data[2*ETH_ALEN+2+1],
			    skb->data[2*ETH_ALEN+2+2]);
			dev_kfree_skb(skb);
			return 0;
		}
	} else {
		/* add RFC 1042 header in front */
		memcpy(payload, rfc1042sig, sizeof(rfc1042sig));
		memcpy(payload + sizeof(rfc1042sig),
		       skb->data + 2*ETH_ALEN, skb->len - 2*ETH_ALEN);
		wlen = sizeof(struct ieee802_11_hdr) + sizeof(rfc1042sig) + 
			skb->len - 2*ETH_ALEN;
	}

	/* make wireless header */
	i802_11_hdr->frame_ctl =
		cpu_to_le16(IEEE802_11_FTYPE_DATA |
			    (dev->wep_enabled ? IEEE802_11_FCTL_WEP : 0) |
			    (dev->iw_mode == IW_MODE_INFRA ? IEEE802_11_FCTL_TODS : 0));

	if(dev->iw_mode == IW_MODE_ADHOC){
		memcpy(i802_11_hdr->addr1, skb->data, ETH_ALEN); /* destination */
		memcpy(i802_11_hdr->addr2, skb->data+ETH_ALEN, ETH_ALEN); /* source */
		memcpy(i802_11_hdr->addr3, dev->bssid, ETH_ALEN);
	}else if(dev->iw_mode == IW_MODE_INFRA){
		memcpy(i802_11_hdr->addr1, dev->bssid, ETH_ALEN);
		memcpy(i802_11_hdr->addr2, skb->data+ETH_ALEN, ETH_ALEN); /* source */
		memcpy(i802_11_hdr->addr3, skb->data, ETH_ALEN); /* destination */
	}

	i802_11_hdr->duration_id = cpu_to_le16(0);
	i802_11_hdr->seq_ctl = cpu_to_le16(0);

	/* setup 'atmel' header */
	tx_buffer->wlength = cpu_to_le16(wlen);
	tx_buffer->tx_rate = dev->txrate; 
        /* for broadcast destination addresses, the firmware 0.100.x 
	   seems to choose the highest rate set with CMD_STARTUP in
	   basic_rate_set replacing this value */
	
	memset(tx_buffer->reserved, 0, 4);

	tx_buffer->padding = cpu_to_le16(calc_padding(wlen));
	submit_len = wlen + AT76C503_TX_HDRLEN + 
		le16_to_cpu(tx_buffer->padding);

	{
		dbg(DBG_TX_DATA_CONTENT, "%s skb->data %s", dev->netdev->name,
		    hex2str(dev->obuf, skb->data, 
			    min((int)(sizeof(dev->obuf)-1)/2,32),'\0'));
		dbg(DBG_TX_DATA, "%s tx  wlen x%x pad x%x rate %d hdr %s",
		    dev->netdev->name,
		    le16_to_cpu(tx_buffer->wlength),
		    le16_to_cpu(tx_buffer->padding), tx_buffer->tx_rate, 
		    hex2str(dev->obuf, (u8 *)i802_11_hdr, 
			    min((sizeof(dev->obuf)-1)/2, 
				sizeof(struct ieee802_11_hdr)),'\0'));
		dbg(DBG_TX_DATA_CONTENT, "%s payload %s", dev->netdev->name,
		    hex2str(dev->obuf, payload, 
			    min((int)(sizeof(dev->obuf)-1)/2,48),'\0'));
	}

	/* send stuff */
	netif_stop_queue(netdev);
	netdev->trans_start = jiffies;

	FILL_BULK_URB(dev->write_urb, dev->udev,
		      usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
		      tx_buffer, submit_len,
		      (usb_complete_t)at76c503_write_bulk_callback, dev);
	ret = submit_urb(dev->write_urb, GFP_ATOMIC);
	if(ret){
		stats->tx_errors++;
		err("%s: error in tx submit urb: %d", netdev->name, ret);
		if (ret == -EINVAL)
			err("-EINVAL: urb %p urb->hcpriv %p urb->complete %p",
			    dev->write_urb,
			    dev->write_urb ? dev->write_urb->hcpriv : (void *)-1,
			    dev->write_urb ? dev->write_urb->complete : (void *)-1);
		goto err;
	}

	stats->tx_bytes += skb->len;

	dev_kfree_skb(skb);
	return 0;

 err:
	return ret;
}


static
void at76c503_tx_timeout(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)(netdev->priv);

	if (!dev)
		return;
	warn("%s: tx timeout.", netdev->name);
	dev->write_urb->transfer_flags |= USB_ASYNC_UNLINK;
	usb_unlink_urb(dev->write_urb);
	dev->stats.tx_errors++;
}

static
int startup_device(struct at76c503 *dev)
{
	struct at76c503_card_config *ccfg = &dev->card_config;
	int ret;

	if (debug & DBG_PARAMS) {
		char ossid[IW_ESSID_MAX_SIZE+1];

		/* make dev->essid printable */
		assert(dev->essid_size <= IW_ESSID_MAX_SIZE);
		memcpy(ossid, dev->essid, dev->essid_size);
		ossid[dev->essid_size] = '\0';

		dbg_uc("%s param: ssid %s (%s) mode %s ch %d wep %s key %d keylen %d",
		       dev->netdev->name, ossid, 
		       hex2str(dev->obuf, dev->essid,
			       min((int)(sizeof(dev->obuf)-1)/2,
				   IW_ESSID_MAX_SIZE), '\0'),
		       dev->iw_mode == IW_MODE_ADHOC ? "adhoc" : "infra",
		       dev->channel,
		       dev->wep_enabled ? "enabled" : "disabled",
		       dev->wep_key_id, dev->wep_keys_len[dev->wep_key_id]);
		dbg_uc("%s param: preamble %s rts %d retry %d frag %d txrate %s auth_mode %d",
		       dev->netdev->name,		       
		       dev->preamble_type == PREAMBLE_TYPE_SHORT ? "short" : "long",
		       dev->rts_threshold, dev->short_retry_limit, dev->frag_threshold,
		       dev->txrate == TX_RATE_1MBIT ? "1MBit" :
		       dev->txrate == TX_RATE_2MBIT ? "2MBit" :
		       dev->txrate == TX_RATE_5_5MBIT ? "5.5MBit" :
		       dev->txrate == TX_RATE_11MBIT ? "11MBit" :
		       dev->txrate == TX_RATE_AUTO ? "auto" : "<invalid>",
		       dev->auth_mode);
		dbg_uc("%s param: pm_mode %d pm_period %d auth_mode %s "
		       "scan_times %d %d scan_mode %s",
		       dev->netdev->name,		       
		       dev->pm_mode, dev->pm_period_us,
		       dev->auth_mode == IEEE802_11_AUTH_ALG_OPEN_SYSTEM ?
		       "open" : "shared_secret",
		       dev->scan_min_time, dev->scan_max_time,
		       dev->scan_mode == SCAN_TYPE_ACTIVE ? "active" : "passive");
	}

	memset(ccfg, 0, sizeof(struct at76c503_card_config));
	ccfg->exclude_unencrypted = 1; /* jal: always exclude unencrypted 
					  if WEP is active */
	ccfg->promiscuous_mode = 0;
	ccfg->short_retry_limit = dev->short_retry_limit;
	//ccfg->long_retry_limit = dev->long_retry_limit;
	
	if (dev->wep_enabled &&
	    dev->wep_keys_len[dev->wep_key_id] > WEP_SMALL_KEY_LEN)
		ccfg->encryption_type = 2;
	else
		ccfg->encryption_type = 1;


	ccfg->rts_threshold = cpu_to_le16(dev->rts_threshold);
	ccfg->fragmentation_threshold = cpu_to_le16(dev->frag_threshold);

	memcpy(ccfg->basic_rate_set, hw_rates, 4);
	/* jal: really needed, we do a set_mib for autorate later ??? */
	ccfg->auto_rate_fallback = (dev->txrate == 4 ? 1 : 0);
	ccfg->channel = dev->channel;
	ccfg->privacy_invoked = dev->wep_enabled;
	memcpy(ccfg->current_ssid, dev->essid, IW_ESSID_MAX_SIZE);
	ccfg->ssid_len = dev->essid_size;

	ccfg->wep_default_key_id = dev->wep_key_id;
	memcpy(ccfg->wep_default_key_value, dev->wep_keys, 4 * WEP_KEY_SIZE);

	ccfg->short_preamble = dev->preamble_type;
	ccfg->beacon_period = cpu_to_le16(100);

	ret = set_card_command(dev->udev, CMD_STARTUP, (unsigned char *)&dev->card_config,
			       sizeof(struct at76c503_card_config));
	if(ret < 0){
		err("%s: set_card_command failed: %d", dev->netdev->name, ret);
		return ret;
	}

	wait_completion(dev, CMD_STARTUP);

	/* remove BSSID from previous run */
	memset(dev->bssid, 0, ETH_ALEN);
  
	if (set_radio(dev, 1) == 1)
		wait_completion(dev, CMD_RADIO);

	if ((ret=set_preamble(dev, dev->preamble_type)) < 0)
		return ret;
	if ((ret=set_frag(dev, dev->frag_threshold)) < 0)
		return ret;

	if ((ret=set_rts(dev, dev->rts_threshold)) < 0)
		return ret;
	
	if ((ret=set_autorate_fallback(dev, dev->txrate == 4 ? 1 : 0)) < 0)
		return ret;

	if ((ret=set_pm_mode(dev, dev->pm_mode)) < 0)
		return ret;
	
	return 0;
}

static
int at76c503_open(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)(netdev->priv);
	int ret = 0;

	dbg(DBG_PROC_ENTRY, "at76c503_open entry");

	if(down_interruptible(&dev->sem))
	   return -EINTR;

	ret = startup_device(dev);
	if (ret < 0)
		goto err;

	/* if netdev->dev_addr != dev->mac_addr we must
	   set the mac address in the device ! */
	if (memcmp(netdev->dev_addr, dev->mac_addr, ETH_ALEN)) {
		if (set_mac_address(dev,netdev->dev_addr) >= 0)
			dbg(DBG_PROGRESS, "%s: set new MAC addr %s",
			    netdev->name, mac2str(netdev->dev_addr));
	}

#if 0 //test only !!!
	dump_mib_mac_addr(dev);
#endif

	dev->nr_submit_rx_tries = NR_SUBMIT_RX_TRIES; /* init counter */

	ret = submit_rx_urb(dev);
	if(ret < 0){
		err("%s: open: submit_rx_urb failed: %d", netdev->name, ret);
		goto err;
	}

  	NEW_STATE(dev,SCANNING);
  	defer_kevent(dev,KEVENT_SCAN);
  	netif_carrier_off(dev->netdev); /* disable running netdev watchdog */
  	netif_stop_queue(dev->netdev); /* stop tx data packets */
   
	dev->open_count++;
	dbg(DBG_PROC_ENTRY, "at76c503_open end");
 err:
	up(&dev->sem);
	return ret < 0 ? ret : 0;
}

static
int at76c503_stop(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)(netdev->priv);
	unsigned long flags;

	dbg(DBG_DEVSTART, "%s: ENTER", __FUNCTION__);

	if (down_interruptible(&dev->sem))
		return -EINTR;

	netif_stop_queue(netdev);

  	NEW_STATE(dev,INIT);

	if (!(dev->flags & AT76C503A_UNPLUG)) {
		/* we are called by "ifconfig wlanX down", not because the
		   device isn't avail. anymore */	
		set_radio(dev, 0);

		/* we unlink the read urb, because the _open()
		   submits it again. _delete_device() takes care of the
		   read_urb otherwise. */
		usb_unlink_urb(dev->read_urb);
	}

	del_timer_sync(&dev->mgmt_timer);

	spin_lock_irqsave(&dev->mgmt_spinlock,flags);
	if (dev->next_mgmt_bulk) {
		kfree(dev->next_mgmt_bulk);
		dev->next_mgmt_bulk = NULL;
	}
	spin_unlock_irqrestore(&dev->mgmt_spinlock,flags);

	/* free the bss_list */
	free_bss_list(dev);

	assert(dev->open_count > 0);
	dev->open_count--;

	up(&dev->sem);

	dbg(DBG_DEVSTART, "%s: EXIT", __FUNCTION__);

	return 0;
}

static
struct net_device_stats *at76c503_get_stats(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)netdev->priv;

	return &dev->stats;
}

static
struct iw_statistics *at76c503_get_wireless_stats(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)netdev->priv;

	return &dev->wstats;
}

static
void at76c503_set_multicast(struct net_device *netdev)
{
	struct at76c503 *dev = (struct at76c503 *)netdev->priv;
	int promisc;

	promisc = ((netdev->flags & IFF_PROMISC) != 0);
	if(promisc != dev->promisc){
		/* grmbl. This gets called in interrupt. */
		dev->promisc = promisc;
		defer_kevent(dev, KEVENT_SET_PROMISC);
	}
}

/* we only store the new mac address in netdev struct,
   it got set when the netdev gets opened. */
static
int at76c503_set_mac_address(struct net_device *netdev, void *addr)
{
	struct sockaddr *mac = addr;
	memcpy(netdev->dev_addr, mac->sa_data, ETH_ALEN);
	return 1;
}

/* == PROC iwspy_update == 
  check if we spy on the sender address of buf and update statistics */
static
void iwspy_update(struct at76c503 *dev, struct at76c503_rx_buffer *buf)
{
#if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
	struct ieee802_11_hdr *hdr = (struct ieee802_11_hdr *)buf->packet;
	u16 lev_dbm = buf->rssi * 5 / 2;
	struct iw_quality wstats = {
		.qual = buf->link_quality,
		.level = (lev_dbm > 255) ? 255 : lev_dbm,
		.noise = buf->noise_level,
		.updated = 1,
	};
#if WIRELESS_EXT <= 15
	int i = 0;
#endif
	
	spin_lock_bh(&(dev->spy_spinlock));
	
#if WIRELESS_EXT > 15	
	if (dev->spy_data.spy_number > 0) {
		wireless_spy_update(dev->netdev, hdr->addr2, &wstats);
	}
#else
	for (i; i < dev->iwspy_nr; i++) {
		if (!memcmp(hdr->addr2, dev->iwspy_addr[i].sa_data, ETH_ALEN)) {
			memcpy(&(dev->iwspy_stats[i]), &wstats, sizeof(wstats));
			break;
		}
	}
#endif // #if WIRELESS_EXT > 15
	spin_unlock_bh(&(dev->spy_spinlock));
#endif // #if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
} /* iwspy_update */

static int ethtool_ioctl(struct at76c503 *dev, void *useraddr)
{
	u32 ethcmd;

	if (get_user(ethcmd, (u32 *)useraddr))
		return -EFAULT;

#if 0
	{
		dbg_uc("%s: %s: ethcmd=x%x buf: %s",
		       dev->netdev->name, __FUNCTION__, ethcmd,
		       hex2str(dev->obuf,useraddr,
			       min((int)(sizeof(dev->obuf)-1)/2,32), '\0'));
	}
#endif	

	switch (ethcmd) {
	case ETHTOOL_GDRVINFO:
	{
		struct ethtool_drvinfo *info = 
			kmalloc(sizeof(struct ethtool_drvinfo), GFP_KERNEL);
		if (!info)
			return -ENOMEM;

		info->cmd = ETHTOOL_GDRVINFO;
		strncpy(info->driver, "at76c503", 
			sizeof(info->driver)-1);
		
		strncpy(info->version, DRIVER_VERSION, sizeof(info->version));
		info->version[sizeof(info->version)-1] = '\0';

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 7))
		snprintf(info->bus_info, sizeof(info->bus_info)-1,
			 "usb%d:%d", dev->udev->bus->busnum,
			 dev->udev->devnum);

		snprintf(info->fw_version, sizeof(info->fw_version)-1,
			 "%d.%d.%d-%d",
			 dev->fw_version.major, dev->fw_version.minor,
			 dev->fw_version.patch, dev->fw_version.build);
#else
                /* Take the risk of a buffer overflow: no snprintf... */
		sprintf(info->bus_info, "usb%d:%d", dev->udev->bus->busnum,
                        dev->udev->devnum);

		sprintf(info->fw_version, "%d.%d.%d-%d",
                        dev->fw_version.major, dev->fw_version.minor,
                        dev->fw_version.patch, dev->fw_version.build);
#endif
		if (copy_to_user (useraddr, info, sizeof (*info))) {
			kfree(info);
			return -EFAULT;
		}

		kfree(info);
		return 0;
	}
	break;

	default:
		break;
	}

	return -EOPNOTSUPP;
}


/*******************************************************************************
 * structure that describes the private ioctls/iw handlers of this driver
 */
static const struct iw_priv_args at76c503_priv_args[] = {
	{ PRIV_IOCTL_SET_SHORT_PREAMBLE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
	  "short_preamble" }, // 0 - long, 1 -short
	
	{ PRIV_IOCTL_SET_DEBUG, 
	  // we must pass the new debug mask as a string,
	  // 'cause iwpriv cannot parse hex numbers
	  // starting with 0x :-( 
	  IW_PRIV_TYPE_CHAR | 10, 0,
	  "set_debug"}, // set debug value
	
	{ PRIV_IOCTL_SET_POWERSAVE_MODE, 
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
	  "powersave_mode"}, // 1 -  active, 2 - power save,
			     // 3 - smart power save
	{ PRIV_IOCTL_SET_SCAN_TIMES, 
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0,
	  "scan_times"}, // min_channel_time,
			 // max_channel_time
	{ PRIV_IOCTL_SET_SCAN_MODE, 
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
	  "scan_mode"}, // 0 - active, 1 - passive scan
};


/*******************************************************************************
 * at76c503 implementations of iw_handler functions:
 */
static
int at76c503_iw_handler_commit(struct net_device *netdev,
			       IW_REQUEST_INFO *info,
			       void *null,
			       char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	unsigned long flags;
	
	dbg(DBG_IOCTL, "%s %s: restarting the device", netdev->name,
		__FUNCTION__);
	
	// stop any pending tx bulk urb
	//TODO
	
	// jal: TODO: protect access to dev->istate by a spinlock
	// (ISR's on other processors may read/write it)
	if (dev->istate != INIT) {
		dev->istate = INIT;
		// stop pending management stuff
		del_timer_sync(&dev->mgmt_timer);

		spin_lock_irqsave(&dev->mgmt_spinlock,flags);
		if (dev->next_mgmt_bulk) {
			kfree(dev->next_mgmt_bulk);
			dev->next_mgmt_bulk = NULL;
		}
		spin_unlock_irqrestore(&dev->mgmt_spinlock,flags);

		netif_carrier_off(dev->netdev);
		netif_stop_queue(dev->netdev);
	}

	// do the restart after two seconds to catch
	// following ioctl's (from more params of iwconfig)
	// in _one_ restart
	mod_timer(&dev->restart_timer, jiffies+2*HZ);
	
	return 0;
}

static 
int at76c503_iw_handler_get_name(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 char *name,
				 char *extra)
{
	strcpy(name, "IEEE 802.11-DS");
	
	dbg(DBG_IOCTL, "%s: SIOCGIWNAME - name %s", netdev->name, name);
	
	return 0;
}

static 
int at76c503_iw_handler_set_freq(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_freq *freq,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int chan = -1;
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWFREQ - freq.m %d freq.e %d", netdev->name, 
		freq->m, freq->e);
	
	// modelled on orinoco.c
	if ((freq->e == 0) && (freq->m <= 1000))
	{
		// Setting by channel number
		chan = freq->m;
	}
	else
	{
		// Setting by frequency - search the table
		int mult = 1;
		int i;
		
		for (i = 0; i < (6 - freq->e); i++) {
			mult *= 10;
		}
		
		for (i = 0; i < NUM_CHANNELS; i++) {
			if (freq->m == (channel_frequency[i] * mult))
				chan = i + 1;
		}
	}
	
	if (chan < 1 || !dev->domain ) {
		// non-positive channels are invalid
		// we need a domain info to set the channel
		// either that or an invalid frequency was 
		// provided by the user
		ret =  -EINVAL;
	} else {
		if (!(dev->domain->channel_map & (1 << (chan-1)))) {
			info("%s: channel %d not allowed for domain %s",
			     dev->netdev->name, chan, dev->domain->name);
			ret = -EINVAL;
		}
	}
	
	if (ret == -EIWCOMMIT) {
		dev->channel = chan;
		dbg(DBG_IOCTL, "%s: SIOCSIWFREQ - ch %d", netdev->name, chan);
	}
	
	return ret;
}

static 
int at76c503_iw_handler_get_freq(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_freq *freq,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	freq->m = dev->channel;
	freq->e = 0;
	
	if (dev->channel)
	{
		dbg(DBG_IOCTL, "%s: SIOCGIWFREQ - freq %ld x 10e%d", netdev->name, 
			channel_frequency[dev->channel - 1], 6);
	}
	dbg(DBG_IOCTL, "%s: SIOCGIWFREQ - ch %d", netdev->name, dev->channel);
	
	return 0;
}

static 
int at76c503_iw_handler_set_mode(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 __u32 *mode,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWMODE - %d", netdev->name, *mode);
	
	if ((*mode != IW_MODE_ADHOC) &&
	    (*mode != IW_MODE_INFRA)) {
		ret = -EINVAL;
	} else {
		dev->iw_mode = *mode;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_get_mode(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 __u32 *mode,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	*mode = dev->iw_mode;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWMODE - %d", netdev->name, *mode);
	
	return 0;
}

static 
int at76c503_iw_handler_get_range(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_point *data,
				  char *extra)
{
	// inspired by atmel.c
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	struct iw_range *range = (struct iw_range*)extra;
	int i;
	
	data->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));
	
	
	//TODO: range->throughput = xxxxxx;
	
	range->min_nwid = 0x0000;
	range->max_nwid = 0x0000;
	
	
	// this driver doesn't maintain sensitivity information
	range->sensitivity = 0;
	
	
	range->max_qual.qual = 
		((dev->board_type == BOARDTYPE_503_INTERSIL_3861) ||
		 (dev->board_type == BOARDTYPE_503_INTERSIL_3863)) ? 100 : 0;
	range->max_qual.level = 100;
	range->max_qual.noise = 0;
	
	range->avg_qual.qual = 
		((dev->board_type == BOARDTYPE_503_INTERSIL_3861) ||
		 (dev->board_type == BOARDTYPE_503_INTERSIL_3863)) ? 30 : 0;
	range->avg_qual.level = 30;
	range->avg_qual.noise = 0;
	
	
	range->bitrate[0] = 1000000;
	range->bitrate[1] = 2000000;
	range->bitrate[2] = 5500000;
	range->bitrate[3] = 11000000;
	range->num_bitrates = 4;
	
	
	range->min_rts = 0;
	range->max_rts = MAX_RTS_THRESHOLD;
	
	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;
	
	
	//range->min_pmp = 0;
	//range->max_pmp = 5000000;
	//range->min_pmt = 0;
	//range->max_pnt = 0;
	//range->pmp_flags = IW_POWER_PERIOD;
	//range->pmt_flags = 0;
	//range->pm_capa = IW_POWER_PERIOD;
	//TODO: find out what values we can use to describe PM capabilities
	range->pmp_flags = IW_POWER_ON;
	range->pmt_flags = IW_POWER_ON;
	range->pm_capa = 0;
	
	
	range->encoding_size[0] = WEP_SMALL_KEY_LEN;
	range->encoding_size[1] = WEP_LARGE_KEY_LEN;
	range->num_encoding_sizes = 2;
	range->max_encoding_tokens = NR_WEP_KEYS;
	//TODO: do we need this?  what is a valid value if we don't support?
	//range->encoding_login_index = -1;
	
	/* both WL-240U and Linksys WUSB11 v2.6 specify 15 dBm as output power
	   - take this for all (ignore antenna gains) */
	range->txpower[0] = 15;
	range->num_txpower = 1;
	range->txpower_capa = IW_TXPOW_DBM;

	// at time of writing (22/Feb/2004), version we intend to support 
	// is v16, 
	range->we_version_source = WIRELESS_EXT_SUPPORTED;
	range->we_version_compiled = WIRELESS_EXT;
	
	// same as the values used in atmel.c
	range->retry_capa = IW_RETRY_LIMIT ;
	range->retry_flags = IW_RETRY_LIMIT;
	range->r_time_flags = 0;
	range->min_retry = 1;
	range->max_retry = 255;
	
	
	range->num_channels = NUM_CHANNELS;
	range->num_frequency = 0;
	
	for (i = 0; 
		i < 32; //number of bits in reg_domain.channel_map 
		i++)
	{
		// test if channel map bit is raised
		if (dev->domain->channel_map & (0x1 << i))
		{
			range->num_frequency += 1;
			
			range->freq[i].i = i + 1;
			range->freq[i].m = channel_frequency[i] * 100000;
			range->freq[i].e = 1; // channel frequency*100000 * 10^1
		}
	}
	
	dbg(DBG_IOCTL, "%s: SIOCGIWRANGE", netdev->name);
	
	return 0;
}

static 
int at76c503_iw_handler_get_priv(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_point *data,
				 char *extra)
{
	dbg(DBG_IOCTL, "%s: SIOCGIWPRIV", netdev->name);
	
	memcpy(extra, at76c503_priv_args, sizeof(at76c503_priv_args));
	data->length = sizeof(at76c503_priv_args) / 
			sizeof(at76c503_priv_args[0]);
	
	return 0;
}


#if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
static 
int at76c503_iw_handler_set_spy(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct iw_point *data,
				char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
#if WIRELESS_EXT <= 15
	int i = 0;
#endif
	int ret = 0;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWSPY - number of addresses %d",
		netdev->name, data->length);
	
	spin_lock_bh(&(dev->spy_spinlock));
#if WIRELESS_EXT > 15
	ret = iw_handler_set_spy(dev->netdev, info, (union iwreq_data *)data, 
		extra);
#else
	if (&data == NULL) {
		return -EFAULT;
	}
	
	if (data->length > IW_MAX_SPY)
	{
		return -E2BIG;
	}
	
	dev->iwspy_nr = data->length;
	
	if (dev->iwspy_nr > 0) {
		memcpy(dev->iwspy_addr, extra,
			sizeof(struct sockaddr) * dev->iwspy_nr);
		
		memset(dev->iwspy_stats, 0, sizeof(dev->iwspy_stats));
	}
	
	// Time to show what we have done...
	dbg(DBG_IOCTL, "%s: New spy list:", netdev->name);
	for (i = 0; i < dev->iwspy_nr; i++) {
		dbg(DBG_IOCTL, "%s: SIOCSIWSPY - entry %d: %s", netdev->name, 
			i + 1, mac2str(dev->iwspy_addr[i].sa_data));
	}
#endif // #if WIRELESS_EXT > 15
	spin_unlock_bh(&(dev->spy_spinlock));
	
	return ret;
}

static 
int at76c503_iw_handler_get_spy(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct iw_point *data,
				char *extra)
{

	struct at76c503 *dev = (struct at76c503*)netdev->priv;
#if WIRELESS_EXT <= 15
	int i = 0;
#endif
	int ret = 0;
	
	spin_lock_bh(&(dev->spy_spinlock));
#if WIRELESS_EXT > 15
	ret = iw_handler_get_spy(dev->netdev, info, 
		(union iwreq_data *)data, extra);
#else
	data->length = dev->iwspy_nr;
	
	if ((data->length > 0) && extra) {
		// Push stuff to user space
		memcpy(extra, dev->iwspy_addr,
			sizeof(struct sockaddr) * data->length);
		
		memcpy(extra + sizeof(struct sockaddr) * data->length,
			dev->iwspy_stats,
			sizeof(struct iw_quality) * data->length);
		
		for (i = 0; i < dev->iwspy_nr; i++) {
			dev->iwspy_stats[i].updated = 0;
		}
	}
#endif // #if WIRELESS_EXT > 15
	spin_unlock_bh(&(dev->spy_spinlock));
	
	dbg(DBG_IOCTL, "%s: SIOCGIWSPY - number of addresses %d",
		netdev->name, data->length);
	
	return ret;
}
#endif // #if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)

#if WIRELESS_EXT > 15
static 
int at76c503_iw_handler_set_thrspy(struct net_device *netdev,
				   IW_REQUEST_INFO *info,
				   struct iw_point *data,
				   char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWTHRSPY - number of addresses %d)",
		netdev->name, data->length);
	
	spin_lock_bh(&(dev->spy_spinlock));
	ret = iw_handler_set_thrspy(netdev, info, (union iwreq_data *)data, 
		extra);
	spin_unlock_bh(&(dev->spy_spinlock));
	
	return ret;
}

static 
int at76c503_iw_handler_get_thrspy(struct net_device *netdev,
				   IW_REQUEST_INFO *info,
				   struct iw_point *data,
				   char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret;
	
	spin_lock_bh(&(dev->spy_spinlock));
	ret = iw_handler_get_thrspy(netdev, info, (union iwreq_data *)data, 
		extra);
	spin_unlock_bh(&(dev->spy_spinlock));
	
	dbg(DBG_IOCTL, "%s: SIOCGIWTHRSPY - number of addresses %d)",
		netdev->name, data->length);
	
	return ret;
}
#endif //#if WIRELESS_EXT > 15

static 
int at76c503_iw_handler_set_wap(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct sockaddr *ap_addr,
				char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWAP - wap/bssid %s", netdev->name, 
		mac2str(ap_addr->sa_data));
	
	// if the incomming address == ff:ff:ff:ff:ff:ff, the user has 
	// chosen any or auto AP preference
	if (!memcmp(ap_addr->sa_data, bc_addr, ETH_ALEN)) {
		dev->wanted_bssid_valid = 0;
	} else {
		// user wants to set a preferred AP address
		dev->wanted_bssid_valid = 1;
		memcpy(dev->wanted_bssid, ap_addr->sa_data, ETH_ALEN);
	}
	
	return -EIWCOMMIT;
}

static
int at76c503_iw_handler_get_wap(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct sockaddr *ap_addr,
				char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	ap_addr->sa_family = ARPHRD_ETHER;
	memcpy(ap_addr->sa_data, dev->bssid, ETH_ALEN);
	
	dbg(DBG_IOCTL, "%s: SIOCGIWAP - wap/bssid %s", netdev->name, 
		mac2str(ap_addr->sa_data));
	
	return 0;
}

/*static 
int at76c503_iw_handler_get_waplist(struct net_device *netdev,
				    IW_REQUEST_INFO *info,
				    struct iw_point *data,
				    char *extra)
{
	return -EOPNOTSUPP;
}*/

#if WIRELESS_EXT > 13
static 
int at76c503_iw_handler_set_scan(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_param *param,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = 0;
	unsigned long flags;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWSCAN", netdev->name);
	
	// stop pending management stuff
	del_timer_sync(&(dev->mgmt_timer));
	
	spin_lock_irqsave(&(dev->mgmt_spinlock), flags);
	if (dev->next_mgmt_bulk) {
		kfree(dev->next_mgmt_bulk);
		dev->next_mgmt_bulk = NULL;
	}
	spin_unlock_irqrestore(&(dev->mgmt_spinlock), flags);
	
	if (netif_running(dev->netdev)) {
		// pause network activity
		netif_carrier_off(dev->netdev);
		netif_stop_queue(dev->netdev);
	}
	
	// change to scanning state
	NEW_STATE(dev, SCANNING);
	
	ret = handle_scan(dev);
	
	if (!ret) {
		NEW_STATE(dev,JOINING);
		assert(dev->curr_bss == NULL);
		defer_kevent(dev, KEVENT_JOIN);
	}
	
	return (ret < 0) ? ret : 0;
}

static 
int at76c503_iw_handler_get_scan(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_point *data,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	unsigned long flags;
	struct list_head *lptr, *nptr;
	struct bss_info *curr_bss;
	struct iw_event *iwe = kmalloc(sizeof(struct iw_event), GFP_KERNEL);
	char *curr_val, *curr_pos = extra;
	int i;

	dbg(DBG_IOCTL, "%s: SIOCGIWSCAN", netdev->name);

	if (!iwe)
		return -ENOMEM;

	spin_lock_irqsave(&(dev->bss_list_spinlock), flags);
	
	list_for_each_safe(lptr, nptr, &(dev->bss_list)) {
		curr_bss = list_entry(lptr, struct bss_info, list);
		
		iwe->cmd = SIOCGIWAP;
		iwe->u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe->u.ap_addr.sa_data, curr_bss->bssid, 6);
		curr_pos = iwe_stream_add_event(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, IW_EV_ADDR_LEN);
		
		iwe->u.data.length = curr_bss->ssid_len;
		iwe->cmd = SIOCGIWESSID;
		// 1: SSID on , 0: SSID off/any
		iwe->u.data.flags = (dev->essid_size > 0) ? 1 : 0;
		
		curr_pos = iwe_stream_add_point(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, curr_bss->ssid);
		
		iwe->cmd = SIOCGIWMODE;
		iwe->u.mode = (curr_bss->capa & IEEE802_11_CAPA_IBSS) ? 
			IW_MODE_ADHOC :
			(curr_bss->capa & IEEE802_11_CAPA_ESS) ? 
			IW_MODE_MASTER :
			IW_MODE_AUTO;
			// IW_MODE_AUTO = 0 which I thought is 
			// the most logical value to return in this case
		curr_pos = iwe_stream_add_event(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, IW_EV_UINT_LEN);
		
		iwe->cmd = SIOCGIWFREQ;
		iwe->u.freq.m = curr_bss->channel;
		iwe->u.freq.e = 0;
		curr_pos = iwe_stream_add_event(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, IW_EV_FREQ_LEN);
		
		iwe->cmd = SIOCGIWENCODE;
		if (curr_bss->capa & IEEE802_11_CAPA_PRIVACY) {
			iwe->u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		} else {
			iwe->u.data.flags = IW_ENCODE_DISABLED;
		}
		iwe->u.data.length = 0;
		curr_pos = iwe_stream_add_point(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, NULL);

		/* Add quality statistics */
		iwe->cmd = IWEVQUAL;
		iwe->u.qual.level = curr_bss->rssi; /* do some calibration here ? */
		iwe->u.qual.noise = 0;
		iwe->u.qual.qual = curr_bss->link_qual; /* good old Intersil radios report this, too */
		/* Add new value to event */
		curr_pos = iwe_stream_add_event(curr_pos, 
			extra + IW_SCAN_MAX_DATA, iwe, IW_EV_QUAL_LEN);

		/* Rate : stuffing multiple values in a single event require a bit
		 * more of magic - Jean II */
		curr_val = curr_pos + IW_EV_LCP_LEN;

		iwe->cmd = SIOCGIWRATE;
		/* Those two flags are ignored... */
		iwe->u.bitrate.fixed = iwe->u.bitrate.disabled = 0;
		/* Max 8 values */
		for(i=0; i < curr_bss->rates_len; i++) {
			/* Bit rate given in 500 kb/s units (+ 0x80) */
			iwe->u.bitrate.value = 
				((curr_bss->rates[i] & 0x7f) * 500000);
			/* Add new value to event */
			curr_val = iwe_stream_add_value(curr_pos, curr_val, 
							extra + IW_SCAN_MAX_DATA,
							iwe, IW_EV_PARAM_LEN);
		}

		/* Check if we added any event */
		if ((curr_val - curr_pos) > IW_EV_LCP_LEN)
			curr_pos = curr_val;


		// more information may be sent back using IWECUSTOM

	}
	
	spin_unlock_irqrestore(&(dev->bss_list_spinlock), flags);
	
	data->length = (curr_pos - extra);
	data->flags = 0;
	
	kfree(iwe);
	return 0;
}
#endif // #if WIRELESS_EXT > 13


static 
int at76c503_iw_handler_set_essid(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_point *data,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWESSID - %s", netdev->name, extra);
	
	if (data->flags)
	{
		memcpy(dev->essid, extra, data->length);
		// iwconfig gives len including 0 byte -
		// 3 hours debugging... grrrr (oku)
		dev->essid_size = data->length - 1;
	}
	else
	{
		dev->essid_size = 0;
	}
	
	return -EIWCOMMIT;
}

static 
int at76c503_iw_handler_get_essid(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_point *data,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	if (dev->essid_size) {
		// not the ANY ssid in dev->essid
		data->flags = 1;
		data->length = dev->essid_size;
		memcpy(extra, dev->essid, data->length);
		extra[data->length] = '\0';
		data->length += 1;
	} else {
		// the ANY ssid was specified
		if (dev->istate == CONNECTED &&
		    dev->curr_bss != NULL) {
			// report the SSID we have found
			data->flags = 1;
			data->length = dev->curr_bss->ssid_len;
			memcpy(extra, dev->curr_bss->ssid, data->length);
			extra[dev->curr_bss->ssid_len] = '\0';
			data->length += 1;
		} else {
			// report ANY back
			data->flags=0;
			data->length=0;
		}
	}
	
	dbg(DBG_IOCTL, "%s: SIOCGIWESSID - %s", netdev->name, extra);
	
	return 0;
}

static
int at76c503_iw_handler_set_nickname(struct net_device *netdev,
				     IW_REQUEST_INFO *info,
				     struct iw_point *data,
				     char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWNICKN - %s", netdev->name, extra);
	
	// iwconfig gives length including 0 byte like in the case of essid
	memcpy(dev->nickn, extra, data->length);
	
	return 0;
}

static
int at76c503_iw_handler_get_nickname(struct net_device *netdev,
				     IW_REQUEST_INFO *info,
				     struct iw_point *data,
				     char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	data->length = strlen(dev->nickn);
	memcpy(extra, dev->nickn, data->length);
	extra[data->length] = '\0';
	data->length += 1;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWNICKN - %s", netdev->name, extra);
	
	return 0;
}

static 
int at76c503_iw_handler_set_rate(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_param *bitrate,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWRATE - %d", netdev->name, 
		bitrate->value);
	
	switch (bitrate->value)
	{
		case -1: 	dev->txrate = TX_RATE_AUTO; break; // auto rate 
		case 1000000: 	dev->txrate = TX_RATE_1MBIT; break;
		case 2000000: 	dev->txrate = TX_RATE_2MBIT; break;
		case 5500000: 	dev->txrate = TX_RATE_5_5MBIT; break;
		case 11000000: 	dev->txrate = TX_RATE_11MBIT; break;
		default:	ret = -EINVAL;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_get_rate(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_param *bitrate,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = 0;
	
	switch (dev->txrate)
	{
		// return max rate if RATE_AUTO
		case TX_RATE_AUTO: 	bitrate->value = 11000000; break; 
		case TX_RATE_1MBIT: 	bitrate->value = 1000000; break;
		case TX_RATE_2MBIT: 	bitrate->value = 2000000; break;
		case TX_RATE_5_5MBIT: 	bitrate->value = 5500000; break;
		case TX_RATE_11MBIT: 	bitrate->value = 11000000; break;
		default:		ret = -EINVAL;
	}
	
	bitrate->fixed = (dev->txrate != TX_RATE_AUTO);
	bitrate->disabled = 0;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWRATE - %d", netdev->name, 
		bitrate->value);
	
	return ret;
}

static 
int at76c503_iw_handler_set_rts(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct iw_param *rts,
				char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = -EIWCOMMIT;
	int rthr = rts->value;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWRTS - value %d disabled %s",
		netdev->name, rts->value, 
		(rts->disabled) ? "true" : "false");
	
	if (rts->disabled)
		rthr = MAX_RTS_THRESHOLD;
	
	if ((rthr < 0) || (rthr > MAX_RTS_THRESHOLD)) {
		ret = -EINVAL;
	} else {
		dev->rts_threshold = rthr;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_get_rts(struct net_device *netdev,
				IW_REQUEST_INFO *info,
				struct iw_param *rts,
				char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	rts->value = dev->rts_threshold;
	rts->disabled = (rts->value >= MAX_RTS_THRESHOLD);
	rts->fixed = 1;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWRTS - value %d disabled %s",
		netdev->name, rts->value, 
		(rts->disabled) ? "true" : "false");
	
	return 0;
}

static 
int at76c503_iw_handler_set_frag(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_param *frag,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = -EIWCOMMIT;
	int fthr = frag->value;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWFRAG - value %d, disabled %s",
		netdev->name, frag->value, 
		(frag->disabled) ? "true" : "false");
	
	if(frag->disabled)
		fthr = MAX_FRAG_THRESHOLD;
	
	if ((fthr < MIN_FRAG_THRESHOLD) || (fthr > MAX_FRAG_THRESHOLD)) {
		ret = -EINVAL;
	} else {
		dev->frag_threshold = fthr & ~0x1; // get an even value
	}
	
	return ret;
}

static 
int at76c503_iw_handler_get_frag(struct net_device *netdev,
				 IW_REQUEST_INFO *info,
				 struct iw_param *frag,
				 char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	frag->value = dev->frag_threshold;
	frag->disabled = (frag->value >= MAX_FRAG_THRESHOLD);
	frag->fixed = 1;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWFRAG - value %d, disabled %s",
		netdev->name, frag->value, 
		(frag->disabled) ? "true" : "false");
	
	return 0;
}

static
int at76c503_iw_handler_get_txpow(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_param *power,
				  char *extra)
{
	power->value = 15;
	power->fixed = 1;	/* No power control */
	power->disabled = 0;
	power->flags = IW_TXPOW_DBM;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWTXPOW - txpow %d dBm", netdev->name, 
		power->value);
	
	return 0;
}

/* jal: short retry is handled by the firmware (at least 0.90.x),
   while long retry is not (?) */
static 
int at76c503_iw_handler_set_retry(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_param *retry,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWRETRY disabled %d  flags x%x val %d",
	    netdev->name, retry->disabled, retry->flags, retry->value);
	
	if(!retry->disabled && (retry->flags & IW_RETRY_LIMIT)) {
		if ((retry->flags & IW_RETRY_MIN) || 
		    !(retry->flags & IW_RETRY_MAX)) {
			dev->short_retry_limit = retry->value;
		} else
			ret = -EINVAL;
	} else {
		ret = -EINVAL;
	}
	
	return ret;
}

// adapted (ripped) from atmel.c
static 
int at76c503_iw_handler_get_retry(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_param *retry,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWRETRY", netdev->name);
	
	retry->disabled = 0;      // Can't be disabled
	
	// Note : by default, display the min retry number
	//if((retry->flags & IW_RETRY_MAX)) {
	//	retry->flags = IW_RETRY_LIMIT | IW_RETRY_MAX;
	//	retry->value = dev->long_retry_limit;
	//} else {
		retry->flags = IW_RETRY_LIMIT;
		retry->value = dev->short_retry_limit;
		
		//if(dev->long_retry_limit != dev->short_retry_limit) {
		//	dev->retry.flags |= IW_RETRY_MIN;
		//}
	//}

	return 0;
}

static 
int at76c503_iw_handler_set_encode(struct net_device *netdev,
				   IW_REQUEST_INFO *info,
				   struct iw_point *encoding,
				   char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int index = (encoding->flags & IW_ENCODE_INDEX) - 1;
	int len = encoding->length;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWENCODE - enc.flags %08x "
		"pointer %p len %d", netdev->name, encoding->flags, 
		encoding->pointer, encoding->length);
	dbg(DBG_IOCTL, "%s: SIOCSIWENCODE - old wepstate: enabled %s key_id %d "
	       "auth_mode %s",
	       netdev->name, (dev->wep_enabled) ? "true" : "false", 
	       dev->wep_key_id, 
	       (dev->auth_mode == IEEE802_11_AUTH_ALG_SHARED_SECRET) ? 
			"restricted" : "open");
	
	// take the old default key if index is invalid
	if ((index < 0) || (index >= NR_WEP_KEYS))
		index = dev->wep_key_id;
	
	if (len > 0)
	{
		if (len > WEP_LARGE_KEY_LEN)
			len = WEP_LARGE_KEY_LEN;
		
		memset(dev->wep_keys[index], 0, WEP_KEY_SIZE);
		memcpy(dev->wep_keys[index], extra, len);
		dev->wep_keys_len[index] = (len <= WEP_SMALL_KEY_LEN) ?
			WEP_SMALL_KEY_LEN : WEP_LARGE_KEY_LEN;
		dev->wep_enabled = 1;
	}
	
	dev->wep_key_id = index;
	dev->wep_enabled = ((encoding->flags & IW_ENCODE_DISABLED) == 0);
	
	if (encoding->flags & IW_ENCODE_RESTRICTED)
		dev->auth_mode = IEEE802_11_AUTH_ALG_SHARED_SECRET;
	if (encoding->flags & IW_ENCODE_OPEN)
		dev->auth_mode = IEEE802_11_AUTH_ALG_OPEN_SYSTEM;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWENCODE - new wepstate: enabled %s key_id %d "
		"key_len %d auth_mode %s",
		netdev->name, (dev->wep_enabled) ? "true" : "false", 
		dev->wep_key_id + 1, dev->wep_keys_len[dev->wep_key_id],
		(dev->auth_mode == IEEE802_11_AUTH_ALG_SHARED_SECRET) ? 
			"restricted" : "open");
	
	return -EIWCOMMIT;
}

static 
int at76c503_iw_handler_get_encode(struct net_device *netdev,
				   IW_REQUEST_INFO *info,
				   struct iw_point *encoding,
				   char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int index = (encoding->flags & IW_ENCODE_INDEX) - 1;
	
	if ((index < 0) || (index >= NR_WEP_KEYS))
		index = dev->wep_key_id;
	
	encoding->flags = 
		(dev->auth_mode == IEEE802_11_AUTH_ALG_SHARED_SECRET) ? 
		  IW_ENCODE_RESTRICTED : IW_ENCODE_OPEN;
	
	if (!dev->wep_enabled)
		encoding->flags |= IW_ENCODE_DISABLED;
	
	if (encoding->pointer)
	{
		encoding->length = dev->wep_keys_len[index];
		
		memcpy(extra, dev->wep_keys[index], dev->wep_keys_len[index]);
		
		encoding->flags |= (index + 1);
	}
	
	dbg(DBG_IOCTL, "%s: SIOCGIWENCODE - enc.flags %08x "
		"pointer %p len %d", netdev->name, encoding->flags, 
		encoding->pointer, encoding->length);
	dbg(DBG_IOCTL, "%s: SIOCGIWENCODE - wepstate: enabled %s key_id %d "
		"key_len %d auth_mode %s",
		netdev->name, (dev->wep_enabled) ? "true" : "false", 
		dev->wep_key_id + 1, dev->wep_keys_len[dev->wep_key_id],
		(dev->auth_mode == IEEE802_11_AUTH_ALG_SHARED_SECRET) ? 
			"restricted" : "open");
	
	return 0;
}

static 
int at76c503_iw_handler_set_power(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_param *power,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	dbg(DBG_IOCTL, "%s: SIOCSIWPOWER - disabled %s flags x%x value x%x", 
		netdev->name, (power->disabled) ? "true" : "false", 
		power->flags, power->value);
	
	if (power->disabled)
	{
		dev->pm_mode = PM_ACTIVE;
	}
	else
	{
		// we set the listen_interval based on the period given
		// no idea how to handle the timeout of iwconfig ???
		if (power->flags & IW_POWER_PERIOD)
		{
			dev->pm_period_us = power->value;
		}
		
		dev->pm_mode = PM_SAVE; // use iw_priv to select SMART_SAVE
	}
	
	return -EIWCOMMIT;
}

static 
int at76c503_iw_handler_get_power(struct net_device *netdev,
				  IW_REQUEST_INFO *info,
				  struct iw_param *power,
				  char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	
	power->disabled = dev->pm_mode == PM_ACTIVE;
	
	if ((power->flags & IW_POWER_TYPE) == IW_POWER_TIMEOUT)
	{
		power->flags = IW_POWER_TIMEOUT;
		power->value = 0;
	}
	else
	{
		unsigned long flags;
		u16 beacon_int; // of the current bss
		
		power->flags = IW_POWER_PERIOD;

		spin_lock_irqsave(&dev->bss_list_spinlock, flags);
		beacon_int = dev->curr_bss != NULL ?
			dev->curr_bss->beacon_interval : 0;
		spin_unlock_irqrestore(&dev->bss_list_spinlock, flags);
		
		if (beacon_int != 0)
		{
			power->value =
				(beacon_int * dev->pm_period_beacon) << 10;
		}
		else
		{
			power->value = dev->pm_period_us;
		}
	}
	
	power->flags |= IW_POWER_ALL_R;
	
	dbg(DBG_IOCTL, "%s: SIOCGIWPOWER - disabled %s flags x%x value x%x", 
		netdev->name, (power->disabled) ? "true" : "false", 
		power->flags, power->value);
	
	return 0;
}


/*******************************************************************************
 * Private IOCTLS
 */
static 
int at76c503_iw_handler_PRIV_IOCTL_SET_SHORT_PREAMBLE
	(struct net_device *netdev, IW_REQUEST_INFO *info, 
	 char *name, char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int val = *((int *)name);
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: PRIV_IOCTL_SET_SHORT_PREAMBLE, %d",
		netdev->name, val);
	
	if (val < 0 || val > 2) {
		//allow value of 2 - in the win98 driver it stands
		//for "auto preamble" ...?
		ret = -EINVAL;
	} else {
		dev->preamble_type = val;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_PRIV_IOCTL_SET_DEBUG
	(struct net_device *netdev, IW_REQUEST_INFO *info, 
	 struct iw_point *data, char *extra)
{
	char *ptr;
	u32 val;
	
	if (data->length > 0) {
		val = simple_strtol(extra, &ptr, 0);
		
		if (ptr == extra) {
			val = DBG_DEFAULTS;
		}
		
		dbg_uc("%s: PRIV_IOCTL_SET_DEBUG input %d: %s -> x%x",
		       netdev->name, data->length, extra, val);
	} else {
		val = DBG_DEFAULTS;
	}
	
	dbg_uc("%s: PRIV_IOCTL_SET_DEBUG, old 0x%x  new 0x%x",
			netdev->name, debug, val);
	
	/* jal: some more output to pin down lockups */
	dbg_uc("%s: netif running %d queue_stopped %d carrier_ok %d",
			netdev->name, 
			netif_running(netdev),
			netif_queue_stopped(netdev),
			netif_carrier_ok(netdev));
	
	debug = val;
	
	return 0;
}

static 
int at76c503_iw_handler_PRIV_IOCTL_SET_POWERSAVE_MODE
	(struct net_device *netdev, IW_REQUEST_INFO *info, 
	 char *name, char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int val = *((int *)name);
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: PRIV_IOCTL_SET_POWERSAVE_MODE, %d (%s)",
		netdev->name, val,
		val == PM_ACTIVE ? "active" : val == PM_SAVE ? "save" :
		val == PM_SMART_SAVE ? "smart save" : "<invalid>");
	if (val < PM_ACTIVE || val > PM_SMART_SAVE) {
		ret = -EINVAL;
	} else {
		dev->pm_mode = val;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_TIMES
	(struct net_device *netdev, IW_REQUEST_INFO *info, 
	 char *name, char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int mint = *((int *)name);
	int maxt = *((int *)name + 1);
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: PRIV_IOCTL_SET_SCAN_TIMES - min %d max %d",
		netdev->name, mint, maxt);
	if (mint <= 0 || maxt <= 0 || mint > maxt) {
		ret = -EINVAL;
	} else {
		dev->scan_min_time = mint;
		dev->scan_max_time = maxt;
	}
	
	return ret;
}

static 
int at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_MODE
	(struct net_device *netdev, IW_REQUEST_INFO *info, 
	 char *name, char *extra)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	int val = *((int *)name);
	int ret = -EIWCOMMIT;
	
	dbg(DBG_IOCTL, "%s: PRIV_IOCTL_SET_SCAN_MODE - mode %s",
		netdev->name, (val = SCAN_TYPE_ACTIVE) ? "active" : 
		(val = SCAN_TYPE_PASSIVE) ? "passive" : "<invalid>");
	
	if (val != SCAN_TYPE_ACTIVE && val != SCAN_TYPE_PASSIVE) {
		ret = -EINVAL;
	} else {
		dev->scan_mode = val;
	}
	
	return ret;
}



#if WIRELESS_EXT > 12
/*******************************************************************************
 * structure that advertises the iw handlers of this driver
 */
static const iw_handler	at76c503_handlers[] =
{
	(iw_handler) at76c503_iw_handler_commit,	// SIOCSIWCOMMIT
	(iw_handler) at76c503_iw_handler_get_name,	// SIOCGIWNAME
	(iw_handler) NULL,				// SIOCSIWNWID
	(iw_handler) NULL,				// SIOCGIWNWID
	(iw_handler) at76c503_iw_handler_set_freq,	// SIOCSIWFREQ
	(iw_handler) at76c503_iw_handler_get_freq,	// SIOCGIWFREQ
	(iw_handler) at76c503_iw_handler_set_mode,	// SIOCSIWMODE
	(iw_handler) at76c503_iw_handler_get_mode,	// SIOCGIWMODE
	(iw_handler) NULL,				// SIOCSIWSENS
	(iw_handler) NULL,				// SIOCGIWSENS
	(iw_handler) NULL,				// -- hole --
	(iw_handler) at76c503_iw_handler_get_range,	// SIOCGIWRANGE
	(iw_handler) NULL,				// -- hole --
	(iw_handler) NULL,				// SIOCGIWPRIV
	(iw_handler) NULL,				// -- hole --
	(iw_handler) NULL,				// -- hole --
	
#if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
	(iw_handler) at76c503_iw_handler_set_spy,	// SIOCSIWSPY
	(iw_handler) at76c503_iw_handler_get_spy,	// SIOCGIWSPY
#else
	(iw_handler) NULL,				// SIOCSIWSPY
	(iw_handler) NULL,				// SIOCGIWSPY
#endif

#if WIRELESS_EXT > 15
	(iw_handler) at76c503_iw_handler_set_thrspy,	// SIOCSIWTHRSPY
	(iw_handler) at76c503_iw_handler_get_thrspy,	// SIOCGIWTHRSPY
#else
	(iw_handler) NULL,				// SIOCSIWTHRSPY
	(iw_handler) NULL,				// SIOCGIWTHRSPY
#endif

	(iw_handler) at76c503_iw_handler_set_wap,	// SIOCSIWAP
	(iw_handler) at76c503_iw_handler_get_wap,	// SIOCGIWAP
	(iw_handler) NULL,				// -- hole --
	(iw_handler) NULL,				// SIOCGIWAPLIST

#if WIRELESS_EXT > 13
	(iw_handler) at76c503_iw_handler_set_scan,	// SIOCSIWSCAN
	(iw_handler) at76c503_iw_handler_get_scan,	// SIOCGIWSCAN
#else
	(iw_handler) NULL,				// SIOCSIWSCAN
	(iw_handler) NULL,				// SIOCGIWSCAN
#endif
	(iw_handler) at76c503_iw_handler_set_essid,	// SIOCSIWESSID
	(iw_handler) at76c503_iw_handler_get_essid,	// SIOCGIWESSID
	(iw_handler) at76c503_iw_handler_set_nickname,	// SIOCSIWNICKN
	(iw_handler) at76c503_iw_handler_get_nickname,	// SIOCGIWNICKN
	(iw_handler) NULL,				// -- hole --
	(iw_handler) NULL,				// -- hole --
	(iw_handler) at76c503_iw_handler_set_rate,	// SIOCSIWRATE
	(iw_handler) at76c503_iw_handler_get_rate,	// SIOCGIWRATE
	(iw_handler) at76c503_iw_handler_set_rts,	// SIOCSIWRTS
	(iw_handler) at76c503_iw_handler_get_rts,	// SIOCGIWRTS
	(iw_handler) at76c503_iw_handler_set_frag,	// SIOCSIWFRAG
	(iw_handler) at76c503_iw_handler_get_frag,	// SIOCGIWFRAG
	(iw_handler) NULL,				// SIOCSIWTXPOW
	(iw_handler) at76c503_iw_handler_get_txpow,	// SIOCGIWTXPOW
	(iw_handler) at76c503_iw_handler_set_retry,	// SIOCSIWRETRY
	(iw_handler) at76c503_iw_handler_get_retry,	// SIOCGIWRETRY
	(iw_handler) at76c503_iw_handler_set_encode,	// SIOCSIWENCODE
	(iw_handler) at76c503_iw_handler_get_encode,	// SIOCGIWENCODE
	(iw_handler) at76c503_iw_handler_set_power,	// SIOCSIWPOWER
	(iw_handler) at76c503_iw_handler_get_power,	// SIOCGIWPOWER
};

/*******************************************************************************
 * structure that advertises the private iw handlers of this driver
 */
static const iw_handler	at76c503_priv_handlers[] =
{
	(iw_handler) at76c503_iw_handler_PRIV_IOCTL_SET_SHORT_PREAMBLE,
	(iw_handler) NULL,
	(iw_handler) at76c503_iw_handler_PRIV_IOCTL_SET_DEBUG,
	(iw_handler) NULL,
	(iw_handler) at76c503_iw_handler_PRIV_IOCTL_SET_POWERSAVE_MODE,
	(iw_handler) NULL,
	(iw_handler) at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_TIMES,
	(iw_handler) NULL,
	(iw_handler) at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_MODE,
	(iw_handler) NULL,
};

static const struct iw_handler_def at76c503_handler_def =
{
	.num_standard	= sizeof(at76c503_handlers)/sizeof(iw_handler),
	.num_private	= sizeof(at76c503_priv_handlers)/sizeof(iw_handler),
	.num_private_args = sizeof(at76c503_priv_args)/
		sizeof(struct iw_priv_args),
	.standard	= (iw_handler *) at76c503_handlers,
	.private	= (iw_handler *) at76c503_priv_handlers,
	.private_args	= (struct iw_priv_args *) at76c503_priv_args,
#if WIRELESS_EXT > 15
	.spy_offset	= offsetof(struct at76c503, spy_data),
#endif // #if WIRELESS_EXT > 15

};

#endif // #if WIRELESS_EXT > 12



static
int at76c503_ioctl(struct net_device *netdev, struct ifreq *rq, int cmd)
{
	struct at76c503 *dev = (struct at76c503*)netdev->priv;
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret = 0;
	char *extra;

	if (! netif_device_present(netdev))
		return -ENODEV;
	
	if (down_interruptible(&dev->sem))
                return -EINTR;
	
	switch (cmd) {
		
	// rudimentary ethtool support for hotplug of SuSE 8.3
	case SIOCETHTOOL:
	{
		ret = ethtool_ioctl(dev,rq->ifr_data);
	}
	break;
	
	case SIOCGIWNAME:
	{
		at76c503_iw_handler_get_name(netdev, NULL, 
			wrq->u.name, NULL);
	}
	break;
	
	case SIOCSIWFREQ:
	{
		ret = at76c503_iw_handler_set_freq(netdev, NULL, 
			&(wrq->u.freq), NULL);
	}
	break;
	
	case SIOCGIWFREQ:
	{ 
		at76c503_iw_handler_get_freq(netdev, NULL, 
			&(wrq->u.freq), NULL);
	}
	break;
	
	case SIOCSIWMODE:
	{
		ret = at76c503_iw_handler_set_mode(netdev, NULL, 
			&(wrq->u.mode), NULL);
	}
	break;
	
	case SIOCGIWMODE:
	{
		at76c503_iw_handler_get_mode(netdev, NULL, 
			&(wrq->u.mode), NULL);
	}
	break;
	
	case SIOCGIWRANGE:
		if (!(extra=kmalloc(sizeof(struct iw_range), GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		at76c503_iw_handler_get_range(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				sizeof(struct iw_range))) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;
	
	case SIOCGIWPRIV:
		if (!(extra=kmalloc(sizeof(at76c503_priv_args), GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}

		at76c503_iw_handler_get_priv(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				sizeof(at76c503_priv_args))) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;
	
#if (WIRELESS_EXT <= 15) && (IW_MAX_SPY > 0)
	// Set the spy list
	case SIOCSIWSPY:
		if (!(extra=kmalloc(sizeof(struct sockaddr) * IW_MAX_SPY, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		if (wrq->u.data.length > IW_MAX_SPY) {
			ret = -E2BIG;
			goto sspyerror;
		}
		
		if (copy_from_user(extra, wrq->u.data.pointer,
				   sizeof(struct sockaddr) * dev->iwspy_nr)) {
			dev->iwspy_nr = 0;
			ret = -EFAULT;
			goto sspyerror;
		}
		
		// never needs a device restart
		at76c503_iw_handler_set_spy(netdev, NULL, 
			&(wrq->u.data), extra);
sspyerror:
		kfree(extra);
		break;
	
	// Get the spy list
	case SIOCGIWSPY:
		if (!(extra = (char*)kmalloc((sizeof(struct sockaddr) +
					      sizeof(struct iw_quality)) * wrq->u.data.length, 
					     GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		at76c503_iw_handler_get_spy(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, dev->iwspy_addr,
				(sizeof(struct sockaddr) + 
				sizeof(struct iw_quality)) * 
				wrq->u.data.length)) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;

#endif // #if (WIRELESS_EXT <= 15) && (IW_MAX_SPY > 0)
	
	case SIOCSIWAP:
		at76c503_iw_handler_set_wap(netdev, NULL, 
			&(wrq->u.ap_addr), NULL);
		break;
	
	case SIOCGIWAP:
		at76c503_iw_handler_get_wap(netdev, NULL, 
			&(wrq->u.ap_addr), NULL);
		break;
	
	/*case SIOCGIWAPLIST:
	  break;*/
	
#if WIRELESS_EXT > 13
	case SIOCSIWSCAN:
		ret = at76c503_iw_handler_set_scan(netdev, NULL, NULL, NULL);
		break;
	
	case SIOCGIWSCAN:
		if (!(extra=kmalloc(IW_SCAN_MAX_DATA, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		at76c503_iw_handler_get_scan(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				wrq->u.data.length)) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;

#endif // #if WIRELESS_EXT > 13
	
	case SIOCSIWESSID:
		if (!(extra=kmalloc(IW_ESSID_MAX_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		if (wrq->u.data.length > IW_ESSID_MAX_SIZE) {
			ret = -E2BIG;
			goto sessiderror;
		}
		
		if (copy_from_user(extra, wrq->u.data.pointer, 
				wrq->u.data.length)) {
			ret = -EFAULT;
			goto sessiderror;
		}
		
		ret = at76c503_iw_handler_set_essid(netdev, NULL, 
			&(wrq->u.data), extra);
sessiderror:
		kfree(extra);
		break;
	
	case SIOCGIWESSID:
		if (!(extra=kmalloc(IW_ESSID_MAX_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		at76c503_iw_handler_get_essid(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				wrq->u.data.length)) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;
	
	case SIOCSIWNICKN:
		if (!(extra=(char*)kmalloc(IW_ESSID_MAX_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		if (wrq->u.data.length > IW_ESSID_MAX_SIZE) {
			ret = -E2BIG;
			goto snickerror;
		}
		
		if (copy_from_user(extra, wrq->u.data.pointer, 
				wrq->u.data.length)) {
			ret = -EFAULT;
			goto snickerror;
		}
		
		ret = at76c503_iw_handler_set_nickname(netdev, NULL, 
			&(wrq->u.data), extra);
snickerror:
		kfree(extra);
		break;
	
	case SIOCGIWNICKN:
		if (!(extra=kmalloc(IW_ESSID_MAX_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		at76c503_iw_handler_get_nickname(netdev, NULL, 
			&(wrq->u.data), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				wrq->u.data.length)) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;
	
	case SIOCSIWRATE:
		ret = at76c503_iw_handler_set_rate(netdev, NULL, 
			&(wrq->u.bitrate), NULL);
		break;
	
	case SIOCGIWRATE:
		at76c503_iw_handler_get_rate(netdev, NULL, 
			&(wrq->u.bitrate), NULL);
		break;
	
	case SIOCSIWRTS:
		ret = at76c503_iw_handler_set_rts(netdev, NULL, 
			&(wrq->u.rts), NULL);
		break;
	
	case SIOCGIWRTS:
		at76c503_iw_handler_get_rts(netdev, NULL, 
			&(wrq->u.rts), NULL);
		break;
	
	case SIOCSIWFRAG:
		ret = at76c503_iw_handler_set_frag(netdev, NULL, 
			&(wrq->u.frag), NULL);
		break;
	
	case SIOCGIWFRAG:
		at76c503_iw_handler_get_frag(netdev, NULL, 
			&(wrq->u.frag), NULL);
		break;
	
	/*case SIOCSIWTXPOW:
	break;*/
		
	case SIOCGIWTXPOW:
		at76c503_iw_handler_get_txpow(netdev, NULL, 
			&(wrq->u.power), NULL);
		break;
	
	case SIOCSIWRETRY:
		ret = at76c503_iw_handler_set_retry(netdev, NULL, 
			&(wrq->u.retry), NULL);
		break;
	
	case SIOCGIWRETRY:
		at76c503_iw_handler_get_retry(netdev, NULL, 
			&(wrq->u.retry), NULL);
		break;
	
	case SIOCSIWENCODE:
		if (!(extra=kmalloc(WEP_KEY_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		if (wrq->u.data.length > WEP_KEY_SIZE) {
			ret = -E2BIG;
			goto sencodeerror;
		}
		
		if (copy_from_user(extra, wrq->u.data.pointer, 
				wrq->u.data.length)) {
			ret = -EFAULT;
			goto sencodeerror;
		}
		
		ret = at76c503_iw_handler_set_encode(netdev, NULL, 
			&(wrq->u.encoding), extra);
sencodeerror:
		kfree(extra);
		break;
	
	case SIOCGIWENCODE:
		if (!(extra=kmalloc(WEP_KEY_SIZE + 1, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		at76c503_iw_handler_get_encode(netdev, NULL, 
			&(wrq->u.encoding), extra);
		
		if (copy_to_user(wrq->u.data.pointer, extra, 
				wrq->u.data.length)) {
			ret = -EFAULT;
		}
		
		kfree(extra);
		break;
	
	case SIOCSIWPOWER:
		ret = at76c503_iw_handler_set_power(netdev, NULL, 
			&(wrq->u.power), NULL);
		break;
		
	case SIOCGIWPOWER:
		at76c503_iw_handler_get_power(netdev, NULL, 
			&(wrq->u.power), NULL);
		break;
		
	case PRIV_IOCTL_SET_SHORT_PREAMBLE:
		ret = at76c503_iw_handler_PRIV_IOCTL_SET_SHORT_PREAMBLE
			(netdev, NULL, wrq->u.name, NULL);
		break;

	case PRIV_IOCTL_SET_DEBUG:
		if (!(extra=kmalloc(wrq->u.data.length, GFP_KERNEL))) {
			ret = -ENOMEM;
			break;
		}
		
		if (copy_from_user(extra, wrq->u.data.pointer, 
				wrq->u.data.length)) {
			ret = -EFAULT;
			goto set_debug_end;
		}       
		
		at76c503_iw_handler_PRIV_IOCTL_SET_DEBUG
			(netdev, NULL, &(wrq->u.data), extra);
set_debug_end:
		kfree(extra);
		break;
	
	case PRIV_IOCTL_SET_POWERSAVE_MODE:
		ret = at76c503_iw_handler_PRIV_IOCTL_SET_POWERSAVE_MODE
			(netdev, NULL, wrq->u.name, NULL);
		break;
	
	case PRIV_IOCTL_SET_SCAN_TIMES:
		ret = at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_TIMES
			(netdev, NULL, wrq->u.name, NULL);
		break;
	
	case PRIV_IOCTL_SET_SCAN_MODE:
		ret = at76c503_iw_handler_PRIV_IOCTL_SET_SCAN_MODE
			(netdev, NULL, wrq->u.name, NULL);
		break;
	
	default:
		dbg(DBG_IOCTL, "%s: ioctl not supported (0x%x)", netdev->name, 
			cmd);
		ret = -EOPNOTSUPP;
	}
	
	// we only restart the device if it was changed and already opened 
	// before
	if (ret == -EIWCOMMIT) {
		if (dev->open_count > 0) {
			at76c503_iw_handler_commit(netdev, NULL, NULL, NULL);
		}
		
		// reset ret so that this ioctl can return success
		ret = 0;
	}
	
	up(&dev->sem);
	return ret;
}

void at76c503_delete_device(struct at76c503 *dev)
{
	int i;
	int sem_taken;

	if (!dev) 
		return;

	/* signal to _stop() that the device is gone */
	dev->flags |= AT76C503A_UNPLUG; 

	dbg(DBG_PROC_ENTRY, "%s: ENTER",__FUNCTION__);

	if (dev->flags & AT76C503A_NETDEV_REGISTERED) {
		if ((sem_taken=down_trylock(&rtnl_sem)) != 0)
			info("%s: rtnl_sem already down'ed", __FUNCTION__);

		/* synchronously calls at76c503_stop() */
		unregister_netdevice(dev->netdev);

		if (!sem_taken)
			rtnl_unlock();
	}

	PUT_DEV(dev->udev);

	// assuming we used keventd, it must quiesce too
	flush_scheduled_work ();

	if(dev->bulk_out_buffer != NULL)
		kfree(dev->bulk_out_buffer);

	kfree(dev->ctrl_buffer);

	if(dev->write_urb != NULL) {
		usb_unlink_urb(dev->write_urb);
		usb_free_urb(dev->write_urb);
	}
	if(dev->read_urb != NULL) {
		usb_unlink_urb(dev->read_urb);
		usb_free_urb(dev->read_urb);
	}
	if(dev->ctrl_buffer != NULL) {
		usb_unlink_urb(dev->ctrl_urb);
		usb_free_urb(dev->ctrl_urb);
	}

	dbg(DBG_PROC_ENTRY,"%s: unlinked urbs",__FUNCTION__);

	if(dev->rx_skb != NULL)
		kfree_skb(dev->rx_skb);

	free_bss_list(dev);
	del_timer_sync(&dev->bss_list_timer);

	if (dev->istate == CONNECTED)
		iwevent_bss_disconnect(dev->netdev);

	for(i=0; i < NR_RX_DATA_BUF; i++)
		if (dev->rx_data[i].skb != NULL) {
			dev_kfree_skb(dev->rx_data[i].skb);
			dev->rx_data[i].skb = NULL;
		}
	dbg(DBG_PROC_ENTRY, "%s: before freeing dev/netdev", __FUNCTION__);
	free_netdev(dev->netdev); /* dev is in net_dev */ 
	dbg(DBG_PROC_ENTRY, "%s: EXIT", __FUNCTION__);
}

static int at76c503_alloc_urbs(struct at76c503 *dev)
{
	struct usb_interface *interface = dev->interface;
//	struct usb_host_interface *iface_desc = &interface->altsetting[0];
	struct usb_endpoint_descriptor *endpoint;
	struct usb_device *udev = dev->udev;
	int i, buffer_size;

	dbg(DBG_PROC_ENTRY, "%s: ENTER", __FUNCTION__);

	dbg(DBG_URB, "%s: NumEndpoints %d ", __FUNCTION__, NUM_EP(interface));

	for(i = 0; i < NUM_EP(interface); i++) {
		endpoint = &EP(interface,i);

		dbg(DBG_URB, "%s: %d. endpoint: addr x%x attr x%x",
		    __FUNCTION__,
		    i,
		    endpoint->bEndpointAddress,
		    endpoint->bmAttributes);

		if ((endpoint->bEndpointAddress & 0x80) &&
		    ((endpoint->bmAttributes & 3) == 0x02)) {
			/* we found a bulk in endpoint */

			dev->read_urb = alloc_urb(0, GFP_KERNEL);
			if (!dev->read_urb) {
				err("No free urbs available");
				return -1;
			}
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
		}
		
		if (((endpoint->bEndpointAddress & 0x80) == 0x00) &&
		    ((endpoint->bmAttributes & 3) == 0x02)) {
			/* we found a bulk out endpoint */
			dev->write_urb = alloc_urb(0, GFP_KERNEL);
			if (!dev->write_urb) {
				err("no free urbs available");
				return -1;
			}
			buffer_size = sizeof(struct at76c503_tx_buffer) + 
			  MAX_PADDING_SIZE;
			dev->bulk_out_size = buffer_size;
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_out_buffer = kmalloc (buffer_size, GFP_KERNEL);
			if (!dev->bulk_out_buffer) {
				err("couldn't allocate bulk_out_buffer");
				return -1;
			}
			FILL_BULK_URB(dev->write_urb, udev, 
				      usb_sndbulkpipe(udev, 
						      endpoint->bEndpointAddress),
				      dev->bulk_out_buffer, buffer_size,
				      (usb_complete_t)at76c503_write_bulk_callback, dev);
		}
	}

	dev->ctrl_urb = alloc_urb(0, GFP_KERNEL);
	if (!dev->ctrl_urb) {
		err("no free urbs available");
		return -1;
	}
	dev->ctrl_buffer = kmalloc(1024, GFP_KERNEL);
	if (!dev->ctrl_buffer) {
		err("couldn't allocate ctrl_buffer");
		return -1;
	}

	dbg(DBG_PROC_ENTRY, "%s: EXIT", __FUNCTION__);

	return 0;
}

struct at76c503 *alloc_new_device(struct usb_device *udev, int board_type,
				     const char *netdev_name)
{
	struct net_device *netdev;
	struct at76c503 *dev = NULL;
	int i;

	/* allocate memory for our device state and intialize it */
	netdev = alloc_etherdev(sizeof(struct at76c503));
	if (netdev == NULL) {
		err("out of memory");
		return NULL;
	}

	dev = (struct at76c503 *)netdev->priv;
	memset(dev, 0, sizeof(*dev));

	dev->udev = udev;
	dev->netdev = netdev;

	init_MUTEX (&dev->sem);
	INIT_WORK (&dev->kevent, kevent, dev);

	dev->open_count = 0;
	dev->flags = 0;

	init_timer(&dev->restart_timer);
	dev->restart_timer.data = (unsigned long)dev;
	dev->restart_timer.function = restart_timeout;

	init_timer(&dev->mgmt_timer);
	dev->mgmt_timer.data = (unsigned long)dev;
	dev->mgmt_timer.function = mgmt_timeout;

	init_timer(&dev->fw_dl_timer);
	dev->fw_dl_timer.data = (unsigned long)dev;
	dev->fw_dl_timer.function = fw_dl_timeout;


	dev->mgmt_spinlock = SPIN_LOCK_UNLOCKED;
	dev->next_mgmt_bulk = NULL;
	dev->istate = INTFW_DOWNLOAD;

	/* initialize empty BSS list */
	dev->curr_bss = dev->new_bss = NULL;
	INIT_LIST_HEAD(&dev->bss_list);
	dev->bss_list_spinlock = SPIN_LOCK_UNLOCKED;

	init_timer(&dev->bss_list_timer);
	dev->bss_list_timer.data = (unsigned long)dev;
	dev->bss_list_timer.function = bss_list_timeout;

#if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
	dev->spy_spinlock = SPIN_LOCK_UNLOCKED;
#if WIRELESS_EXT <= 15
	dev->iwspy_nr = 0;
#endif
#endif

	/* mark all rx data entries as unused */
	for(i=0; i < NR_RX_DATA_BUF; i++)
		dev->rx_data[i].skb = NULL;

	dev->tasklet.func = rx_tasklet;
	dev->tasklet.data = (unsigned long)dev;

	dev->board_type = board_type;

	dev->pm_mode = pm_mode;
	dev->pm_period_us = pm_period;

	strcpy(netdev->name, netdev_name);

	return dev;
} /* alloc_new_device */

/* == PROC init_new_device == 
   firmware got downloaded, we can continue with init */
/* We may have to move the register_netdev into alloc_new_device,
   because hotplug may try to configure the netdev _before_
   (or parallel to) the download of firmware */
int init_new_device(struct at76c503 *dev)
{
	struct net_device *netdev = dev->netdev;
	int ret;

	/* set up the endpoint information */
	/* check out the endpoints */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
	dev->interface = dev->udev->actconfig->interface[0];
#else
	dev->interface = &dev->udev->actconfig->interface[0];
#endif

	dbg(DBG_DEVSTART, "USB interface: %d endpoints",
	    NUM_EP(dev->interface));

	/* we let this timer run the whole time this driver instance lives */
	mod_timer(&dev->bss_list_timer, jiffies+BSS_LIST_TIMEOUT);

	if(at76c503_alloc_urbs(dev) < 0)
		goto error;

	/* get firmware version */
	ret = get_mib(dev->udev, MIB_FW_VERSION, (u8*)&dev->fw_version, sizeof(dev->fw_version));
	if((ret < 0) || ((dev->fw_version.major == 0) && 
			 (dev->fw_version.minor == 0) && 
			 (dev->fw_version.patch == 0) && 
			 (dev->fw_version.build == 0))){
		err("getting firmware failed with %d, or version is 0", ret);
		err("this probably means that the ext. fw was not loaded correctly");
		goto error;
	}

	/* fw 0.84 doesn't send FCS with rx data */
	if (dev->fw_version.major == 0 && dev->fw_version.minor <= 84)
		dev->rx_data_fcs_len = 0;
	else
		dev->rx_data_fcs_len = 4;

	info("$Id: at76c503.c,v 1.66 2004/08/18 22:01:45 jal2 Exp $ compiled %s %s", __DATE__, __TIME__);
	info("firmware version %d.%d.%d #%d (fcs_len %d)",
	     dev->fw_version.major, dev->fw_version.minor,
	     dev->fw_version.patch, dev->fw_version.build,
	     dev->rx_data_fcs_len);

	/* MAC address */
	ret = get_hw_config(dev);
	if(ret < 0){
		err("could not get MAC address");
		goto error;
	}

        dev->domain = getRegDomain(dev->regulatory_domain);
	/* init. netdev->dev_addr */
	memcpy(netdev->dev_addr, dev->mac_addr, ETH_ALEN);
	info("device's MAC %s, regulatory domain %s (id %d)",
	     mac2str(dev->mac_addr), dev->domain->name,
	     dev->regulatory_domain);

	/* initializing */
	dev->channel = DEF_CHANNEL;
	dev->iw_mode = IW_MODE_ADHOC;
	memset(dev->essid, 0, IW_ESSID_MAX_SIZE);
	memcpy(dev->essid, DEF_ESSID, DEF_ESSID_LEN);
	dev->essid_size = DEF_ESSID_LEN;
	strncpy(dev->nickn, DEF_ESSID, sizeof(dev->nickn));
	dev->rts_threshold = DEF_RTS_THRESHOLD;
	dev->frag_threshold = DEF_FRAG_THRESHOLD;
	dev->short_retry_limit = DEF_SHORT_RETRY_LIMIT;
	//dev->long_retr_limit = DEF_LONG_RETRY_LIMIT;
	dev->txrate = TX_RATE_AUTO;
	dev->preamble_type = preamble_type;
	dev->auth_mode = auth_mode ? IEEE802_11_AUTH_ALG_SHARED_SECRET :
	  IEEE802_11_AUTH_ALG_OPEN_SYSTEM;
	dev->scan_min_time = scan_min_time;
	dev->scan_max_time = scan_max_time;
	dev->scan_mode = scan_mode;

	netdev->flags &= ~IFF_MULTICAST; /* not yet or never */
	netdev->open = at76c503_open;
	netdev->stop = at76c503_stop;
	netdev->get_stats = at76c503_get_stats;
	netdev->get_wireless_stats = at76c503_get_wireless_stats;
	netdev->hard_start_xmit = at76c503_tx;
	netdev->tx_timeout = at76c503_tx_timeout;
	netdev->watchdog_timeo = 2 * HZ;
#if WIRELESS_EXT > 12
	netdev->wireless_handlers = 
		(struct iw_handler_def*)&at76c503_handler_def;
#endif
	netdev->do_ioctl = at76c503_ioctl;
	netdev->set_multicast_list = at76c503_set_multicast;
	netdev->set_mac_address = at76c503_set_mac_address;
	//  netdev->hard_header_len = 8 + sizeof(struct ieee802_11_hdr);
	/*
//    netdev->hard_header = at76c503_header;
*/

	/* putting this inside rtnl_lock() - rtnl_unlock() hangs modprobe ...? */
	ret = register_netdev(dev->netdev);
	if (ret) {
		err("unable to register netdevice %s (status %d)!",
		    dev->netdev->name, ret);
		goto error;
	}
	info("registered %s", dev->netdev->name);
	dev->flags |= AT76C503A_NETDEV_REGISTERED;

	return 0;

 error:
	at76c503_delete_device(dev);
	return -1;

} /* init_new_device */


/* == PROC at76c503_get_fw_info ==
   disassembles the firmware image into version, str,
   internal and external fw part. returns 0 on success, < 0 on error */
static int at76c503_get_fw_info(u8 *fw_data, int fw_size,
				u32 *board, u32 *version, char **str,
				u8 **int_fw, u32 *int_fw_size,
				u8 **ext_fw, u32 *ext_fw_size)
{
/* fw structure (all numbers are little_endian)
   offset  length  description
   0       4       crc 32 (seed ~0, no post, all gaps are zeros, header included)
   4       4       board type (see at76c503.h)
   8       4       version (major<<24|middle<<16|minor<<8|build)
   c       4       offset of printable string (id) area from begin of image
                   (must be \0 terminated !)
  10       4       offset of internal fw part area
  14       4       length of internal fw part
  18       4       offset of external fw part area (may be first byte _behind_
                   image in case we have no external part)
  1c       4       length of external fw part
*/

	u32 val;
	
	if (fw_size < 0x21) {
		err("fw too short (x%x)",fw_size);
		return -EFAULT;
	}

	/* crc currently not checked */

	memcpy(&val,fw_data+4,4);
	*board = le32_to_cpu(val);

	memcpy(&val,fw_data+8,4);
	*version = le32_to_cpu(val);

	memcpy(&val,fw_data+0xc,4);
	*str = fw_data + le32_to_cpu(val);

	memcpy(&val,fw_data+0x10,4);
	*int_fw = fw_data + le32_to_cpu(val);
	memcpy(&val,fw_data+0x14,4);
	*int_fw_size = le32_to_cpu(val);

	memcpy(&val,fw_data+0x18,4);
	*ext_fw = fw_data + le32_to_cpu(val);
	memcpy(&val,fw_data+0x1c,4);
	*ext_fw_size = le32_to_cpu(val);

	return 0;
}

/* == PROC at76c503_do_probe == */
int at76c503_do_probe(struct module *mod, struct usb_device *udev, 
		      struct usb_driver *calling_driver,
		      u8 *fw_data, int fw_size, u32 board_type,
		      const char *netdev_name, void **devptr)
{
	struct usb_interface *intf = 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
		udev->actconfig->interface[0];
#else
		&udev->actconfig->interface[0];
#endif
	int ret;
	struct at76c503 *dev = NULL;
	int op_mode;
	char *id_str;
	u32 version;

	GET_DEV(udev);

	if ((dev=alloc_new_device(udev, (u8)board_type, netdev_name)) == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	op_mode = get_op_mode(udev);

	usb_set_intfdata(intf, dev);
	dev->interface = intf;

	dbg(DBG_DEVSTART, "opmode %d", op_mode);

	/* we get OPMODE_NONE with 2.4.23, SMC2662W-AR ???
	   we get 204 with 2.4.23, Fiberline FL-WL240u (505A+RFMD2958) ??? */

	if (op_mode == OPMODE_HW_CONFIG_MODE) {
	  err("cannot handle a device in HW_CONFIG_MODE (opmode %d)", op_mode);
	  ret = -ENODEV;
	  goto error;
	}

	if (op_mode != OPMODE_NORMAL_NIC_WITH_FLASH &&
	    op_mode != OPMODE_NORMAL_NIC_WITHOUT_FLASH) {

		dbg(DBG_DEVSTART, "need to download firmware");

		/* disassem. the firmware */
		if ((ret=at76c503_get_fw_info(fw_data, fw_size, &dev->board_type,
					      &version, &id_str,
					      &dev->intfw, &dev->intfw_size,
					      &dev->extfw, &dev->extfw_size))) {
			goto error;
		}

		dbg(DBG_DEVSTART, "firmware board %u version %u.%u.%u#%u "
		    "(int %x:%x, ext %x:%x)",
		    dev->board_type, version>>24,(version>>16)&0xff,
		    (version>>8)&0xff, version&0xff,
		    dev->intfw_size, dev->intfw-fw_data,
		    dev->extfw_size, dev->extfw-fw_data);
		if (*id_str)
			dbg(DBG_DEVSTART, "firmware id %s",id_str);

		if (dev->board_type != board_type) {
			err("inconsistent board types %u != %u",
			    board_type, dev->board_type);
			at76c503_delete_device(dev);
			goto error;
		}

		/* download internal firmware part */
		dbg(DBG_DEVSTART, "downloading internal firmware");
		NEW_STATE(dev,INTFW_DOWNLOAD);
		defer_kevent(dev,KEVENT_INTERNAL_FW);

	} else {
		/* internal firmware already inside the device */
		/* get firmware version to test if external firmware is loaded */
		/* This works only for newer firmware, e.g. the Intersil 0.90.x
		   says "control timeout on ep0in" and subsequent get_op_mode() fail
		   too :-( */
		int force_fw_dwl = 0;

		/* disassem. the firmware */
		if ((ret=at76c503_get_fw_info(fw_data, fw_size, &dev->board_type,
					      &version, &id_str,
					      &dev->intfw, &dev->intfw_size,
					      &dev->extfw, &dev->extfw_size))) {
			goto error;
		}
		
		/* if version >= 0.100.x.y or device with built-in flash we can query the device
		 * for the fw version */
		if (version >= ((0<<24)|(100<<16)) || (op_mode == OPMODE_NORMAL_NIC_WITH_FLASH)) {
			ret = get_mib(udev, MIB_FW_VERSION, (u8*)&dev->fw_version, 
				      sizeof(dev->fw_version));
		} else {
			/* force fw download only if the device has no flash inside */
			force_fw_dwl = 1;
		}

		if ((force_fw_dwl) || (ret < 0) || ((dev->fw_version.major == 0) && 
						       (dev->fw_version.minor == 0) && 
						       (dev->fw_version.patch == 0) && 
						       (dev->fw_version.build == 0))) {
			if (force_fw_dwl)
				dbg(DBG_DEVSTART, "forced download of external firmware part");
			else
				dbg(DBG_DEVSTART, "cannot get firmware (ret %d) or all zeros "
				    "- download external firmware", ret);
			dbg(DBG_DEVSTART, "firmware board %u version %u.%u.%u#%u "
			    "(int %x:%x, ext %x:%x)",
			    dev->board_type, version>>24,(version>>16)&0xff,
			    (version>>8)&0xff, version&0xff,
			    dev->intfw_size, dev->intfw-fw_data,
			    dev->extfw_size, dev->extfw-fw_data);
			if (*id_str)
				dbg(DBG_DEVSTART, "firmware id %s",id_str);

			if (dev->board_type != board_type) {
				err("inconsistent board types %u != %u",
				    board_type, dev->board_type);
				at76c503_delete_device(dev);
				goto error;
			}

			NEW_STATE(dev,EXTFW_DOWNLOAD);
			defer_kevent(dev,KEVENT_EXTERNAL_FW);
		} else {
			NEW_STATE(dev,INIT);
			if (init_new_device(dev) < 0) {
				ret = -ENODEV;
				goto error;
			}
		}
	}

	SET_NETDEV_DEV(dev->netdev, &intf->dev);

	*devptr = dev; /* return dev for 2.4.x kernel probe functions */
	return 0;
error:
	PUT_DEV(udev);
	*devptr = NULL;
	return ret;
}

/**
 * 	at76c503_usbdfu_post
 *
 * 	Called by usbdfu driver after the firmware has been downloaded, before
 * 	the final reset.
 * 	(this is called in a usb probe (khubd) context)
 */

int at76c503_usbdfu_post(struct usb_device *udev)
{
	int result;

	dbg(DBG_DEVSTART, "Sending remap command...");
	result = at76c503_remap(udev);
	if (result < 0) {
		err("Remap command failed (%d)", result);
		return result;
	}
	return 0;
}

/* == PROC == */


/**
 *	at76c503_init
 */
static int __init at76c503_init(void)
{
	info(DRIVER_DESC " " DRIVER_VERSION);
	return 0;
}

/**
 *	at76c503_exit
 */
static void __exit at76c503_exit(void)
{
	info(DRIVER_DESC " " DRIVER_VERSION " exit");
}

module_init (at76c503_init);
module_exit (at76c503_exit);

EXPORT_SYMBOL(at76c503_do_probe);
EXPORT_SYMBOL(at76c503_delete_device);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
