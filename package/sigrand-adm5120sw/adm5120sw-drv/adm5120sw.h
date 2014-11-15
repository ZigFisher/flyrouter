/*
 *	ADM5120 ethernet switch driver
 *	
 *	Based on original ADMTEK 2.4.18 driver, copyright ADMtek Inc.
 *	daniel@admtek.com.tw
 *	
 *	Port to 2.4.31 kernel and modified to able to load as module
 *	by Joco, rjoco77@kezdionline.ro
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2, or (at your option)
 *	any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.	
 */

#ifndef __ADM5120SW_H__
#define __ADM5120SW_H__

#include <asm/am5120/adm5120.h>

#define ADM5120_DEBUG
//#define DEBUG_DEF 6

/********************* MII Phy Reg Access ********************/
#define PHY_ADDR_MASK			0x0000000F
#define PHY_REG_MASK			0x00001F00
#define PHY_WRITE_CMD			0x00002000
#define PHY_READ_CMD			0x00004000

#define PHY_REG_SHIFT			8

#define PHY_WRITE_OK			0x00000001
#define PHY_READ_OK			0x00000002
#define PHY_DATA_MASK			0xFFFF0000
#define PHY_DATA_SHIFT			16

#define MII_PHY_CTRL_REG		0
#define MII_PHY_STATUS_REG		1
#define MII_PHY_ID0_REG			2
#define MII_PHY_ID1_REG			3
#define MII_PHY_ANAR_REG		4
#define MII_PHY_ANLPAR_REG		5
#define MII_PHY_LINK_REG		0x10

#define MII_PHY_FORCE_LINK		0x03

/*************** VLAN *****************/
#define MAX_VLAN_GROUP			4
#define VLAN_PORT_MASK			0x7f

// Vlan status
#define VLAN_NOT_DEFINED		0
#define VLAN_DISABLED			1
#define VLAN_ENABLED			2

#define VLAN_NONE			0x00
#define VLAN0_ID			0x01
#define VLAN1_ID			0x02
#define VLAN2_ID			0x04
#define VLAN3_ID			0x08

/***************** Tx/Rx Desc **********************/
#define HWDESC_ALIGN			16

//Common
#define OWN_BIT				0x80000000
#define END_OF_RING			0x10000000
#define BUF_ADDR_MASK			0x01FFFFFF
#define BUF2_EN				0x80000000

#define BUF1_LEN_MASK			0x000007FF
#define PKT_LEN_MASK			0x07FF0000
#define PKT_LEN_SHIFT			16

// RxDesc Only
#define RX_SRC_PORT_MASK		0x00007000
#define RX_SRC_PORT_SHIFT		12

#define RX_FRAME_TYPE_MASK		0x00000030
#define RX_FRAME_UC			0
#define RX_FRAME_MC			0x00000010
#define RX_FRAME_BC			0x00000020

#define RX_PKT_IPSUM_ERR		0x00000008
#define RX_PKT_VLAN_TAG			0x00000004

#define RX_PKT_TYPE_MASK		0x00000003
#define RX_PKT_IP			0x00000000
#define RX_PKT_PPPOE			0x00000001

#define RX_PKT_FLAG_MASK		0x0000007F

// TxDesc Only
#define TX_ADD_IPSUM			0x80000000
#define TX_DEST_VLAN_MASK		0x0000003F
#define TX_DEST_PORT_MASK		0x00003F00
/******************* Interrupts *********************/
#define RX_H_INT			(RX_H_DONE_INT | RX_H_DESC_FULL_INT)
#define RX_L_INT			(RX_L_DONE_INT | RX_L_DESC_FULL_INT)
#define TX_H_INT			SEND_H_DONE_INT
#define TX_L_INT			SEND_L_DONE_INT
#define LINK_INT			PORT_STATUS_CHANGE_INT

#define PORT_INT			PORT_STATUS_CHANGE_INT
#define WATCHDOG0_INT			WATCHDOG0_EXPR_INT
#define WATCHDOG1_INT			WATCHDOG1_EXPR_INT

/******************* Switch ports *****************/
#define ETH_PORT_NUM			4
#define CPU_PORT			6
#define CPU_PORT_MASK			(0x1 << CPU_PORT)
#define GMII_PORT			5
#define IF5120_LAST_PORT		GMII_PORT
#define NUM_IF5120_PORTS		(IF5120_LAST_PORT+1)  //Excluding the cpu port
#define IF5120_PORT_MASK		0x3f
#define PORT_VLAN_MASK			0x3f
#define IF5120_PORT_NUM			7

/* Port status */
#define PORT_DISABLED			0
#define PORT_ENABLED			1

/************************ Switch config ***************************/
#define DEF_CPUPORT_CFG			(SW_CPU_PORT_DISABLE | SW_PADING_CRC | SW_DIS_UN_MASK)
#define DEF_PORTS_CFG			(SW_EN_BP_MASK | SW_EN_MC_MASK | SW_DISABLE_PORT_MASK)

/***********************************************************/
#define ETH_MAC_LEN			6
#define ETH_VLANTAG_LEN			4
#define ETH_CRC_LEN			4		// Ethernet CRC Length

#define TX_SMALL_BUF_SIZE		256
#define TX_LARGE_BUF_SIZE		1536
#define TX_BUF_SIZE			1536

#define RX_BUF_SIZE			1680
#define RX_BUF_REV_SIZE			130
#define DEF_RX_BUF_SIZE			(RX_BUF_SIZE - RX_BUF_REV_SIZE)

#define SMALL_PKT_LEN			TX_SMALL_BUF_SIZE
#define MAX_PKT_LEN			1514

#define MIN_ETH_FRAME_LEN		60

/****************** PSEUDO-NIC Control Flags ********************/
#define IF_TX_PRIORITY_H		0x0001
#define IF_PROMISC_MODE			0x0002
#define IF_ATTACH_BRIDGE		0x0010
#define IF_BRIDGE_HOSTIF		0x0020
#define IF_CTRL_FLAG_MASK		0x0033
#define IF_MAC_ALIAS			0x0100

/**************************************************************/
#define MEM_KSEG0_BASE			0x80000000
#define MEM_KSEG1_BASE			0xA0000000
#define MEM_SEG_MASK			0xE0000000
#define KVA2PA(_addr)			((unsigned long)(_addr) & ~MEM_SEG_MASK)

#define MIPS_KSEG0A(_addr)		(KVA2PA(_addr) | MEM_KSEG0_BASE)
#define MIPS_KSEG1A(_addr)		(KVA2PA(_addr) | MEM_KSEG1_BASE)

#define PA2VA(_addr)			(KVA2PA(_addr) | MEM_KSEG1_BASE)
#define PA2CACHEVA(_addr)		(KVA2PA(_addr) | MEM_KSEG0_BASE)

#define ADM5120SW_BOARD_CFG_ADDR	0x1FC08000
#define ADM5120SW_DEFAULT_MAC		"\x00\x11\x22\x33\x44\x55"

/* tx driver descriptor */
typedef struct adm5120sw_tx_drv_desc_s
{
	struct sk_buff *skb;
} TX_DRV_DESC_T, *PTX_DRV_DESC_T;

/* rx driver descriptor */
typedef struct adm5120sw_rx_drv_desc_s
{
	struct sk_buff *skb;
} RX_DRV_DESC_T, *PRX_DRV_DESC_T;

/* tx descriptor */
typedef struct adm5120sw_tx_desc_s
{
	unsigned long buf1cntl;
	unsigned long buf2cntl;
	unsigned long buf1len;
	unsigned long pktcntl;
} TXDESC_T, *PTXDESC_T;

/* rx descriptor */
typedef struct adm5120sw_rx_desc_s
{
	unsigned long buf1cntl;
	unsigned long buf2cntl;
	unsigned long buf1len;
	unsigned long status;
} RXDESC_T, *PRXDESC_T;

/* Rx engine */
typedef struct adm5120sw_rx_eng_s
{
	PRXDESC_T hwDesc;
	PRX_DRV_DESC_T drvDesc;
	int numDesc;
	int idx;
} RX_ENG_T, *PRX_ENG_T;

/* Tx engine */
typedef struct adm5120sw_tx_eng_s
{
	PTXDESC_T hwDesc;
	PTX_DRV_DESC_T drvDesc;
	int numDesc;
	int idxHead;
	int idxTail;
	unsigned long txTrig;
} TX_ENG_T, *PTX_ENG_T;

/* port status */
typedef struct port_status_s
{
	unsigned long status;
	unsigned long vlanId;
	unsigned long ifUnit;
} PORT_CFG_T, *PPORT_CFG_T;

typedef struct adm5120sw_context_s {
	TX_ENG_T txH, txL;
	RX_ENG_T rxH, rxL;
	
	PORT_CFG_T port[NUM_IF5120_PORTS];
	
	/* Vlan group of IF unit */
	unsigned char vlanGrp[MAX_VLAN_GROUP];
	unsigned char nr_if;
	
	unsigned long intMask;
	unsigned long intStatus;
	unsigned long linkStatus;
	
	int ifCnt;
	int actIfCnt;
	
	/* tx drv descriptor pool */
	unsigned char *txDrvDescPool;
	
	/* rx drv descriptor pool */
	unsigned char *rxDrvDescPool;
	
	/* Hardware descriptor pool */
	unsigned char *hwDescPool;
	
	spinlock_t lock;

} SW_CONTEXT_T, *PSW_CONTEXT_T;

/* net device private data */
typedef struct adm5120sw_priv_s
{
	struct net_device_stats stats;
	
	spinlock_t lock;
	
	/* interface unit */
	int unit;
	
	int status;
	int priority;
	int csum_flags;
	
	/* interface flags */
	int iflags;
	
	/* for fill tx descriptor */
	unsigned long vlanId;
	
} SW_PRIV_T, *PSW_PRIV_T;

#endif

