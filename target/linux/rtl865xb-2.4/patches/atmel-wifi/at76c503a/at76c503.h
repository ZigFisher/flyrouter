/* -*- linux-c -*- */
/* $Id: at76c503.h,v 1.26 2004/08/18 22:01:45 jal2 Exp $
 *
 * Copyright (c) 2002 - 2003 Oliver Kurth
 *           (c) 2003 - 2004 Jörg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 *  This driver was based on information from the Sourceforge driver
 *  released and maintained by Atmel:
 *
 *  http://sourceforge.net/projects/atmelwlandriver/
 *
 *  althrough the code was completely re-written.
 *  It would have been impossible without Atmel's decision to
 *  release an Open Source driver (unfortunately the firmware was
 *  kept binary only). Thanks for that decision to Atmel!
 *  
 *  For the latest version of this driver, mailinglists
 *  and other info,  please check
 *        http://at76c503a.berlios.de/
 */

#ifndef _AT76C503_H
#define _AT76C503_H

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#if WIRELESS_EXT > 12
#include <net/iw_handler.h>
#endif
#include <linux/version.h>

#include "at76_ieee802_11.h" /* we need some constants here */

#ifndef COMPILE_FIRMWARE_INTO_DRIVER
# define CONFIG_AT76C503_FIRMWARE_DOWNLOAD
# define VERSION_APPEND "-fw_dwl"
#else
# ifdef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
#  undef CONFIG_AT76C503_FIRMWARE_DOWNLOAD
# endif
# define VERSION_APPEND "-static"
#endif
 
/* current driver version */
#define DRIVER_VERSION "v0.12beta17" VERSION_APPEND

/* Workqueue / task queue backwards compatibility stuff */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41)
#include <linux/workqueue.h>
#else
#include <linux/tqueue.h>
#define work_struct tq_struct
#define INIT_WORK(a,b,c) INIT_TQUEUE(a,b,c)
#define schedule_work(w) schedule_task(w)
#define flush_scheduled_work() flush_scheduled_tasks()
#endif

/* Backward compatibility for free_netdev() */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#ifndef HAVE_FREE_NETDEV
#define free_netdev(dev) kfree(dev)
#endif
#endif

/* this wasn't even defined in early 2.4.x kernels ... */
#ifndef SIOCIWFIRSTPRIV
#  define SIOCIWFIRSTPRIV SIOCDEVPRIVATE
#endif

/* WIRELESS_EXT 8 does not provide iw_point ! */
#if WIRELESS_EXT <= 8
/* this comes from <linux/wireless.h> */
#undef IW_MAX_SPY 
#define IW_MAX_SPY 0
#endif

/* our private ioctl's */
/* set preamble length*/
#define PRIV_IOCTL_SET_SHORT_PREAMBLE  (SIOCIWFIRSTPRIV + 0x0)
/* set debug parameter */
#define PRIV_IOCTL_SET_DEBUG           (SIOCIWFIRSTPRIV + 0x2)
/* set power save mode (incl. the Atmel proprietary smart save mode */
#define PRIV_IOCTL_SET_POWERSAVE_MODE  (SIOCIWFIRSTPRIV + 0x4)
/* set min and max channel times for scan */
#define PRIV_IOCTL_SET_SCAN_TIMES      (SIOCIWFIRSTPRIV + 0x6)
/* set scan mode */
#define PRIV_IOCTL_SET_SCAN_MODE       (SIOCIWFIRSTPRIV + 0x8)

#define DEVICE_VENDOR_REQUEST_OUT    0x40
#define DEVICE_VENDOR_REQUEST_IN     0xc0
#define INTERFACE_VENDOR_REQUEST_OUT 0x41
#define INTERFACE_VENDOR_REQUEST_IN  0xc1
#define CLASS_REQUEST_OUT            0x21
#define CLASS_REQUEST_IN             0xa1

#define CMD_STATUS_IDLE                   0x00
#define CMD_STATUS_COMPLETE               0x01
#define CMD_STATUS_UNKNOWN                0x02
#define CMD_STATUS_INVALID_PARAMETER      0x03
#define CMD_STATUS_FUNCTION_NOT_SUPPORTED 0x04
#define CMD_STATUS_TIME_OUT               0x07
#define CMD_STATUS_IN_PROGRESS            0x08
#define CMD_STATUS_HOST_FAILURE           0xff
#define CMD_STATUS_SCAN_FAILED            0xf0

/* answers to get op mode */
#define OPMODE_NONE                         0x00
#define OPMODE_NORMAL_NIC_WITH_FLASH        0x01
#define OPMODE_HW_CONFIG_MODE               0x02
#define OPMODE_DFU_MODE_WITH_FLASH          0x03
#define OPMODE_NORMAL_NIC_WITHOUT_FLASH     0x04

#define CMD_SET_MIB    0x01
#define CMD_GET_MIB    0x02
#define CMD_SCAN       0x03
#define CMD_JOIN       0x04
#define CMD_START_IBSS 0x05
#define CMD_RADIO      0x06
#define CMD_STARTUP    0x0B
#define CMD_GETOPMODE  0x33

#define MIB_LOCAL      0x01
#define MIB_MAC_ADD    0x02
#define MIB_MAC        0x03
#define MIB_MAC_MGMT   0x05
#define MIB_MAC_WEP    0x06
#define MIB_PHY        0x07
#define MIB_FW_VERSION 0x08
#define MIB_MDOMAIN    0x09

#define ADHOC_MODE 1
#define INFRASTRUCTURE_MODE 2

/* values for struct mib_local, field preamble_type */
#define PREAMBLE_TYPE_SHORT 1
#define PREAMBLE_TYPE_LONG  0

/* values for tx_rate */
#define TX_RATE_1MBIT 0
#define TX_RATE_2MBIT 1
#define TX_RATE_5_5MBIT 2
#define TX_RATE_11MBIT 3
#define TX_RATE_AUTO 4

/* power management modi */
#define PM_ACTIVE     1
#define PM_SAVE       2
#define PM_SMART_SAVE 3

/* offsets into the MIBs we use to configure the device */
#define TX_AUTORATE_FALLBACK_OFFSET offsetof(struct mib_local,txautorate_fallback)
#define FRAGMENTATION_OFFSET        offsetof(struct mib_mac,frag_threshold)
#define PREAMBLE_TYPE_OFFSET        offsetof(struct mib_local,preamble_type)
#define RTS_OFFSET                  offsetof(struct mib_mac, rts_threshold)      

/* valid only for rfmd and 505 !*/
#define IBSS_CHANGE_OK_OFFSET       offsetof(struct mib_mac_mgmt, ibss_change)
#define IROAMING_OFFSET \
  offsetof(struct mib_mac_mgmt, multi_domain_capability_enabled)
/* the AssocID */
#define STATION_ID_OFFSET           offsetof(struct mib_mac_mgmt, station_id)
#define POWER_MGMT_MODE_OFFSET      offsetof(struct mib_mac_mgmt, power_mgmt_mode)
#define LISTEN_INTERVAL_OFFSET      offsetof(struct mib_mac, listen_interval)

#define BOARDTYPE_503_INTERSIL_3861 1
#define BOARDTYPE_503_INTERSIL_3863 2
#define BOARDTYPE_503_RFMD          3
#define BOARDTYPE_503_RFMD_ACC      4
#define BOARDTYPE_505_RFMD          5
#define BOARDTYPE_505_RFMD_2958     6
#define BOARDTYPE_505A_RFMD_2958    7

struct hwcfg_r505 {
	u8 cr39_values[14];
	u8 reserved1[14];
	u8 bb_cr[14];
	u8 pidvid[4];
	u8 mac_addr[ETH_ALEN];
	u8 regulatory_domain;
	u8 reserved2[14];
	u8 cr15_values[14];
	u8 reserved3[3];
} __attribute__ ((packed));

struct hwcfg_rfmd {
	u8 cr20_values[14]; 
	u8 cr21_values[14]; 
	u8 bb_cr[14]; 
	u8 pidvid[4]; 
	u8 mac_addr[ETH_ALEN]; 
	u8 regulatory_domain; 
	u8 low_power_values[14];     
	u8 normal_power_values[14]; 
	u8 reserved1[3];   
} __attribute__ ((packed));

struct hwcfg_intersil {
	u8   mac_addr[ETH_ALEN];
	u8   cr31_values[14];
	u8   cr58_values[14];
	u8   pidvid[4];
	u8   regulatory_domain;
	u8   reserved[1];
} __attribute__ ((packed));

#define WEP_KEY_SIZE 13
#define NR_WEP_KEYS 4
#define WEP_SMALL_KEY_LEN (40/8)
#define WEP_LARGE_KEY_LEN (104/8)

struct at76c503_card_config{
	u8 exclude_unencrypted;
	u8 promiscuous_mode;
	u8 short_retry_limit;
	u8 encryption_type;
	u16 rts_threshold;
	u16 fragmentation_threshold;         // 256..2346
	u8 basic_rate_set[4];
	u8 auto_rate_fallback;                       //0,1
	u8 channel;
	u8 privacy_invoked;
	u8 wep_default_key_id;                        // 0..3
	u8 current_ssid[32];
	u8 wep_default_key_value[4][WEP_KEY_SIZE];
	u8 ssid_len;
	u8 short_preamble;
	u16 beacon_period;
} __attribute__ ((packed));

struct at76c503_command{
	u8 cmd;
	u8 reserved;
	u16 size;
} __attribute__ ((packed));

/* the length of the Atmel firmware specific rx header before IEEE 802.11 starts */
#define AT76C503_RX_HDRLEN offsetof(struct at76c503_rx_buffer, packet)

struct at76c503_rx_buffer {
	u16 wlength;
	u8 rx_rate;
	u8 newbss;
	u8 fragmentation;
	u8 rssi;
	u8 link_quality;
	u8 noise_level;
	u8 rx_time[4];
	u8 packet[IEEE802_11_MAX_FRAME_LEN];
} __attribute__ ((packed));

/* the length of the Atmel firmware specific tx header before IEEE 802.11 starts */
#define AT76C503_TX_HDRLEN offsetof(struct at76c503_tx_buffer, packet)

struct at76c503_tx_buffer {
	u16 wlength;
	u8 tx_rate;
	u8 padding;
	u8 reserved[4];
	u8 packet[IEEE802_11_MAX_FRAME_LEN];
} __attribute__ ((packed));

/* defines for scan_type below */
#define SCAN_TYPE_ACTIVE  0
#define SCAN_TYPE_PASSIVE 1

struct at76c503_start_scan {
	u8   bssid[ETH_ALEN];
	u8   essid[32];
	u8   scan_type;
	u8   channel;
	u16  probe_delay;
	u16  min_channel_time;
	u16  max_channel_time;
	u8   essid_size;
	u8   international_scan;
} __attribute__ ((packed));

struct at76c503_start_bss {
	u8 bssid[ETH_ALEN];
	u8 essid[32];
	u8 bss_type;
	u8 channel;
	u8 essid_size;
	u8 reserved[3];
} __attribute__ ((packed));

struct at76c503_join {
	u8 bssid[ETH_ALEN];
	u8 essid[32];
	u8 bss_type;
	u8 channel;
	u16 timeout;
	u8 essid_size;
	u8 reserved;
} __attribute__ ((packed));

struct set_mib_buffer {
	u8 type;
	u8 size;
	u8 index;
	u8 reserved;
	u8 data[72];
} __attribute__ ((packed));

struct mib_local {
        u16 reserved0;
        u8  beacon_enable;
        u8  txautorate_fallback;
        u8  reserved1;
        u8  ssid_size;
        u8  promiscuous_mode;
        u16 reserved2;
        u8  preamble_type;
        u16 reserved3;
} __attribute__ ((packed));

struct mib_mac_addr {
	u8 mac_addr[ETH_ALEN];
        u8 res[2]; /* ??? */
        u8 group_addr[4][ETH_ALEN];
        u8 group_addr_status[4];
} __attribute__ ((packed));

struct mib_mac {
        u32 max_tx_msdu_lifetime;
        u32 max_rx_lifetime;
        u16 frag_threshold;
        u16 rts_threshold;
        u16 cwmin;
        u16 cwmax;
        u8  short_retry_time;
        u8  long_retry_time;
        u8  scan_type; /* active or passive */
        u8  scan_channel;
        u16 probe_delay; /* delay before sending a ProbeReq in active scan, RO */
        u16 min_channel_time;
        u16 max_channel_time;
        u16 listen_interval;
        u8  desired_ssid[32];
        u8  desired_bssid[ETH_ALEN];
        u8  desired_bsstype; /* ad-hoc or infrastructure */
        u8  reserved2;
} __attribute__ ((packed));

struct mib_mac_mgmt {
	u16 beacon_period;
	u16 CFP_max_duration;
	u16 medium_occupancy_limit;
	u16 station_id;  /* assoc id */
	u16 ATIM_window;
	u8  CFP_mode;
	u8  privacy_option_implemented;
	u8  DTIM_period;
	u8  CFP_period;
	u8  current_bssid[ETH_ALEN];
	u8  current_essid[32];
	u8  current_bss_type;
	u8  power_mgmt_mode;
	/* rfmd and 505 */
	u8  ibss_change;
	u8  res;
	u8  multi_domain_capability_implemented;
	u8  multi_domain_capability_enabled;
	u8  country_string[3];
	u8  reserved[3];
} __attribute__ ((packed));

struct mib_mac_wep {
        u8 privacy_invoked; /* 0 disable encr., 1 enable encr */
        u8 wep_default_key_id;
        u8 wep_key_mapping_len;
        u8 exclude_unencrypted;
        u32 wep_icv_error_count;
        u32 wep_excluded_count;
        u8 wep_default_keyvalue[NR_WEP_KEYS][WEP_KEY_SIZE];
        u8 encryption_level; /* 1 for 40bit, 2 for 104bit encryption */
} __attribute__ ((packed));

struct mib_phy {
	u32 ed_threshold;
  
	u16 slot_time;
	u16 sifs_time;
	u16 preamble_length;
	u16 plcp_header_length;
	u16 mpdu_max_length;
	u16 cca_mode_supported;
  
	u8 operation_rate_set[4];
	u8 channel_id;
	u8 current_cca_mode;
	u8 phy_type;
	u8 current_reg_domain;
} __attribute__ ((packed));

struct mib_fw_version {
        u8 major;
        u8 minor;
        u8 patch;
        u8 build;
} __attribute__ ((packed));

struct mib_mdomain {
        u8 tx_powerlevel[14];
        u8 channel_list[14]; /* 0 for invalid channels */
} __attribute__ ((packed));

/* states in infrastructure mode */
enum infra_state {
	INIT,
	SCANNING,
	AUTHENTICATING,
	ASSOCIATING,
	REASSOCIATING,
	DISASSOCIATING,
	JOINING,
	CONNECTED,
	STARTIBSS,
	INTFW_DOWNLOAD,
	EXTFW_DOWNLOAD,
	WAIT_FOR_DISCONNECT,
};

/* a description of a regulatory domain and the allowed channels */
struct reg_domain {
  u16 code;
  char const *name;
  u32 channel_map; /* if bit N is set, channel (N+1) is allowed */
};

/* how long do we keep a (I)BSS in the bss_list in jiffies 
   this should be long enough for the user to retrieve the table
   (by iwlist ?) after the device started, because all entries from
   other channels than the one the device locks on get removed, too */
#define BSS_LIST_TIMEOUT (120*HZ)

/* struct to store BSS info found during scan */
#define BSS_LIST_MAX_RATE_LEN 32 /* 32 rates should be enough ... */

struct bss_info{
	struct list_head list;

	u8 mac[ETH_ALEN]; /* real mac address, differs 
			     for ad-hoc from bssid */
	u8 bssid[ETH_ALEN]; /* bssid */
	u8 ssid[IW_ESSID_MAX_SIZE+1]; /* ssid, +1 for trailing \0 
					 to make it printable */
	u8 ssid_len; /* length of ssid above */
	u8 channel;
	u16 capa; /* the capabilities of the BSS (in original endianess -
		     we only check IEEE802_11 bits in it) */
	u16 beacon_interval; /* the beacon interval in units of TU (1.024 ms)
				(in cpu endianess - we must calc. values from it) */
	u8 rates[BSS_LIST_MAX_RATE_LEN]; /* supported rates (list of bytes: 
				   (basic_rate ? 0x80 : 0) + rate/(500 Kbit/s); e.g. 
				   x82,x84,x8b,x96 for basic rates 1,2,5.5,11 MBit/s) */
	u8 rates_len;

	/* quality of received beacon */
	u8 rssi;
	u8 link_qual;
	u8 noise_level;

	unsigned long last_rx; /* time (jiffies) of last beacon received */
	u16 assoc_id;          /* if this is dev->curr_bss this is the assoc id we got
				  in a successful AssocResponse */
};

/* a rx data buffer to collect rx fragments */
struct rx_data_buf {
	u8 sender[ETH_ALEN]; /* sender address */
	u16 seqnr; /* sequence number */
	u16 fragnr; /* last fragment received */
	unsigned long last_rx; /* jiffies of last rx */
	struct sk_buff *skb; /* == NULL if entry is free */
};

#define NR_RX_DATA_BUF 8

/* how often do we try to submit a rx urb until giving up */
#define NR_SUBMIT_RX_TRIES 8

struct at76c503 {
	struct usb_device *udev;			/* save off the usb device pointer */
	struct net_device *netdev;			/* save off the net device pointer */
	struct net_device_stats stats;
	struct iw_statistics wstats;
	struct usb_interface *interface;		/* the interface for this device */
	
//	unsigned char		num_ports;		/* the number of ports this device has */
//	char			num_interrupt_in;	/* number of interrupt in endpoints we have */
//	char			num_bulk_in;		/* number of bulk in endpoints we have */
//	char			num_bulk_out;		/* number of bulk out endpoints we have */
	
	struct sk_buff *	rx_skb;			/* skbuff for receiving packets */
	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	
	unsigned char *	bulk_out_buffer;	/* the buffer to send data */
	int			bulk_out_size;		/* the size of the send buffer */
	struct urb *		write_urb;		/* the urb used to send data */
	struct urb *		read_urb;
	__u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */

//	struct work_struct	tqueue;			/* task queue for line discipline waking up */
	int			open_count;		/* number of times this port has been opened */
	struct semaphore	sem;			/* locks this structure */


	unsigned long kevent_flags;
	struct work_struct kevent;
	int nr_submit_rx_tries; /* number of tries to submit an rx urb left */
	struct tasklet_struct tasklet;
	struct urb *rx_urb; /* tmp urb pointer for rx_tasklet */

	unsigned char *ctrl_buffer;
	struct urb *ctrl_urb;

	u8 op_mode;

        /* the WEP stuff */
        int wep_enabled;      /* 1 if WEP is enabled */ 
        int wep_key_id;       /* key id to be used */
        u8 wep_keys[NR_WEP_KEYS][WEP_KEY_SIZE]; /* the four WEP keys,
						   5 or 13 bytes are used */
        u8  wep_keys_len[NR_WEP_KEYS]; /* the length of the above keys */

	int channel;
	int iw_mode;
	int curr_ap;
	u8 bssid[ETH_ALEN];
	u8 essid[IW_ESSID_MAX_SIZE];
	char nickn[IW_ESSID_MAX_SIZE+1]; /* nickname, only used in the iwconfig i/f */
	int essid_size;
	int radio_on;
	int promisc;

	int preamble_type; /* 0 - long preamble, 1 - short preamble */
	int auth_mode; /* authentication type: 0 open, 1 shared key */
	int txrate; /* 0,1,2,3 = 1,2,5.5,11 MBit, 4 is auto-fallback */
        int frag_threshold; /* threshold for fragmentation of tx packets */
        int rts_threshold; /* threshold for RTS mechanism */
	int short_retry_limit;
	//int long_retry_limit;

	int scan_min_time; /* scan min channel time */
	int scan_max_time; /* scan max channel time */
	int scan_mode;     /* SCAN_TYPE_ACTIVE, SCAN_TYPE_PASSIVE */

	/* the list we got from scanning */
	spinlock_t bss_list_spinlock; /* protects bss_list operations and setting
				     curr_bss and new_bss */
	struct list_head bss_list; /* the list of bss we received beacons from */
	struct timer_list bss_list_timer; /* a timer removing old entries from
					     the bss_list. It must aquire bss_list_spinlock
					     before and must not remove curr_bss nor
					     new_bss ! */
	struct bss_info *curr_bss; /* if istate == AUTH, ASSOC, REASSOC, JOIN or CONN 
				      dev->bss[curr_bss] is the currently selected BSS
				      we operate on */
	struct bss_info *new_bss; /* if istate == REASSOC dev->new_bss
				     is the new bss we want to reassoc to */
	
	u8 wanted_bssid[ETH_ALEN];
	int wanted_bssid_valid; /* != 0 if wanted_bssid is to be used */
	
	/* some data for infrastructure mode only */
	spinlock_t mgmt_spinlock; /* this spinlock protects access to
				     next_mgmt_bulk and istate */
	struct at76c503_tx_buffer *next_mgmt_bulk; /* pending management msg to
						     send via bulk out */
	enum infra_state istate;

	struct timer_list restart_timer; /* the timer we use to delay the restart a bit */

	struct timer_list mgmt_timer; /* the timer we use to repeat auth_req etc. */
	int retries; /* counts backwards while re-trying to send auth/assoc_req's */
	u16 assoc_id; /* the assoc_id for states JOINING, REASSOCIATING, CONNECTED */
	u8  pm_mode ; /* power management mode: ACTIVE, SAVE, SMART_SAVE */
	u32 pm_period_us; /* power manag. period (in us ?) - set by iwconfig */
	u32 pm_period_beacon; /* power manag. period (in beacon intervals
				 of the curr_bss) */
	u32 board_type; /* BOARDTYPE_* defined above*/

	struct reg_domain const *domain; /* the description of the regulatory domain */

	/* iwspy support */
#if (WIRELESS_EXT > 15) || (IW_MAX_SPY > 0)
	spinlock_t spy_spinlock;
#if WIRELESS_EXT > 15
	struct iw_spy_data spy_data;
#else
	int iwspy_nr; /* nr of valid entries below */
	struct sockaddr iwspy_addr[IW_MAX_SPY];
	struct iw_quality iwspy_stats[IW_MAX_SPY];
#endif
#endif

	/* These fields contain HW config provided by the device (not all of
	 * these fields are used by all board types) */
	u8 mac_addr[ETH_ALEN];
	u8 bb_cr[14];
	u8 pidvid[4];
	u8 regulatory_domain;
	u8 cr15_values[14];
	u8 cr20_values[14]; 
	u8 cr21_values[14]; 
	u8 cr31_values[14];
	u8 cr39_values[14];
	u8 cr58_values[14];
	u8 low_power_values[14];     
	u8 normal_power_values[14]; 

	struct at76c503_card_config card_config;
	struct mib_fw_version fw_version;

	int rx_data_fcs_len; /* length of the trailing FCS 
				(0 for fw <= 0.84.x, 4 otherwise) */

	/* store rx fragments until complete */
	struct rx_data_buf rx_data[NR_RX_DATA_BUF];

	/* firmware downloading stuff */
	struct timer_list fw_dl_timer; /* timer used to wait after REMAP
					  until device is reset */
	int extfw_size;
	int intfw_size;
	/* these point into a buffer managed by at76c503-xxx.o, no need to dealloc */
	u8 *extfw; /* points to external firmware part, extfw_size bytes long */
	u8 *intfw; /* points to internal firmware part, intfw_size bytes long */
	struct usb_driver *calling_driver; /* the calling driver: at76c503-{rfmd,i3861,i3863,...} */
	int flags; /* AT76C503A_UNPLUG signals at76c503a_stop() 
		      that the device was unplugged 
		      AT76C503A_NETDEV_REGISTERED signals that register_netdevice
		      xwas successfully called */
	char obuf[2*256+1]; /* global debug output buffer to reduce stack usage */
	char obuf_s[3*32]; /* small global debug output buffer to reduce stack usage */
	struct set_mib_buffer mib_buf; /* global buffer for set_mib calls */
};

#define AT76C503A_UNPLUG 1
#define AT76C503A_NETDEV_REGISTERED 2

/* Function prototypes */

int at76c503_do_probe(struct module *mod, struct usb_device *udev, 
		      struct usb_driver *calling_driver,
		      u8 *fw_data, int fw_size, u32 board_type,
		      const char *netdev_name, void **devptr);

void at76c503_delete_device(struct at76c503 *dev);

#endif /* _AT76C503_H */
