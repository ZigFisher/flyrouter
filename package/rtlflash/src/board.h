/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* $Header: /home/cvsroot/uClinux-dist/user/goahead-2.1.4/LINUX/board.h,v 1.92 2004/08/13 06:00:50 cfliu Exp $
*
* Abstract: Board specific definitions.
*
* $Author: cfliu $
*
* $Log: board.h,v $
* Revision 1.92  2004/08/13 06:00:50  cfliu
* +: Update port number from 8650's 5 to 8650B's 9
*
* Revision 1.91  2004/08/10 12:13:20  yjlou
* *: modify to support kernel mode
*
* Revision 1.90  2004/08/04 14:58:27  yjlou
* *: By default, we DO NOT define _SUPPORT_LARGE_FLASH_ !!
*
* Revision 1.89  2004/08/04 14:50:18  yjlou
* *: change type of __flash_base from 'int*' to 'uint32'
*
* Revision 1.88  2004/07/27 07:23:52  chenyl
* +: DoS ignore type setting
*
* Revision 1.87  2004/07/14 13:55:59  chenyl
* +: web page for MN queue
*
* Revision 1.86  2004/07/12 07:35:32  chhuang
* *: modify rate limit web page
*
* Revision 1.85  2004/07/12 07:03:47  tony
* +: add wan type dhcpl2tp.
*
* Revision 1.84  2004/07/12 04:25:36  chenyl
* *: extend existing port scan mechanism
*
* Revision 1.83  2004/07/08 10:51:41  tony
* *: change ipUp routine to new arch.
*
* Revision 1.82  2004/07/07 05:12:36  chhuang
* +: add a new WAN type (DHCP+L2TP). But not complete yet!!
*
* Revision 1.81  2004/07/06 06:19:25  chhuang
* +: add rate limit
*
* Revision 1.80  2004/06/29 09:40:01  cfliu
* *: Change WLAN rate defines
*
* Revision 1.79  2004/06/14 13:48:17  rupert
* +: Add work properly with MMU kernel
*
* Revision 1.78  2004/06/14 09:47:25  tony
* *: fix PPTP can't compiler well  when not define PPTP/L2TP.
*
* Revision 1.77  2004/06/11 09:20:18  tony
* +: using share memory instead of direct access for pRomeCfgParam.
*
* Revision 1.76  2004/06/11 00:44:56  cfliu
* +: Add port ON/Off webpage
*
* Revision 1.75  2004/06/10 14:35:27  cfliu
* +: Add port config webpage
*
* Revision 1.74  2004/06/10 10:34:52  tony
* +: add PPTP dial status: disconnecting, make redial function correct.
*
* Revision 1.73  2004/06/09 12:32:16  tony
* +: add PPTP/L2TP UI page.(Dial and Hang_Up).
*
* Revision 1.72  2004/06/08 10:54:22  cfliu
* +: Add WLAN dual mode webpages. Not yet completed...
*
* Revision 1.71  2004/05/28 09:49:16  yjlou
* +: support Protocol-Based NAT
*
* Revision 1.70  2004/05/27 05:12:36  tony
* fix multiple pppoe dial problem.
*
* Revision 1.69  2004/05/26 06:51:49  yjlou
* *: use IS_865XB() instead of IS_REV_B()
* *: use IS_865XA() instead of IS_REV_A()
*
* Revision 1.68  2004/05/21 12:08:50  chenyl
* *: TCP/UDP/ICMP spoof -> check for (sip) == (dip)
* *: modify DOS prevention webpage
*
* Revision 1.67  2004/05/20 08:43:55  chhuang
* add Web Page for QoS
*
* Revision 1.66  2004/05/17 12:17:00  cfliu
* Add AC Name field
*
* Revision 1.65  2004/05/13 13:27:01  yjlou
* +: loader version is migrated to "00.00.07".
* +: new architecture for INTEL flash (code is NOT verified).
* *: FLASH_BASE is decided by IS_REV_A()
* -: remove flash_map.h (content moved to flashdrv.h)
* -: remove un-necessary calling setIlev()
*
* Revision 1.64  2004/05/12 08:09:56  chhuang
* +: dhcp static binding
*
* Revision 1.63  2004/05/12 07:20:15  chenyl
* +: source ip blocking
* *: modify dos mechanism
*
* Revision 1.62  2004/05/12 05:15:05  tony
* support PPTP/L2TP in RTL865XB.
*
* Revision 1.61  2004/05/10 05:49:20  chhuang
* add src ip blocking for url-filtering
*
* Revision 1.60  2004/05/05 08:27:07  tony
* new features: add remote management spec
*
* Revision 1.59  2004/05/03 15:02:51  cfliu
* Add 8650B extension port support.
* Revise all WLAN/extport related code.
*
* Revision 1.58  2004/04/08 13:18:12  tony
* add PPTP/L2TP routine for MII lookback port.
*
* Revision 1.57  2004/04/08 12:40:56  cfliu
* Update WLAN webpages and related ASP code.
*
* Revision 1.56  2004/03/31 01:59:36  tony
* add L2TP wan type UI pages.
*
* Revision 1.55  2004/03/19 09:51:18  tony
* make the 'lan permit' acl is able to select by user.
*
* Revision 1.54  2004/03/03 03:43:31  tony
* add static routing table in turnkey.
*
* Revision 1.53  2004/02/05 07:11:16  tony
* add acl filter field: IP.
*
* Revision 1.52  2004/02/04 03:46:10  tony
* change lantype from uint8 to uint32
*
* Revision 1.51  2004/02/03 08:14:34  tony
* add UDP Blocking web UI configuration.
*
* Revision 1.50  2004/01/30 12:03:05  tony
* make Special Application is able to support multiple session.
*
* Revision 1.49  2004/01/16 12:16:46  tony
* modify ALG cfg params and web UI,
* rearrange rtl8651a_setAlgStatus() must called after rtl8651_addIpIntf()
*
* Revision 1.48  2004/01/14 02:46:24  hiwu
* add PPTP configuration
*
* Revision 1.47  2004/01/08 13:28:29  orlando
* dos/url/log related changes
*
* Revision 1.46  2004/01/08 12:13:44  tony
* add Port Range into Server Port.
* support Server Port for multiple session UI.
*
* Revision 1.45  2004/01/08 07:23:21  tony
* support multiple session UI for ACL, URL Filter, DoS log.
*
* Revision 1.44  2004/01/07 10:59:56  tony
* Support multiple session UI plugin for ACL.
*
* Revision 1.43  2004/01/07 09:10:04  tony
* add PPTP Client UI in Config Wizard.
*
* Revision 1.42  2004/01/07 07:36:36  tony
* Support multiple session UI plugin for ALG.
*
* Revision 1.41  2004/01/06 13:50:25  tony
* Support multiple session UI plugin for the following functions:
* DMZ, URL Filter, Dos Prevention
*
* Revision 1.40  2003/12/19 04:33:01  tony
* add Wireless Lan config pages: Basic Setting, Advance Setting, Security, Access Control, WDS
*
* Revision 1.39  2003/12/12 01:34:54  tony
* add NAPT toggle in Unnumbered PPPoE.
*
* Revision 1.38  2003/12/09 13:44:34  tony
* add ACL,DoS,URL Filter logging function in kernel space.
*
* Revision 1.37  2003/12/02 10:24:47  tony
* Add Routine: support DoS is able to set threshold by user(Webs GUI).
*
* Revision 1.36  2003/12/01 12:35:52  tony
* make ALG is able to be configured by users(Web GUI).
*
* Revision 1.35  2003/11/18 09:04:47  tony
* add routine: support mtu configure by user in pppoe.
*
* Revision 1.34  2003/11/13 13:33:58  tony
* pppoe: automaic Hang up all the connections after save.
*
* Revision 1.33  2003/11/13 12:52:06  tony
* add MTU and MRU field in pppoe.
*
* Revision 1.32  2003/11/07 06:31:22  tony
* add type PPPOECFGPARAM_DESTNETTYPE_NONE in MultiPPPoE dest network.
*
* Revision 1.31  2003/11/06 02:25:08  tony
* add field in multi-pppoe cfg.
*
* Revision 1.30  2003/11/04 09:30:56  tony
* modfiy special-application list in board.c
*
* Revision 1.29  2003/10/29 10:20:41  tony
* modify acl policy list.
*
* Revision 1.28  2003/10/29 01:48:24  tony
* fix trigger port bug: when ouside host send a SYN to internal computer,
* gateway return a RST packet problem.
*
* Revision 1.27  2003/10/24 10:25:58  tony
* add DoS attack interactive webpage,
* FwdEngine is able to get WAN status by rtl8651_wanStatus(0:disconnect,1:connect)
*
* Revision 1.26  2003/10/15 12:12:08  orlando
* add pppoeCfgParam[].pppx to keep track of hw ppp obj id (0,1,2,3,...)
* in association with linux ppp dial up interface (ppp0,ppp1,ppp2,ppp3,...)
*
* Revision 1.25  2003/10/14 08:15:01  tony
* add user account management routine
*
* Revision 1.24  2003/10/03 12:27:35  tony
* add NTP time sync routine in management web page.
*
* Revision 1.23  2003/10/03 01:26:42  tony
* add ServiceName field in pppoeCfgParam_t.
* add dynamic check in PPPoE/Unnumbered PPPoE/Multiple PPPoE web page.
*
* Revision 1.22  2003/10/02 10:50:17  orlando
* add manualHangup and whichPppObjId fields in pppoeCfgParam_t for auto
* reconnect implementation
*
* Revision 1.21  2003/10/01 05:57:31  tony
* add URL Filter routine
*
* Revision 1.20  2003/09/30 08:56:29  tony
* remove newServerpCfgParam[] from flash, rename ram PPPoeCfg to ramPppoeCfgParam
*
* Revision 1.19  2003/09/29 13:21:03  rupert
* add primary and secondary dns fields of ifCfgParam_s
*
* Revision 1.18  2003/09/29 08:57:32  tony
* add routine: when pppoe config changed, the dial function will be disabled.
* add routine: dhcps is able to start/stop server on runtime.
*
* Revision 1.17  2003/09/29 05:05:42  orlando
* add pppoeCfgParam[].svrMac/sessionId
*
* Revision 1.16  2003/09/26 01:55:31  orlando
* clean the structure
*
* Revision 1.15  2003/09/25 10:44:50  tony
* small change
*
* Revision 1.14  2003/09/25 10:14:50  tony
* modify pppoe webpage, support unnumbered IP, PPPoE, Multiple PPPoE.
*
* Revision 1.13  2003/09/25 06:43:32  rupert
* add contant of routing function
*
* Revision 1.12  2003/09/25 06:06:45  orlando
* adjust inclue files
*
* Revision 1.11  2003/09/25 03:48:35  orlando
* add romecfg.txt file to pass romeCfgParam structure pointer for other user space application
*
* Revision 1.10  2003/09/25 03:37:36  orlando
* integration...
*
* Revision 1.9  2003/09/25 02:15:32  orlando
* Big Change
*
* Revision 1.8  2003/09/24 07:10:30  rupert
* rearrange init sequence
*
* Revision 1.7  2003/09/23 03:47:29  tony
* add ddnsCfgParam,ddnsDefaultFactory,ddns_init
*
* Revision 1.6  2003/09/22 08:01:45  tony
* add UPnP Web Configuration Function Routine
*
* Revision 1.5  2003/09/22 06:33:37  tony
* add syslogd process start/stop by CGI.
* add eventlog download/clear routine.
*
* Revision 1.4  2003/09/19 06:06:51  tony
* *** empty log message ***
*
* Revision 1.3  2003/09/19 01:43:50  tony
* add dmz routine
*
* Revision 1.2  2003/09/18 02:05:45  tony
* modify pppoeCfgParam to array
*
* Revision 1.1.1.1  2003/08/27 06:20:15  rupert
* uclinux initial
*
* Revision 1.1.1.1  2003/08/27 03:08:53  rupert
*  initial version
*
* Revision 1.24  2003/07/03 12:15:59  tony
* fix some warning messages.
*
* Revision 1.23  2003/07/03 06:01:11  tony
* change dhcpcCfgParam_s field order, for 4 bytes alignment.
*
* Revision 1.22  2003/07/01 10:25:22  orlando
* call table driver server port and acl API in board.c
*
* Revision 1.21  2003/07/01 03:09:06  tony
* add aclGlobalCfgParam structure.
* modify aclCfgParam and serverpCfgParam structure.
*
* Revision 1.20  2003/06/30 13:27:55  tony
* add a field in server_port table
*
* Revision 1.19  2003/06/30 07:46:12  tony
* add ACL and Server_Port structure
*
* Revision 1.18  2003/06/27 13:53:25  orlando
* remove dhcpcCfgParam.valid field, use ifCfgParam[0].connType field
* modify rtConfigHook() for cloneMac:
* use dhcpcCfgParam.cloneMac if ifCfgParam[0].connType is
* DHCPC and dhcpcCfgParam.cmacValid is 1 during boottime.
*
* Revision 1.17  2003/06/27 05:33:06  orlando
* adjust some structure.
*
* Revision 1.16  2003/06/26 03:22:29  tony
* remove some element in dhcpc data structure.
*
* Revision 1.15  2003/06/26 03:19:53  orlando
* add ifCfgParam_connType enumeration.
*
* Revision 1.14  2003/06/23 10:57:23  elvis
* change include path of  rtl_types.h
*
* Revision 1.13  2003/06/20 12:59:50  tony
* add dhcp client
*
* Revision 1.12  2003/06/16 08:08:30  tony
* add dhcps & dns function
*
* Revision 1.11  2003/06/11 11:39:29  tony
* add WAN IP connection Type (static, PPPoE, DHCP)
*
* Revision 1.10  2003/06/06 11:57:06  orlando
* add pppoe cfg.
*
* Revision 1.9  2003/06/06 06:31:45  idayang
* add mgmt table in cfgmgr.
*
* Revision 1.8  2003/06/03 10:56:57  orlando
* add nat table in cfgmgr.
*
* Revision 1.7  2003/05/20 08:45:03  elvis
* change the include path of "rtl_types.h"
*
* Revision 1.6  2003/05/09 12:22:20  kckao
* remove RTL8650_LOAD_COUNT
*
* Revision 1.5  2003/05/02 08:51:45  orlando
* merge cfgmgr with old board.c/board.h.
*
* Revision 1.4  2003/04/29 14:20:19  orlando
* modified for cfgmgr.
*
* Revision 1.3  2003/04/08 15:29:47  elvis
* add interface ,route and static arp configurations
*
* Revision 1.2  2003/04/04 02:28:48  elvis
* add constant definition for PHY0, ..., PHY4
*
* Revision 1.1  2003/04/03 01:52:37  danwu
* init
*
* ---------------------------------------------------------------
*/
#ifndef _BOARD_H_
#define _BOARD_H_

#if defined(__KERNEL__) && defined(__linux__)
	/* kernel mode */
	#include "rtl865x/rtl_types.h"
	#include "rtl865x/asicRegs.h"
	#define IS_865XB() ( ( REG32( CRMR ) & 0xffff ) == 0x5788 )
	#define IS_865XA() ( !IS_865XB() )
	#define FLASH_BASE          (IS_865XA()?0xBFC00000:0xBE000000)
#else
	/* goahead */
	#include <rtl_types.h>
	#include <linux/config.h>
	#include <asicRegs.h>
	#ifdef __uClinux__
	/* MMU */
	#define IS_865XB() ( ( REG32( CRMR ) & 0xffff ) == 0x5788 )
	#define IS_865XA() ( !IS_865XB() )
	#define FLASH_BASE          (IS_865XA()?0xBFC00000:0xBE000000)
	#else
	/* no MMU */
	#define IS_865XB() ( 1 )
	#define IS_865XA() ( 0 )
	extern uint32 __flash_base;
	#define FLASH_BASE          __flash_base
	int rtl865x_mmap(int base,int length);
	#endif
#endif

#define _SUPPORT_LARGE_FLASH_ /* to support single 8MB/16MB flash */


/* Define flash device
*/

#define VLAN_LOAD_COUNT		8
#define MAX_PORT_NUM		9
#define MAX_IP_INTERFACE	VLAN_LOAD_COUNT
#define MAX_ROUTE			MAX_IP_INTERFACE
#define MAX_STATIC_ARP		10
#define MAX_MGMT_USER		2
#define MAX_ACL				8
#define MAX_SERVER_PORT	    20
#define MAX_SPECIAL_AP		8
#define MAX_PPPOE_SESSION	2
#define MAX_WLAN_CARDS		2
#define MAX_URL_FILTER		8
#define MAX_WLAN_AC		    8
#define MAX_WLAN_WDS		8
#define	MAX_IP_STRING_LEN	48
#define MAX_ROUTING	5
#define MAX_QOS				13
#define MAX_RATIO_QOS		10
#define MAX_PBNAT			4
#define ROMECFGFILE         "/var/romecfg.txt"

#define PHY0	            0x00000001
#define PHY1	            0x00000002
#define PHY2	            0x00000004
#define PHY3	            0x00000008
#define PHY4	            0x00000010
#define PHYMII		0x00000020
#define MAX_PHYS	        5

/* add routing functions  constant*/
#define error_msg(fmt,args...)
#define ENOSUPP(fmt,args...)
#define RTACTION_ADD   1
#define RTACTION_DEL   2
#define RTACTION_HELP  3
#define RTACTION_FLUSH 4
#define RTACTION_SHOW  5
#define E_LOOKUP	-1
#define E_OPTERR	-2
#define E_SOCK	-3
#define mask_in_addr(x) (((struct sockaddr_in *)&((x).rt_genmask))->sin_addr.s_addr)
#define full_mask(x) (x)

#define SHM_PROMECFGPARAM 0x1000

typedef struct vlanCfgParam_s
{
	uint32          vid;        /* VLAN index */
	uint32          memberPort; /* member port list */
	uint32	   enablePorts; /*Enabled Port Numbers*/
	uint32          mtu;        /* layer 3 mtu */
	uint32          valid;      /* valid */
	ether_addr_t	gmac;	    /* clone gateway mac address */
	uint8           rsv[2];     /* for 4 byte alignment */
} vlanCfgParam_t;


enum ifCfgParam_connType_e {
	IFCFGPARAM_CONNTYPE_STATIC = 0,
	IFCFGPARAM_CONNTYPE_PPPOE,
	IFCFGPARAM_CONNTYPE_DHCPC,
	IFCFGPARAM_CONNTYPE_PPPOE_UNNUMBERED,
	IFCFGPARAM_CONNTYPE_PPPOE_MULTIPLE_PPPOE,
	IFCFGPARAM_CONNTYPE_PPTP,	//pptp=5
	IFCFGPARAM_CONNTYPE_L2TP,
	IFCFGPARAM_CONNTYPE_DHCPL2TP,
	IFCFGPARAM_CONNTYPE_BIGPOND
};

typedef struct ifCfgParam_s	/* Interface Configuration */
{
	uint32          if_unit;	/* interface unit; 0, 1(vl0, vl1)... */
	uint8           ipAddr[4];	/* ip address */
	uint8           ipMask[4];	/* network mask */
	uint8           gwAddr[4];  /* default gateway address */
	uint8           dnsPrimaryAddr[4]; /* dns address */
	uint8           dnsSecondaryAddr[4]; /* dns address */
	uint32          flags;	    /* flags for interface attributes */
	uint32          valid;	    /* valid */
	uint32		    connType;	/* connection type: 0->static ip, 1->pppoe, 2->dhcp */
	uint8			mac[6];		/* Lan Mac -- Min Lee */
	uint16		mtu;			/* wan fix mtu ---- Eddie chang*/
} ifCfgParam_t;



enum pptpCfgParam_dialState_e {
	PPTPCFGPARAM_DIALSTATE_OFF=0,
	PPTPCFGPARAM_DIALSTATE_DIALED_WAITING,
	PPTPCFGPARAM_DIALSTATE_DIALED_TRYING,
	PPTPCFGPARAM_DIALSTATE_DIALED_SUCCESS,
	PPTPCFGPARAM_DIALSTATE_DISCONNECTING
};

enum l2tpCfgParam_dialState_e {
	L2TPCFGPARAM_DIALSTATE_OFF=0,
	L2TPCFGPARAM_DIALSTATE_DIALED_WAITING,
	L2TPCFGPARAM_DIALSTATE_DIALED_TRYING,
	L2TPCFGPARAM_DIALSTATE_DIALED_SUCCESS,
	L2TPCFGPARAM_DIALSTATE_DISCONNECTING,
	L2TPCFGPARAM_DIALSTATE_DIALED_DHCP_DISCOVER,
};

enum bigpondCfgParam_dialState_e {
	BIGPONDCFGPARAM_DIALSTATE_OFF=0,
	BIGPONDCFGPARAM_DIALSTATE_DIALED_WAITING,
	BIGPONDCFGPARAM_DIALSTATE_DIALED_TRYING,	
	BIGPONDCFGPARAM_DIALSTATE_DIALED_SUCCESS	
};

typedef struct pptpCfgParam_s	/* PPTP Configuration */
{
	uint8	ipAddr[4];	/* ip address */
	uint8	ipMask[4];	/* network mask */
	uint8	svAddr[4];  /* pptp server ip address */
	int8		username[64];  /* pppoe server username */
	int8		password[32];  /* pppoe server password */
	uint32	mtu;

	uint8			manualHangup;        /* only meaningful for ram version, to record a manual hangup event from web ui */
	uint8			autoReconnect; /* 1/0 */
	uint8			demand;		   /* 1/0 */
	uint8			dialState;	   /*  only meaningful for ram version, dial state */

	uint32			silentTimeout; /* in seconds */
	uint8    ipgw[4];            /* Default gateway */

} pptpCfgParam_t;

typedef struct l2tpCfgParam_s	/* L2TP Configuration */
{
	uint8	ipAddr[4];	/* ip address */
	uint8	ipMask[4];	/* network mask */
	uint8	svAddr[4];  /* l2tp server ip address */
	int8		username[64];  /* pppoe server username */
	int8		password[32];  /* pppoe server password */
	uint32	mtu;

	uint8			manualHangup;        /* only meaningful for ram version, to record a manual hangup event from web ui */
	uint8			autoReconnect; /* 1/0 */
	uint8			demand;		   /* 1/0 */
	uint8			dialState;	   /*  only meaningful for ram version, dial state */

	uint32			silentTimeout; /* in seconds */
	uint8    ipgw[4];            /* Default gateway */

} l2tpCfgParam_t;

typedef struct bigpondCfgParam_s	/* bigpond Configuration */
{
	int8    name[64];
    int8    password[32];
    int8    srv_name[64];
    uint32  server_ip;  
    uint8	dialState;	   /*  only meaningful for ram version, dial state */ 
} bigpondCfgParam_t;

typedef struct routeCfgParam_s	/* Route Configuration */
{
	uint32          if_unit;	  /* interface unit; 0, 1(vl0, vl1)... */
	uint8           ipAddr[4];	  /* ip network address */
	uint8           ipMask[4];	  /* ip network mask */
	uint8           ipGateway[4]; /* gateway address */
	uint32          valid; 		  /* valid */
} routeCfgParam_t;

typedef struct arpCfgParam_s /* Static ARP Configuration */
{
	uint32          if_unit;	  /* interface unit; 0, 1(vl0, vl1)... */
	uint8           ipAddr[4];	  /* ip host address */
	uint32          port;
	uint32          valid;
	ether_addr_t	mac;
	uint8           rsv[2];       /* for 4 byte alignment */
} arpCfgParam_t;

typedef struct natCfgParam_s /* nat Configuration */
{
	uint8			enable;			/* 0: disable, 1: enable */
	uint8			hwaccel;			/* 1:hardware acceleration,0:no hw accel */
	uint8			ipsecPassthru;	/* 1: passthru, 0: no passthru */
	uint8			pptpPassthru;	/* 1: passthru, 0: no passthru */
	uint8			l2tpPassthru;	/* 1: passthru, 0: no passthru */
	int8			mac_filter;	/* 0: disable, 1: enable */
	uint8			rsv[3];			/* for 4 byte alignment */
} natCfgParam_t;

enum ppppoeCfgParam_dialState_e {
	PPPOECFGPARAM_DIALSTATE_OFF=0,
	PPPOECFGPARAM_DIALSTATE_DIALED_WAITING,
	PPPOECFGPARAM_DIALSTATE_DIALED_TRYING,
	PPPOECFGPARAM_DIALSTATE_DIALED_SUCCESS
};

enum ppppoeCfgParam_destnetType_e {
	PPPOECFGPARAM_DESTNETTYPE_NONE=0,
	PPPOECFGPARAM_DESTNETTYPE_IP,
	PPPOECFGPARAM_DESTNETTYPE_DOMAIN,
	PPPOECFGPARAM_DESTNETTYPE_TCPPORT,
	PPPOECFGPARAM_DESTNETTYPE_UDPPORT,
};

enum ppppoeCfgParam_lanType_e {
	PPPOECFGPARAM_LANTYPE_NAPT=0,
	PPPOECFGPARAM_LANTYPE_UNNUMBERED,
};

typedef struct pppoeCfgParam_s
{
	uint8			enable;        /* 1:used/enabled, 0:no pppoe */
	uint8			defaultSession;   	/* default pppoe session  */
	uint8			unnumbered;		/* unnumbered pppoe */
	uint8			autoReconnect; /* 1/0 */

	uint8			demand;		   /* 1/0 */
	uint8			dialState;	   /*  only meaningful for ram version, dial state */
	uint16			sessionId;	   /* pppoe session id */

	uint32			silentTimeout; /* in seconds */

	uint8           ipAddr[4];	   /* ip address, for multiple pppoe only */
	uint8           ipMask[4];	   /* network mask, for multiple pppoe only */
	uint8           gwAddr[4];     /* default gateway address, for multiple pppoe only */
	uint8           dnsAddr[4];    /* dns address, for multiple pppoe only */

	uint8           localHostIp[4]; /* nat's local host ip, for pppoe/nat case only */

	int8			username[64];  /* pppoe server username */

	int8			password[32];  /* pppoe server password */

	int8 			acName[32];

	int8			serviceName[32];  /* pppoe service name */

	int8			destnet[4][32];  /* pppoe destination network route 1~4 */

	int8			destnetType[4];  /* pppoe destination network type 1~4 */

	uint8			unnumberedIpAddr[4];  /* global IP assign by ISP */

	uint8			unnumberedIpMask[4];  /* global Network mask assign by ISP */

	uint8           svrMac[6];     /* pppoe server mac address */
    uint8           pppx; /* i am pppx (0,1,2,3,...: ppp0,ppp1,ppp2,...) */
    uint8           unnumberedNapt;

	uint8           whichPppObjId; /* only meaningful for ram version, and for pppoeCfgParam[0] only, to keep track of who is the up/down pppObjId */
	uint8           manualHangup; /* only meaningful for ram version, to record a manual hangup event from web ui */
	uint16	mtu;

	uint32	lanType;

} pppoeCfgParam_t;

typedef struct dhcpsCfgParam_s
{
	uint8           enable;     /* 1:used/enabled, 0:disable */
	uint8           startIp;    /* DHCP Server IP Pool Start IP */
	uint8           endIp;      /* DHCP Server IP Pool End IP */
	uint8	   rsv;     	/* for 4 byte alignment */
	uint32	   manualIp[8];
	uint8	   hardwareAddr[8][6];
} dhcpsCfgParam_t;

typedef struct dhcpcCfgParam_s
{
	uint32      cmacValid; 	    /* cmac valid */
	uint8		cloneMac[6];	/* clone mac address */
	uint8		host_name[62];	/* clone mac address */
	uint8       rsv[2]; 		    /* for alignment */
} dhcpcCfgParam_t;

typedef struct dnsCfgParam_s
{
	uint32          enable;  /* 1:used/enabled, 0:disable */
	uint8		    primary[4];  /* primary dns server ip */
	uint8		    secondary[4]; /* secondary dns server ip */
} dnsCfgParam_t;

typedef struct mgmtCfgParam_s
{
	uint8           name[16];
	uint8           password[16];
	uint32          valid; 		/* valid */
	uint8			remoteMgmtEnable;	/* 0: disable, 1: enable */
	uint32			remoteMgmtIp;
	uint32			remoteMgmtMask; /* reserved */
	uint16			remoteMgmtPort; /* remote management port */
	uint8			remoteIcmpEnable; /* remote icmp enable */
	uint8			multicastEnable; /* Multicast Streams 0: disable, 1: enable */
	uint8			rsv;
} mgmtCfgParam_t;

enum aclGlobalCfgParam_e {
	ACLGLOBALCFGPARAM_ALLOWALLBUTACL = 0,
	ACLGLOBALCFGPARAM_DENYALLBUTACL,
	ACLGLOBALCFGPARAM_ALLOWALLBUTACL_LOG,
};

typedef struct aclGlobalCfgParam_s
{
	uint8		policy;				// 0: allow all except ACL , 1: deny all except ACL
	uint8		lanPermit;			// for sercomm spec
	uint8		rsv[2];				// for 4 bytes alignment
} aclGlobalCfgParam_t;

enum aclCfgParam_type_e {
	ACLCFGPARAM_PROTOCOL_TCP = 0,
	ACLCFGPARAM_PROTOCOL_UDP,
	ACLCFGPARAM_PROTOCOL_IP,
	// Dino Chang add 2004/11/08
	ACLCFGPARAM_PROTOCOL_ICMP,
	ACLCFGPARAM_PROTOCOL_ANY
	// Dino Chang
};

enum aclCfgParam_direction_e {
	ACLCFGPARAM_DIRECTION_EGRESS_DEST = 0,
	ACLCFGPARAM_DIRECTION_INGRESS_DEST,
	ACLCFGPARAM_DIRECTION_EGRESS_SRC,
	ACLCFGPARAM_DIRECTION_INGRESS_SRC
};

typedef struct aclCfgParam_s
{
	uint8		ip[4];		// IP address
	uint16		port;		// IP port
	uint8		enable;		// 0: disable,  1:enable
	uint8		direction;	// 0: egress dest, 1: ingress dest, 2: egress src, 3: ingress src
	uint8		type;		// 0: TCP, 1: UDP
	uint8		rsv[3];		// for 4 bytes alignment
        // Dino Chang add 2004/11/08
        uint8           name[33];
        uint8           src_end_ip[4];
        uint8           dst_start_ip[4];
        uint8           dst_end_ip[4];
        uint16          end_port;
        uint8           allow;
        int8            src_iface;
        int8            dst_iface;
        int8            apptime;
        int8            start_day;
        int8            end_day;
        uint16          start_time;
        uint16          end_time;
	uint8		filter;
        // Dino Chang
} aclCfgParam_t;


enum serverpCfgParam_e {
	SERVERPCFGPARAM_PROTOCOL_TCP = 0,
	SERVERPCFGPARAM_PROTOCOL_UDP,
	// Dino Chang 2005/01/10
	SERVERPCFGPARAM_PROTOCOL_BOTH,
	// Dino
};

typedef struct serverpCfgParam_s
{
	uint8        name[33];
	uint8		ip[4];		// Server IP address
	uint16		wanPortStart;	// WAN Port
	uint16		wanPortFinish;	// WAN Port
	uint16		portStart;		// Server Port
	uint8       protocol;	// SERVERPCFGPARAM_PROTOCOL_TCP or SERVERPCFGPARAM_PROTOCOL_UDP
	uint8		enable;		// enable
	/*	schedule -- Min Lee */
	int8        apptime;
    int8        start_day;
    int8        end_day;
    uint16  	start_time;
    uint16  	end_time;
} serverpCfgParam_t;

enum specialapCfgParam_e {
	SPECIALACPCFGPARAM_PROTOCOL_TCP = 0,
	SPECIALACPCFGPARAM_PROTOCOL_UDP,
	SPECIALACPCFGPARAM_PROTOCOL_BOTH
};

#define SPECIAL_AP_MAX_IN_RANGE 80
typedef struct specialapCfgParam_s
{
	uint8		name[16];		// name
	uint32		inType;			// incoming type
	uint8		inRange[SPECIAL_AP_MAX_IN_RANGE];	// incoming port range
	uint32		outType;		// outgoing type
	uint16		outStart;		// outgoing start port
	uint16		outFinish;		// outgoing finish port
	uint8		enable;			// enable
	uint8		rsv[3];			// for 4 bytes alignment
} specialapCfgParam_t;

typedef struct tbldrvCfgParam_s
{
	uint8		naptAutoAdd;		// napt auto add
	uint8       naptAutoDelete;     // napt auto delete
	uint16		naptIcmpTimeout;	// napt icmp timeout
	uint16      naptUdpTimeout;
	uint16		naptTcpLongTimeout;
	uint16      naptTcpMediumTimeout;
	uint16		naptTcpFastTimeout;
} tbldrvCfgParam_t;

typedef struct dmzCfgParam_s	/* DMZ Configuration */
{
	uint32		enable; 	/* 0:off 1:on */
	uint32		dmzIp;

} dmzCfgParam_t;

typedef struct logCfgParam_s	/* Event Log Configuration */
{
	uint32		module;  /* each bit mapping to each module */
	char            smtp_srv[31];    //eddie for log send email
	char            smtp_sendto[100];     //eddie for log send email
} logCfgParam_t;

typedef struct upnpCfgParam_s	/* UPnP Configuration */
{
	uint32		enable; 	/* 0:off 1:on */

} upnpCfgParam_t;

typedef struct ddnsCfgParam_s	/* DDNS Configuration */
{
	uint32		enable; 	/* 0:off 1:on */
	uint8		username[32];
	uint8		password[32];
	uint8		Rhostname[64];	//Added by Nick HO
	uint8		ddns_type;	//0=dyndns, 1=easydns, 2=noip


} ddnsCfgParam_t;

typedef struct urlfilterCfgParam_s	/* URL FILTER Configuration */
{
	uint32		enable; 	/* 0:off 1:on */
	uint8		string[32];
	uint32		ip_start;
	uint32		ip_end;
} urlfilterCfgParam_t;

/* added Mac Filter structure by Min Lee */
typedef struct macfilterCfgParam_s	/* MAC FILTER Configuration */
{
	uint32		allow; 	/* 0:deny, 1:allow*/
	uint8		name[80];
	uint8		mac[6];
} macfilterCfgParam_t;

#define DAYLIGHT_SAVING 1
typedef struct timeCfgParam_s	/* Time Configuration */
{
	uint8		time_type;		// 0- auto NTP, 1-Manual
	uint32		timeZoneIndex;
	uint8		timeZone[16];
	uint8		ntpServer1[32];
	uint8		ntpServer2[32];
	uint8		ntpServer3[32];

    uint16   	months;	//savior 20041216 for manuel set time
    uint16   	days;
    uint16   	hours;
    uint16   	minutes;
    uint16		years;
#ifdef DAYLIGHT_SAVING    
    uint16		daylight;	// enable or disable
    uint8   	daylight_start_m;	// start month
    uint8  		daylight_start_w;	// start week
    uint8   	daylight_start_d;	// start day
    uint8		daylight_start_h;	// start hour
    uint8		daylight_start_min;	// start minute
    uint8   	daylight_end_m;	// end month
    uint8		daylight_end_w;	// end week
    uint8   	daylight_end_d;	// end day
    uint8		daylight_end_h;	// end hour
    uint8		daylight_end_min; // end minute
#endif
} timeCfgParam_t;

typedef struct dosCfgParam_s	/* DoS Configuration */
{
	uint32		enable;
	uint32		enableItem;
	uint8		ignoreLanSideCheck;
	/* <---Whole System counter---> */
	uint16		syn;
	uint16		fin;
	uint16		udp;
	uint16		icmp;
	/* <---Per source IP tracking---> */
	uint16		perSrc_syn;
	uint16		perSrc_fin;
	uint16		perSrc_udp;
	uint16		perSrc_icmp;
	/* <----Source IP blocking----> */
	uint16		sipblock_enable;
	uint16		sipblock_PrisonTime;
	/* <----Port Scan Level ----> */
	uint16		portScan_level;
} dosCfgParam_t;

typedef struct naptCfgParam_s	/* NAPT Option Configuration */
{
	uint8		weakNaptEnable;						/* Weak TCP mode */
	uint8 		looseUdpEnable;	/*Loose UDP mode*/
	uint8		tcpStaticPortMappingEnable;				/* TCP static port mapping mode */
	uint8		udpStaticPortMappingEnable;				/* UDP static port mapping mode */
} naptCfgParam_t;

typedef struct algCfgParam_s	/* ALG Configuration */
{
	uint32		algList;
	ipaddr_t		localIp[32];
} algCfgParam_t;


extern int totalWlanCards;
 typedef enum{
	CRYPT_NONE=0,
	CRYPT_WEP64,
	CRYPT_WEP128,
	CRYPT_TKIP,
	CRYPT_AES
}WlanCryptType;

 typedef enum{
 	AUTH_NONE=0,
	AUTH_KEY,
	AUTH_8021x,
}WlanAuthType;


enum wlanDataRateModeCfgParam_e {
	WLAN_DATA_RATE_BG =0,
	WLAN_DATA_RATE_G,
	WLAN_DATA_RATE_A,
	WLAN_DATA_RATE_B
};


enum wlanDataRateCfgParam_e {
	WLAN_DATA_RATE_AUTO = 108,
	WLAN_DATA_RATE_1M = 1,
    WLAN_DATA_RATE_2M = 2,
    WLAN_DATA_RATE_5_5M = 5,
	WLAN_DATA_RATE_6M = 6,
	WLAN_DATA_RATE_9M = 9,
	WLAN_DATA_RATE_11M = 11,
	WLAN_DATA_RATE_12M = 12,
	WLAN_DATA_RATE_18M = 18,
	WLAN_DATA_RATE_24M = 24,
	WLAN_DATA_RATE_36M = 36,
	WLAN_DATA_RATE_48M = 48,
	WLAN_DATA_RATE_54M = 54,
};

enum wlanPreambleTypeCfgParam_e {
	WLAN_PREAMBLE_TYPE_LONG = 0,
	WLAN_PREAMBLE_TYPE_SHORT = 0,
};


typedef struct wlanCfgParam_s	/* Wireless LAN Configuration */
{
	uint8	aliasName[32];
	uint8	ssid[33];
	uint8   superG;
	uint8   turboG;
	uint8   ssidBC;
	uint8	enable;
	uint8	ap_mode;	//0: ap, 1: ap client -- Min Lee
	uint8	channel; //0:auto
	uint8	dataRateMode; //enum  //
	uint8   rsv;

	uint16	dataRate; //enum	//
	uint16	frag;
	uint16	cts;
	uint16	rts;
	uint16	beacon;
	uint16	dtim;
	uint8	preambleType; //enum
	uint8	broadcastSSID;
	uint8   txPower;
	uint8	iapp;

	uint8	cryptType; //enum
	uint8	authType;	// 1:Open System, 2: Shared Key, 3: WPA-PSK
	uint32	authRadiusPort;
	uint32	authRadiusIp;
	uint8	authRadiusPasswd[16];
	uint8	acMacList[MAX_WLAN_AC][6];
	uint8	acComment[MAX_WLAN_AC][16];
	uint8	wdsMacList[MAX_WLAN_WDS][6];
	uint8	wdsComment[MAX_WLAN_WDS][16];
	uint8	acEnable;

	uint8	wdsEnable; //for 4 bytes alignment
	uint8   rsv2[2];
	uint8	key[68]; //10 hex for wep64,26 hex for wep128, 64 hex for WPA-PSK
	/* add by Min Lee */
	uint8	wep_type;			// 0:disable, 1:enable
	uint8 	wep_key_type;		// 0:Hex, 1:ASCII
	uint8	wep_key_len;		// 1:64Bits, 2:128Bits
	uint8	wep_def_key;		// 0:key1, 1:key2, 2:key3, 3:key4
	uint8	key64[4][5];		// 64Bits -- Min Lee
	uint8	key128[4][13];		// 128Bits -- Min Lee
	uint8	passphrase[64];		// Passphrase for WPA-PSK
	
	uint8	cipher;				// 0-TKIP , 1-AES
	/*Radius Server */
    uint32	rad_srv_ip[2];
    uint16	rad_srv_port[2];
    uint8	rad_srv_secret[2];

} wlanCfgParam_t;

typedef struct udpblockCfgParam_s	/* UDP Blocking Configuration */
{
	uint32		enable;
	uint32		size;
} udpblockCfgParam_t;

enum routingInterfaceCfgParam_e {
	ROUTING_INTERFACE_NONE = 0,
	ROUTING_INTERFACE_LAN,
	ROUTING_INTERFACE_WAN
};

typedef struct routingCfgParam_s	/* Routing Table Configuration */
{
	uint32		route;
	uint32		mask;
	uint32		gateway;
	uint8		interface;
} routingCfgParam_t;

typedef struct qosCfgParam_s /* QoS Configuration */
{
	uint32		qosType;
	union {
		struct {
			uint16	portNumber;
			uint16	isHigh;
		} policy_based;
		struct {
			uint16	port;
			uint8	isHigh;
			uint8	enableFlowCtrl;
			uint32	inRL;
			uint32	outRL;
		} port_based;
	} un;
} qosCfgParam_t;

typedef struct ratioQosUpstreamCfgParam_s /* Ratio based QoS: Upstream total bandwidth Configuration */
{
	uint32	maxRate;
	uint8	maxRateUnit;		/* 0: Mbps, 1: Kbps */
	uint8	remainingRatio_h;	/* remaining ratio in high queue */
	uint8	remainingRatio_l;	/* remaining ratio in low queue */
} ratioQosUpstreamCfgParam_t;

typedef struct ratioQosEntry_s	/* Ratio based QoS entry */
{
	uint8	enable;		/* 0: disable, 1: enable */
	uint8	isSrc;		/* 0: dst, 1: src */
	uint32	ip;
	uint32	ipMask;
	uint8	protoType;	/* 0: TCP, 1: UDP, 2: IP */
	uint16	s_port;
	uint16	e_port;
	uint8	isHigh;		/* 0: Low 1: High */
	uint8	ratio;		/* percentage */
} ratioQosEntry_t;

typedef struct ratioQosCfgParam_s	/* Ratio based QoS Configuration */
{
	uint8 enable;							/* 0: disable, 1: enable */
	ratioQosUpstreamCfgParam_t upInfo;
	ratioQosEntry_t entry[MAX_RATIO_QOS];
} ratioQosCfgParam_t;

typedef struct pbnatCfgParam_s /* Protocol-Based NAT Configuration */
{
	uint8 protocol;
	uint8 enabled;
	ipaddr_t IntIp;
} pbnatCfgParam_t;

#define NULL_QOS						0x00
#define PORT_BASED_QOS				0x01
#define POLICY_BASED_QOS				0x02
#define ENABLE_QOS					0x04


typedef struct rateLimitEntry_s
{
	uint8		enable;		/* 0: disable, 1: enable */
	uint32		ip;
	uint32		ipMask;
	uint16		s_port, e_port;
	uint8		protoType;	/* 0: TCP, 1: UDP, 2: IP */
	uint8		isByteCount;	/* 0: Packet Count, 1: Byte Count */
	uint32		rate;
	uint8		rateUnit;		/* 0: Kbps, 1: Mbps */
	uint32		maxRate;		/* Tolerance */
	uint8		maxRateUnit;	/* 0: Kbps, 1: Mbps */
	uint8		isDropLog;	/* 1: Drop & Log, 0: Drop */
	uint8		isSrcIp;
} rateLimitEntry_t;

typedef struct rateLimitCfgParam_s
{
	uint32		enable;		/* 0: disable, 1: enable */
	rateLimitEntry_t entry[16];
} rateLimitCfgParam_t;

// Dino Chang 2005/09/20
#if defined(TMSS_FUNC)
#define PC_USER_NAME_LEN_S        	32
#define PC_PASS_LEN_S        	  	16
#define PC_USER_LEN_S				9
#define PC_CATA_LEN_S				52
#define PC_SCHED_LEN_S				10

typedef struct pc_schedule_s {
	uint16	pc_start_day;
	uint16	pc_start_time;
	uint16	pc_end_day;
	uint16	pc_end_time;
	uint8   apptime;	
} pc_schedule_t;

typedef struct pc_control_s {
	uint8			general_control;
	uint8			pc_cat[PC_CATA_LEN_S];
	//uint8 			more;			// 0: No, 1: Yes	
	uint8			schedule_cnt;
	pc_schedule_t   	schedule[PC_SCHED_LEN_S];
} pc_control_t;

typedef struct pc_user_s {
	int8 			user_name[PC_USER_NAME_LEN_S+1];
	int8			user_pass[PC_PASS_LEN_S+1];
	uint8			status;			// 0: disable, 1: enable
	pc_control_t    	user_control;		
} pc_user_t;

typedef struct tmssCInfoCfgParam_s {
	uint8	mac[6];
	uint32	license;
	uint8	report;
	uint8	filter;
} tmssCInfoCfgParam_t;

typedef struct tmssCfgParam_s
{	
	uint8			tmss_enable;	// 0: disable, 1: enable
	uint8			show_web_time;

	uint8			auto_update;	// 0: disable, 1: enable
	uint8			check_update_time;

	// Dino Chang 2005/01/18
	uint8	license_enable;
	// Dino
	
	// parental control
	uint8			pc_enable;				// 0: disable, 1: enable
	uint8			pc_control;				// 0: Use General Control, 1: Enforce Per-User Controls
	int8			parent_pass[PC_PASS_LEN_S];	// Parents Overridden Password
	pc_control_t    	general;
	uint8			pc_user_cnt;
	uint16			user_idle_timeout;		// 0: off, 900: 15mins, 1800: 30mins, 2700: 45mins, 3600: 60mins
	pc_user_t		pc_user[PC_USER_LEN_S];
}tmssCfgParam_t;
#endif // TMSS_FUNC
// Dino

typedef struct romeCfgParam_s
{
	vlanCfgParam_t      vlanCfgParam[VLAN_LOAD_COUNT];
	ifCfgParam_t        ifCfgParam[MAX_IP_INTERFACE];
	routeCfgParam_t     routeCfgParam[MAX_ROUTE];
	arpCfgParam_t       arpCfgParam[MAX_STATIC_ARP];
	natCfgParam_t       natCfgParam;
	pppoeCfgParam_t     pppoeCfgParam[MAX_PPPOE_SESSION];
	dhcpsCfgParam_t     dhcpsCfgParam;
	dhcpcCfgParam_t     dhcpcCfgParam;
	dnsCfgParam_t       dnsCfgParam;
	mgmtCfgParam_t      mgmtCfgParam[MAX_MGMT_USER];
	aclGlobalCfgParam_t	aclGlobalCfgParam;
	aclCfgParam_t       aclCfgParam[MAX_PPPOE_SESSION][MAX_ACL];
	serverpCfgParam_t   serverpCfgParam[MAX_PPPOE_SESSION][MAX_SERVER_PORT];
	specialapCfgParam_t specialapCfgParam[MAX_PPPOE_SESSION][MAX_SPECIAL_AP];
    dmzCfgParam_t       dmzCfgParam[MAX_PPPOE_SESSION];
	logCfgParam_t       logCfgParam[MAX_PPPOE_SESSION];
    upnpCfgParam_t      upnpCfgParam;
    ddnsCfgParam_t      ddnsCfgParam;
	tbldrvCfgParam_t    tbldrvCfgParam;
	urlfilterCfgParam_t	urlfilterCfgParam[MAX_PPPOE_SESSION][MAX_URL_FILTER];
	macfilterCfgParam_t macfilterCfgParam[MAX_PPPOE_SESSION][MAX_URL_FILTER];
	// Dino Chang 2005/09/20
#if defined(TMSS_FUNC)
	tmssCfgParam_t		tmssCfgParam;
	tmssCInfoCfgParam_t	tmssCInfoCfgParam[PC_USER_LEN_S];
#endif
	// Dino
	dosCfgParam_t		dosCfgParam[MAX_PPPOE_SESSION];
	naptCfgParam_t		naptCfgParam;
	timeCfgParam_t		timeCfgParam;
	algCfgParam_t		algCfgParam[MAX_PPPOE_SESSION];
	wlanCfgParam_t		wlanCfgParam[MAX_WLAN_CARDS];
	udpblockCfgParam_t	udpblockCfgParam[MAX_PPPOE_SESSION];
	routingCfgParam_t	routingCfgParam[MAX_ROUTING];
	pptpCfgParam_t		pptpCfgParam;
	l2tpCfgParam_t		l2tpCfgParam;
	bigpondCfgParam_t   bigpondCfgParam;
	qosCfgParam_t		qosCfgParam[MAX_QOS];
	rateLimitCfgParam_t	rateLimitCfgParam;
	ratioQosCfgParam_t	ratioQosCfgParam;
	pbnatCfgParam_t		pbnatCfgParam[MAX_PPPOE_SESSION][MAX_PBNAT];
} romeCfgParam_t;


#endif /* _BOARD_H_ */
