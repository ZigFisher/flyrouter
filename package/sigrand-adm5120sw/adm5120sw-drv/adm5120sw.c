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
 
	
	V1.0 Successful compile on 2.4.31 as module
	V1.1 Modify promisc mode to work on bridge mode
	V1.2 Added module param (vlan_mx) for modify vlan struct and ethernet
	     interfaces.
	     ex: vlan_mx="0x41,0x42,0x44,0x48,0x50,0x60"     -> 5 eth
	         vlan_mx="0x5E,0x41,0,0,0,0"		     -> 2 eth Edimax layout
	V1.3 Read MAC from Edimax type config partition
	V1.4 procfs improvements 
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h> 	
#include <linux/slab.h>		
#include <linux/gfp.h>		
#include <linux/errno.h>	
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/in6.h>

#include <asm/checksum.h>
//#define ADM5120_DEBUG
//#define DEBUG_DEF 1
#include <asm/am5120/debug.h>
#include <linux/delay.h>

#include "adm5120sw.h"

#define NUM_TX_H_DESC           24      /* Number of the Transmitting descriptor */
#define NUM_TX_L_DESC           128 /*128      Number of the Transmitting descriptor */
#define NUM_RX_H_DESC           24      /* Number of the Receiving descriptors */
#define NUM_RX_L_DESC           128    /* 128      Number of the Receiving descriptors */

#define SW_IRQ			9

int adm5120sw_init(struct net_device *dev);
static int SetupVLAN(int unit, unsigned long portmask);
int ProgramVlanMac(int vlan, char *Mac, int clear);
void EnableVlanGroup(int unit, unsigned long vlanGrp);
void DisableVlanGroup(int unit, unsigned long vlanGrp);


static int unit = 0;
static int timeout = 5;

PSW_CONTEXT_T sw_context;

static int vlan_mx[] = { 0x41,0x42,0x44,0x48,0x50,0x60 };

MODULE_DESCRIPTION("ADM5120 switch ethernet driver");
MODULE_AUTHOR("Joco (rjoco77@kezdionline.ro)");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");

struct net_device adm5120sw_devs[MAX_VLAN_GROUP] = {
    { name: "eth0", init:adm5120sw_init },
    { name: "eth1", init:adm5120sw_init },
    { name: "eth2", init:adm5120sw_init },
    { name: "eth3", init:adm5120sw_init },
//    { name: "eth4", init:adm5120sw_init },
//    { name: "eth5", init:adm5120sw_init }
};

int adm5120_get_nrif (char * vlan_matrix)
{
	int i,nr = 0;
	for ( i = 0; i < MAX_VLAN_GROUP; i++)
		if (vlan_matrix[i] & 0x40) nr++;
	return nr;
}


/* ------------------------------------------------------
		    Switch driver init    	
  ------------------------------------------------------*/

// InitTxDesc 
static void InitTxDesc(PTX_ENG_T pTxEng)
{
	int num = pTxEng->numDesc;
	
	pTxEng->hwDesc[--num].buf1cntl |= END_OF_RING;
	pTxEng->idxHead = pTxEng->idxTail = 0;
}

// InitRxDesc 
static void InitRxDesc(PRX_ENG_T pRxEng)
{
	PRX_DRV_DESC_T drvDesc = pRxEng->drvDesc;
	int i;

	for (i = 0; i < pRxEng->numDesc; i++, drvDesc++) {
		drvDesc->skb = dev_alloc_skb(DEF_RX_BUF_SIZE+16);
		if (!drvDesc->skb) {
			printk("Init rx skb : low on mem\n");
			return;
		}
		skb_reserve(drvDesc->skb, 2); /* align IP on 16B boundary */
	}

	drvDesc = pRxEng->drvDesc;

	for (i = 0; i < pRxEng->numDesc; i++) {
		pRxEng->hwDesc[i].buf2cntl = pRxEng->hwDesc[i].status = 0;
		pRxEng->hwDesc[i].buf1len = DEF_RX_BUF_SIZE;
		pRxEng->hwDesc[i].buf1cntl =
			((unsigned long)pRxEng->drvDesc[i].skb->data &
			 BUF_ADDR_MASK) | OWN_BIT;
	}

	pRxEng->hwDesc[--i].buf1cntl |= END_OF_RING;
	pRxEng->idx = 0;
}

// Switch driver init    	
int adm5120swdrv_init (void)
{
	int i;

	/* Allocate the switch driver context */
	if ((sw_context = (PSW_CONTEXT_T)kmalloc(sizeof(SW_CONTEXT_T), GFP_KERNEL)) == NULL)
		return (-1);

	memset((char *)sw_context, 0, sizeof(SW_CONTEXT_T));

	/* Allocate the Tx/Rx hardware descriptor pool */
	i = HWDESC_ALIGN + sizeof(RXDESC_T) * (NUM_RX_H_DESC + NUM_RX_L_DESC)
		+ sizeof(TXDESC_T) * (NUM_TX_H_DESC + NUM_TX_L_DESC);

	if ((sw_context->hwDescPool = (char *)kmalloc(i, GFP_KERNEL)) == NULL)
		goto ErrRes;

	memset(sw_context->hwDescPool, 0, i);

	/* Allocate the tx driver descriptor */
	i = sizeof(TX_DRV_DESC_T) * (NUM_TX_H_DESC + NUM_TX_L_DESC);

	if ((sw_context->txDrvDescPool = (char *)kmalloc(i, GFP_KERNEL)) == NULL)
		goto ErrRes;

	memset(sw_context->txDrvDescPool, 0, i);

	/* Allocate the rx driver descriptor */
	i = sizeof(RX_DRV_DESC_T) * (NUM_RX_H_DESC + NUM_RX_L_DESC);

	if ((sw_context->rxDrvDescPool = (char *)kmalloc(i, GFP_KERNEL)) == NULL)
		goto ErrRes;

	memset(sw_context->rxDrvDescPool, 0, i);

	/*
	 *!! Note: The Hardware Tx/Rx descriptors should be allocated at non-
	 cached memory and aligned 16 bytes boundry!!!!!
	 */

	/* assign hardware descriptor to txH pool */
	if (((unsigned long)sw_context->hwDescPool) & (HWDESC_ALIGN - 1)) {
		sw_context->txH.hwDesc = (PTXDESC_T)MIPS_KSEG1A((unsigned long)(sw_context->hwDescPool
					+ HWDESC_ALIGN - 1) & ~(HWDESC_ALIGN - 1));
	} else
		sw_context->txH.hwDesc = (PTXDESC_T)MIPS_KSEG1A(sw_context->hwDescPool);

	sw_context->txH.numDesc = NUM_TX_H_DESC;
	sw_context->txH.txTrig = SEND_TRIG_HIGH;
	InitTxDesc(&sw_context->txH);
	sw_context->txH.drvDesc = (PTX_DRV_DESC_T)sw_context->txDrvDescPool;

	/* assign hardware descriptor to txL pool */
	sw_context->txL.hwDesc = &sw_context->txH.hwDesc[NUM_TX_H_DESC];
	sw_context->txL.numDesc = NUM_TX_L_DESC;
	sw_context->txL.txTrig = SEND_TRIG_LOW;
	InitTxDesc(&sw_context->txL);
	sw_context->txL.drvDesc = (PTX_DRV_DESC_T)(sw_context->txDrvDescPool +
			(sizeof(TX_DRV_DESC_T) * NUM_TX_H_DESC));

	/* assign hardware descriptor to rxH pool */
	sw_context->rxH.hwDesc = (PRXDESC_T)&sw_context->txL.hwDesc[NUM_TX_L_DESC];
	sw_context->rxH.numDesc = NUM_RX_H_DESC;
	sw_context->rxH.drvDesc = (PRX_DRV_DESC_T)sw_context->rxDrvDescPool;
	InitRxDesc(&sw_context->rxH);

	/* assign hardware descriptor to rxL pool */
	sw_context->rxL.hwDesc = &sw_context->rxH.hwDesc[NUM_RX_H_DESC];
	sw_context->rxL.numDesc = NUM_RX_L_DESC;
	sw_context->rxL.drvDesc = (PRX_DRV_DESC_T)(sw_context->rxDrvDescPool +
			(sizeof(RX_DRV_DESC_T) * NUM_RX_H_DESC));
	InitRxDesc(&sw_context->rxL);

	for (i = 0; i < MAX_VLAN_GROUP; i++)
		sw_context->vlanGrp[i] = vlan_mx[i] & 0x7F;

	/* disable cpu port, CRC padding from cpu and no send unknown packet
	   from port0 to port5 to cpu */
	ADM5120_SW_REG(CPUp_conf_REG) = DEF_CPUPORT_CFG ;

	/* Disable all port, enable BP & MC */
	ADM5120_SW_REG(Port_conf0_REG) = DEF_PORTS_CFG;

	/* Wait until switch enter idle state */
	for (i = 0; i < 500000; i++);

	/* Put Phys to normal mode */
	ADM5120_SW_REG(PHY_cntl2_REG) |= SW_PHY_AN_MASK | SW_PHY_NORMAL_MASK;
	ADM5120_SW_REG(PHY_cntl3_REG) |= 0x400;

	/* Disable Switch Interrupts */
	ADM5120_SW_REG(SW_Int_mask_REG) = SWITCH_INT_MASK;

	/* Clear the Interrupt status */
	ADM5120_SW_REG(SW_Int_st_REG) = SWITCH_INT_MASK;

	/* Initialize the adm5120 Desc */
	ADM5120_SW_REG(Send_HBaddr_REG) = (unsigned long)sw_context->txH.hwDesc;
	ADM5120_SW_REG(Send_LBaddr_REG) = (unsigned long)sw_context->txL.hwDesc;
	ADM5120_SW_REG(Recv_HBaddr_REG) = (unsigned long)sw_context->rxH.hwDesc;
	ADM5120_SW_REG(Recv_LBaddr_REG) = (unsigned long)sw_context->rxL.hwDesc;

	/* Clear all vlan setting */
	ADM5120_SW_REG(VLAN_G1_REG) = 0;
	ADM5120_SW_REG(VLAN_G2_REG) = 0;

	/* Update link status */
	sw_context->linkStatus = 0;

	sw_context->intMask = RX_H_INT | RX_L_INT | TX_H_INT | TX_L_INT | PORT0_QUE_FULL_INT |
		PORT1_QUE_FULL_INT | PORT2_QUE_FULL_INT | PORT3_QUE_FULL_INT |
		PORT4_QUE_FULL_INT | PORT5_QUE_FULL_INT | CPU_QUE_FULL_INT |
		CPU_HOLD_INT | SEND_DESC_ERR_INT | RX_DESC_ERR_INT;

	ADM5120_SW_REG(CPUp_conf_REG) &= ~SW_CPU_PORT_DISABLE;
	spin_lock_init(&sw_context->lock);
	return (0);

ErrRes:
	/* Free all resources */
	if (sw_context->hwDescPool != NULL)
		kfree(sw_context->hwDescPool);
	if (sw_context->txDrvDescPool != NULL)
		kfree(sw_context->txDrvDescPool);
	if (sw_context->rxDrvDescPool != NULL);
	kfree(sw_context->rxDrvDescPool);
	kfree(sw_context);
	return (-1);	
}

/* ------------------------------------------------------
    			Interrupts
  ------------------------------------------------------*/

void ProcessRxInt(PRX_ENG_T rxEng)
{
	struct net_device *rdev;
	PSW_PRIV_T priv = 0;
	PRXDESC_T rxDesc = 0;
	PRX_DRV_DESC_T drvDesc = 0;
	int unit;
	int srcPort;
	int idx;
	int len;
	idx = rxEng->idx;
	rxDesc = &rxEng->hwDesc[idx];
	while (!(rxDesc->buf1cntl & OWN_BIT)) { 
		drvDesc = &rxEng->drvDesc[idx];
		if (drvDesc->skb == 0)
			goto get_desc;

		srcPort = (rxDesc->status & RX_SRC_PORT_MASK) >> RX_SRC_PORT_SHIFT;
		unit = sw_context->port[srcPort].ifUnit;
		rdev = &adm5120sw_devs[unit];
		priv = (PSW_PRIV_T)rdev->priv;

		len = ((rxDesc->status & PKT_LEN_MASK) >> PKT_LEN_SHIFT) - ETH_CRC_LEN;
		if (len <= 0 || (rxDesc->status & RX_PKT_IPSUM_ERR)) {
			priv->stats.rx_errors++;
			priv->stats.rx_length_errors++;
			goto next;
		}

		skb_put(drvDesc->skb, len);

		/* Write metadata, and then pass to the receive level */
		drvDesc->skb->dev = rdev;
		drvDesc->skb->protocol = eth_type_trans(drvDesc->skb, rdev);
		drvDesc->skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

		if (netif_rx(drvDesc->skb) == NET_RX_DROP) {
			priv->stats.rx_dropped++;
		} else {
			rdev->last_rx = jiffies;
			priv->stats.rx_packets++;
			priv->stats.rx_bytes += drvDesc->skb->len;
		}

get_desc:

		drvDesc->skb = dev_alloc_skb(DEF_RX_BUF_SIZE+16);

		if (drvDesc->skb) {
			skb_reserve(drvDesc->skb, 2); /* align IP on 16B boundary */
next:
			rxDesc->buf2cntl = rxDesc->status = 0;
			rxDesc->buf1len = DEF_RX_BUF_SIZE;
			rxDesc->buf1cntl = (rxDesc->buf1cntl & END_OF_RING) | OWN_BIT
				| (((unsigned long)drvDesc->skb->data)& BUF_ADDR_MASK);
		} else
			printk("Init rx skb : low on mem\n");

		if (++idx == rxEng->numDesc)
			idx = 0;

		rxDesc = &rxEng->hwDesc[idx];
	}
	rxEng->idx = idx;
	return;
}

void ProcessTxInt(PTX_ENG_T txEng)
{
	PTX_DRV_DESC_T drvDesc;
	int idx;
	idx = txEng->idxTail;

	while (!(txEng->hwDesc[idx].buf1cntl & OWN_BIT) && (idx != txEng->idxHead)) {
		drvDesc = &txEng->drvDesc[idx];
		dev_kfree_skb_irq(drvDesc->skb);

		drvDesc->skb = 0;

		if (++idx == txEng->numDesc)
			idx = 0;
	}

	txEng->idxTail = idx;
}

irqreturn_t swdrv_ProcessInt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long intReg;

	spin_lock(&sw_context->lock);


	/* disable switch interrupt */
	ADM5120_SW_REG(SW_Int_mask_REG) |= sw_context->intMask;

	/* recording the current interrupts in the context. */
	intReg = ADM5120_SW_REG(SW_Int_st_REG);

	/* acknowledge  all interrupts */
	ADM5120_SW_REG(SW_Int_st_REG ) = intReg;

	/* receive one high priority packet to cpu */
	if (intReg & RX_H_INT)
		ProcessRxInt(&sw_context->rxH);

	/* receive one normal priority packet to cpu */
	if (intReg & RX_L_INT)
		ProcessRxInt(&sw_context->rxL);

	/* transmit one high priority packet to cpu */
	if (intReg & TX_H_INT)
		ProcessTxInt(&sw_context->txH);

	/* transmit one normal priority packet from cpu */
	if (intReg & TX_L_INT)
		ProcessTxInt(&sw_context->txL);


	ADM5120_SW_REG(SW_Int_mask_REG) &= ~sw_context->intMask;

	spin_unlock(&sw_context->lock);		
	return IRQ_HANDLED;
}


/*------------------------------------------------------
    			PROC FS
  ------------------------------------------------------*/
#define MAX_PROC_STR 256
#define PFS_VLAN0 0
#define PFS_VLAN1 1
#define PFS_VLAN2 2
#define PFS_VLAN3 3
#define PFS_STATUS 4
#define PFS_VLAN0_LINK_DOWN 5
#define PFS_VLAN1_LINK_DOWN 6
#define PFS_VLAN2_LINK_DOWN 7
#define PFS_VLAN3_LINK_DOWN 8


char adm5120_procdir[]="sys/net/adm5120sw";
struct proc_dir_entry *adm5120_entry;
struct proc_dir_entry *vlan_entry;

struct dev_entrie{
	char *name;
	int mark;
	struct proc_dir_entry *pent;
	mode_t mode;
	read_proc_t *fread;
	write_proc_t *fwrite;
};

static int store_vlan2port(struct file *file,const char *buffer,unsigned long count,void *data);
static int read_switch_status(char *buf, char **start, off_t offset, int count, int *eof, void *data);

//#define ADM5120_FORCE_LINK_CONTROL
#ifdef ADM5120_FORCE_LINK_CONTROL
static int store_force_link(struct file *file,const char *buffer,unsigned long count,void *data);
static int read_force_link(char *buf, char **start, off_t offset, int count, int *eof, void *data); 
#endif

static struct dev_entrie entries[]={
	{ "eth0", PFS_VLAN0, NULL, 400, NULL, store_vlan2port },
	{ "eth1", PFS_VLAN1, NULL, 400, NULL, store_vlan2port },
	{ "eth2", PFS_VLAN2, NULL, 400, NULL, store_vlan2port },
	{ "eth3", PFS_VLAN3, NULL, 400, NULL, store_vlan2port },
	{ "status", PFS_STATUS, NULL, 400, read_switch_status, NULL },
#ifdef ADM5120_FORCE_LINK_CONTROL
	{ "force_lnk_down_eth0", PFS_VLAN0_LINK_DOWN, NULL, 400, read_force_link, store_force_link },
	{ "force_lnk_down_eth1", PFS_VLAN1_LINK_DOWN, NULL, 400, read_force_link, store_force_link },	
	{ "force_lnk_down_eth2", PFS_VLAN2_LINK_DOWN, NULL, 400, read_force_link, store_force_link },	
	{ "force_lnk_down_eth3", PFS_VLAN3_LINK_DOWN, NULL, 400, read_force_link, store_force_link },	
#endif
};

#define PFS_ENTS  (sizeof(entries)/sizeof(struct dev_entrie))

static int init_adm5120_in_procfs(void);
static void del_adm5120_from_procfs(void);


static int set_entry(struct proc_dir_entry *ent,read_proc_t *fread, write_proc_t *fwrite,int mark)
{
	short *mk;
	if( !( mk=(short *)kmalloc( sizeof( short ),GFP_KERNEL ) ) )
		return -1;
	*mk=mark;
	ent->data=(void *)mk;
	ent->owner=THIS_MODULE;
	ent->read_proc=fread;
	ent->write_proc=fwrite;
	return 0;
}


static int init_adm5120_in_procfs(void)
{
	int i,j;
	adm5120_entry=proc_mkdir(adm5120_procdir,NULL);
	if ( adm5120_entry==NULL )
		return -ENOMEM;
	PDEBUG(0,"creating entries");
	for ( i=0; i<PFS_ENTS; i++ ) {
		if ( !(entries[i].pent = create_proc_entry(entries[i].name,entries[i].mode,adm5120_entry) ) )
			goto err1;
		PDEBUG(0,"file \"%s\" created successfuly",entries[i].name);			
		if ( set_entry(	entries[i].pent,entries[i].fread, entries[i].fwrite,entries[i].mark) )
			goto err1;
		PDEBUG(0,"parameters of \"%s\" setted",entries[i].name);
	}
	return 0;

err1:
	PDEBUG(0,"eror creating \"%s\", abort",entries[i].name);
	for ( j=0; j<=i; j++ )
		if( entries[j].pent->data )
			kfree(entries[j].pent->data);
	for ( j=0; j<=i; j++ )
		remove_proc_entry(entries[j].name,adm5120_entry);
	remove_proc_entry("",adm5120_entry);
	return -1;
}        

static void del_adm5120_from_procfs(void)
{
	int j;
	PDEBUG(0,"start");
	for ( j=0; j<PFS_ENTS; j++ )
		if( entries[j].pent->data )
			kfree(entries[j].pent->data);

	PDEBUG(0,"1:");			
	for ( j=0; j<PFS_ENTS; j++ ){
		remove_proc_entry(entries[j].name,adm5120_entry);
		PDEBUG(0,"delete %s",entries[j].name);
	}
	PDEBUG(0,"2:");
	remove_proc_entry(adm5120_procdir,NULL);
	PDEBUG(0,"end");	
}        

static int skip_blanks(char **ptr,int cnt)
{
	int i;
	PDEBUG(5,"start ptr=%x",*ptr);
	for ( i=0;i<cnt;i++,(*ptr)++ )
		if( (**ptr)!=' ')
			break;
	PDEBUG(5,"end ptr=%x,num of blanks=%d",*ptr,i);			
	return i;
}

static int store_vlan2port(struct file *file,const char *buffer,unsigned long count,void *data)
{
	int i;
	char *ptr=(char*)buffer;
	u8 cport;
	int vlan_val=0;
	int vlan_num = *(short*)data;
	struct net_device *ndev = &adm5120sw_devs[vlan_num];	
	PSW_PRIV_T priv = (PSW_PRIV_T)ndev->priv;

	u8 err = 0;

	PDEBUG(0,"start store vlan to port mapping\nstr=%s",buffer);	
	// check for correct symbols
	for ( i=0;i<count;i++ )
		if( ( buffer[i]<'0' || buffer[i]>'9' ) 
				&& buffer[i]!=' ' && buffer[i]!='\0'
				&& buffer[i]!='\n' && buffer[i]!='d'){
			PDEBUG(5,"error: bad symbol: i=%d, sym=(%c,%d)",i,buffer[i],buffer[i]);			
			goto exit;
		}
	PDEBUG(0,"all characters are ok");
	// parse input string
	i=skip_blanks(&ptr,count);

	// Disable VLAN
	if( *ptr == 'd' ){
		// down if
		SetupVLAN(priv->unit,0);
		DisableVlanGroup(priv->unit,sw_context->vlanGrp[ vlan_num ]);
		sw_context->vlanGrp[ vlan_num ] =  1<<CPU_PORT;
		goto exit;
	}

	// Setup VLAN
	while ( (ptr-buffer)<count && ( *ptr>='0' && *ptr<='9' ) ) {
		cport=*ptr-'0';
		if( cport<ETH_PORT_NUM ){
			//check that this port is not in any other VLAN
			err = 0;
			for ( i=0;i<MAX_VLAN_GROUP;i++ ){
				if( i == vlan_num )
					continue;
				if( (1<<cport) & sw_context->vlanGrp[i] ) {
					printk(KERN_NOTICE"adm5120sw: port%d is already in VLAN%d\n",
							cport,i);
					err = 1;
				}
			}
			if( !err )		
				vlan_val |= 1<<cport;
		}
		ptr++;
	}
	PDEBUG(0,"commit changes: vlan[%d]=%x",vlan_num,vlan_val);
	sw_context->vlanGrp[vlan_num] &= 1<<CPU_PORT;
	sw_context->vlanGrp[vlan_num] |= vlan_val & 0x7F;
	// Apply changes	
	if ( ndev->flags & IFF_UP ) {
		SetupVLAN(priv->unit, sw_context->vlanGrp[priv->unit]);
		ProgramVlanMac(priv->unit, ndev->dev_addr, SW_MAC_AGE_VALID);
		EnableVlanGroup(priv->unit, (unsigned long)sw_context->vlanGrp[priv->unit]);
	}
exit:
	return count;
}

static int read_switch_status(char *buf, char **start, off_t offset, int count, int *eof, void *data) { 
	/* 
	 *      Original code by SVIt v1(a)t5.ru 
	 *      ported to kernel mode by Vlad Moskovets midge(a)vlad.org.ua 20061002 
	 * 
	 */ 
	int len = 0, i, j; 
	i = ADM5120_SW_REG(PHY_st_REG); 
	for ( j=0; j < 5; j++ ) { 
		len += sprintf(buf + len, "Port%d\t", j); 
		len += sprintf(buf + len, (i & (1<<j))?"up  \t":"down\t"); 
		if (i & (1<<j)) { 
			len += sprintf(buf + len, (i & (256<<j)?"100M\t":"10M \t")); 
			len += sprintf(buf + len, (i & (65536<<j)?"full-duplex\t":"half-duplex\t")); 
		} else  
			len += sprintf(buf + len, "-\t-\t\t"); 

		len += sprintf(buf + len, "%s\t", (sw_context->port[j].status == PORT_ENABLED)?"enabled ":"disabled"); 
		len += sprintf(buf + len, "vlanid=%d\t", (int) sw_context->port[j].vlanId); 
		len += sprintf(buf + len, "unit=%d\t", (int) sw_context->port[j].ifUnit); 
		len += sprintf(buf + len, "\n");  
	} 

	return len; 
} 

/*
 * Force link down on some of switch interfaces
 * This can be used as example as part of mapping other interfaces on eth
 * input buffer:
 *	"0" - disable forced link down
 *	"1" - enable forced link down 
 */
 
#ifdef ADM5120_FORCE_LINK_CONTROL
 
static int store_force_link(struct file *file,const char *buf,unsigned long count,void *data)
{
	char *ptr=(char*)buf;
	int vlan_num = *(short*)data - PFS_VLAN0_LINK_DOWN;
	struct net_device *ndev = &adm5120sw_devs[vlan_num];
	u32 vlan_port_mask = (sw_context->vlanGrp[ vlan_num ]<<SW_PHY_NORMAL_SHIFT) & SW_PHY_NORMAL_MASK;

	if( !count ){
		return 0;
	}
/*	
	printk("Set forced link status of %s to %c\n",ndev->name,buf[0]);
	printk("%08x & %08x",sw_context->vlanGrp[ vlan_num ]<<SW_PHY_NORMAL_SHIFT,SW_PHY_NORMAL_MASK);
	printk("Disabling ports: %08x\n",vlan_port_mask);
*/
	if( *ptr == '1' )
	    ADM5120_SW_REG(PHY_cntl2_REG) &= ~(vlan_port_mask);
	else if( *ptr == '0' )
	    ADM5120_SW_REG(PHY_cntl2_REG) |= vlan_port_mask;
	return count;
}

static int read_force_link(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{ 
	int vlan_num = *(short*)data - PFS_VLAN0_LINK_DOWN;
	u32 vlan_port_mask = (sw_context->vlanGrp[ vlan_num ]<<SW_PHY_NORMAL_SHIFT) & SW_PHY_NORMAL_MASK;
	u32 tmp = ADM5120_SW_REG(PHY_cntl2_REG) & vlan_port_mask;
	*eof = 1;	
	if( !(ADM5120_SW_REG(PHY_cntl2_REG) & vlan_port_mask) )
		return snprintf(buf,count,"enabled");
	else
		return snprintf(buf,count,"disabled");	
} 
#endif


/*********************************************
 *
 * VLAN
 *
 *********************************************/

static int SetupVLAN(int unit, unsigned long portmask)
{
	unsigned long reg, shiftcnt;

	if (unit < 0 || unit > 6)
		return -1;

	if (unit <= 3) {
		shiftcnt = 8 * unit;
		reg = ADM5120_SW_REG(VLAN_G1_REG) & ~(VLAN_PORT_MASK << shiftcnt);
		reg |= (portmask & VLAN_PORT_MASK) << shiftcnt;
		ADM5120_SW_REG(VLAN_G1_REG) = reg;
	} else {
		shiftcnt = 8 * (unit - 4);
		reg = ADM5120_SW_REG(VLAN_G2_REG) & ~(VLAN_PORT_MASK << shiftcnt);
		reg |= (portmask & VLAN_PORT_MASK) << shiftcnt;
		ADM5120_SW_REG(VLAN_G2_REG) = reg;	
	}

	return 0;
}

int ProgramVlanMac(int vlan, char *Mac, int clear)
{
	unsigned long Reg0, Reg1;

	if (vlan < 0 || vlan >= MAX_VLAN_GROUP)
		return -1;

	Reg0 = (((unsigned char)Mac[1] << 24) | ((unsigned char)Mac[0] << 16)) | (vlan << SW_MAC_VLANID_SHIFT)
		| SW_MAC_WRITE | SW_MAC_VLANID_EN;

	if (!clear)
		Reg0 |= SW_MAC_AGE_VALID;

	Reg1 = ((unsigned char)Mac[5] << 24) | ((unsigned char)Mac[4] << 16) |
		((unsigned char)Mac[3] << 8) | (unsigned char)Mac[2];

	ADM5120_SW_REG(MAC_wt1_REG) = Reg1;
	ADM5120_SW_REG(MAC_wt0_REG) = Reg0;

	while (!(ADM5120_SW_REG(MAC_wt0_REG) & SW_MAC_WRITE_DONE));

	return 0;
}

void EnableVlanGroup(int unit, unsigned long vlanGrp)
{
	int i;
	int vlanId = 0x01 << unit;

	vlanGrp &= SW_DISABLE_PORT_MASK;
	ADM5120_SW_REG(Port_conf0_REG) &= ~vlanGrp;

	/* Mark the enabled ports */
	for (i = 0; i < ETH_PORT_NUM; i++) {
		if (vlanGrp & (0x01 << i)) {
			sw_context->port[i].status = PORT_ENABLED;
			sw_context->port[i].vlanId = vlanId;
			sw_context->port[i].ifUnit = unit;
		}
	}
}	

void DisableVlanGroup(int unit, unsigned long vlanGrp)
{
	int i;
	unsigned long reg;

	vlanGrp &= SW_DISABLE_PORT_MASK;

	reg = ADM5120_SW_REG(Port_conf0_REG) | vlanGrp;
	ADM5120_SW_REG(Port_conf0_REG) = reg;

	/* Mark the disabled ports */
	for (i = 0; i < NUM_IF5120_PORTS; i++)
		if (vlanGrp & (0x01<<i))
			sw_context->port[i].status = PORT_DISABLED;
}


/*------------------------------------------------------
  Linux interface related functions
  ------------------------------------------------------*/

int adm5120sw_open(struct net_device *dev)
{
	PSW_PRIV_T priv = (PSW_PRIV_T)dev->priv;

	int unit = priv->unit;

	/* setup vlan reg */
	SetupVLAN(unit, sw_context->vlanGrp[unit]);

	/* program vlan mac */
	ProgramVlanMac(unit, dev->dev_addr, SW_MAC_AGE_VALID);

	/* Enable vlan Group */
	EnableVlanGroup(unit, (unsigned long)sw_context->vlanGrp[unit]);

	priv->vlanId = 1 << unit;

	if (sw_context->actIfCnt == 0) {
		/* Enable interrupt */
		ADM5120_SW_REG(SW_Int_mask_REG) = ~sw_context->intMask;
	}
	sw_context->actIfCnt++;

	spin_unlock(&sw_context->lock);

	dev->irq = SW_IRQ;
	netif_start_queue(dev);

	return 0;
}


int adm5120sw_release(struct net_device *dev)
{
	PSW_PRIV_T priv = (PSW_PRIV_T)dev->priv;
	int unit = priv->unit;

	netif_stop_queue(dev); /* can't transmit any more */

	spin_lock(&sw_context->lock);

	/* Enable vlan Group */
	DisableVlanGroup(unit, (unsigned long)sw_context->vlanGrp[unit]);

	if (--sw_context->actIfCnt <= 0) {
		ADM5120_SW_REG(SW_Int_mask_REG) = SWITCH_INT_MASK;
	}

	spin_unlock(&sw_context->lock);

	return 0;
}

int adm5120sw_config(struct net_device *dev, struct ifmap *map)
{
	/* can't act on a running  interface */
	if (dev->flags & IFF_UP)
		return -EBUSY;

	/* ignore other fields */
	return 0;
}

void adm5120sw_set_rx_mode(struct net_device *dev)
{
	PSW_PRIV_T priv = (PSW_PRIV_T)dev->priv;
	int port = sw_context->vlanGrp[priv->unit] & 0x3F;

	/* Note do not reorder, GCC is clever about common statements. */
	if (dev->flags & IFF_PROMISC) {
		//		printk (KERN_NOTICE "%s: Promiscous mode enabled.\n", dev->name);
		/*		If set promisc(Bridge mode) we need to disable te SA so all 
				data on same vlan will be resend (hub mode)
				The best way in bridge mode is to set 5 ethernet and put on
				bridge, disavantage all data will be processed by the bridge.
				No good, if not set blocking state can't see the bridge mac-s
				*/
		priv->iflags = IF_PROMISC_MODE;
		ADM5120_SW_REG(CPUp_conf_REG) &= ~(port  << 9);
		ADM5120_SW_REG(Port_conf1_REG) |= (port) | (port << 6);//disable sa_learn & blocking state (for bridge)
	} else {
		//		printk (KERN_NOTICE "%s: Promiscous mode disabled.\n", dev->name);
		priv->iflags &= ~IF_PROMISC_MODE;
		ADM5120_SW_REG(CPUp_conf_REG) |= (port  << 9);
		ADM5120_SW_REG(Port_conf1_REG) &= ~(port ) | ~(port << 6);
	}

}

int adm5120sw_set_mac(struct net_device *dev, void *p)
{
	struct sockaddr *addr = (struct sockaddr *)p;
	unsigned long flags;
	spinlock_t *lock = &((PSW_PRIV_T)dev->priv)->lock;

	spin_lock_irqsave(lock, flags);

	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	spin_unlock_irqrestore(lock, flags);

	return 0;
}


int adm5120sw_tx(struct sk_buff *skb, struct net_device *dev)
{
	PSW_PRIV_T priv = (PSW_PRIV_T)dev->priv;
	unsigned long eflags;
	PTX_ENG_T txEng;
	PTXDESC_T hdesc;
	PTX_DRV_DESC_T drvDesc;
	unsigned long csum = 0;
	int len;

	if (skb == NULL) return 0;

	spin_lock_irqsave(&priv->lock, eflags);

	dev->trans_start = jiffies; /* save the timestamp */

	/* get tx engine */
	if (priv->priority == IF_TX_PRIORITY_H)
		txEng = &sw_context->txH;
	else
		txEng = &sw_context->txL;

	/* get check sum flag */
	if (priv->csum_flags)
		csum = TX_ADD_IPSUM;

	/* get hardware descriptor */
	hdesc = &txEng->hwDesc[txEng->idxHead];
	if (hdesc->buf1cntl & OWN_BIT) {
		dev_kfree_skb(skb);
		priv->stats.tx_dropped++;
		return 0;
	}

	drvDesc = &txEng->drvDesc[txEng->idxHead];
	drvDesc->skb = skb;

	hdesc->buf1cntl = (hdesc->buf1cntl & END_OF_RING) | 
		(((unsigned long)skb->data & BUF_ADDR_MASK) | OWN_BIT);

	len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
	hdesc->pktcntl = (len << PKT_LEN_SHIFT) | priv->vlanId | csum;
	hdesc->buf1len = len;

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += len;

	if (++txEng->idxHead >= txEng->numDesc)
		txEng->idxHead = 0;
	ADM5120_SW_REG(Send_trig_REG) = txEng->txTrig;

	spin_unlock_irqrestore(&priv->lock, eflags);

	return 0;
}

int adm5120sw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	return 0;
}

void adm5120sw_tx_timeout (struct net_device *dev)
{
	return;
}

struct net_device_stats *adm5120sw_stats(struct net_device *dev)
{
	PSW_PRIV_T priv = (PSW_PRIV_T)dev->priv;
	return &priv->stats;
}

int adm5120sw_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	spinlock_t *lock = &((PSW_PRIV_T)dev->priv)->lock;

	/* check ranges */
	if ((new_mtu < 68) || (new_mtu > 1500))
		return -EINVAL;

	spin_lock_irqsave(lock, flags);
	dev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);
	return 0;
}

int adm5120sw_rebuild_header(struct sk_buff *skb){return 0;}

int adm5120sw_header(struct sk_buff *skb, struct net_device *dev,
		unsigned short type, void *daddr, void *saddr,
		unsigned int len)
{
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);

	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);
	return (dev->hard_header_len);
}

int adm5120sw_init(struct net_device *dev)
{
	PSW_PRIV_T priv;

	/* assign some of the fields */
	ether_setup(dev);

	dev->open		= adm5120sw_open;
	dev->stop		= adm5120sw_release;
	dev->set_config		= adm5120sw_config;
	dev->set_multicast_list = adm5120sw_set_rx_mode;
	dev->set_mac_address	= adm5120sw_set_mac;
	dev->hard_start_xmit	= adm5120sw_tx;
	dev->do_ioctl		= adm5120sw_ioctl;
	dev->get_stats		= adm5120sw_stats;
	dev->change_mtu		= adm5120sw_change_mtu;
	dev->rebuild_header	= adm5120sw_rebuild_header;
	dev->hard_header	= adm5120sw_header;
	dev->tx_timeout		= adm5120sw_tx_timeout;
	dev->watchdog_timeo	= timeout;

	set_bit(__LINK_STATE_PRESENT, &dev->state);

	/*
	 * Then, allocate the priv field. This encloses the statistics
	 * and few private fields.
	 */
	dev->priv = kmalloc(sizeof(SW_PRIV_T), GFP_KERNEL);
	if (dev->priv == NULL)
		return (-ENOMEM);

	memset(dev->priv, 0, sizeof(SW_PRIV_T));

	priv = (PSW_PRIV_T)dev->priv;
	priv->unit = unit++;

	spin_lock_init(&priv->lock);

	return 0;
}




/* ------------------------------------------------------
   Driver initialisation
   ------------------------------------------------------*/


static int __init adm5120switch_init (void)
{
	int result, i, device_present = 0;
	BOARD_CFG_T boardCfg = {0};

	printk("ADM5120 Switch Module Init V1.3\n");
	if (adm5120swdrv_init() != 0)
		return (-ENODEV);

	sw_context->nr_if = adm5120_get_nrif(sw_context->vlanGrp);

	memcpy((char *)&boardCfg, (char *)PA2VA(ADM5120SW_BOARD_CFG_ADDR), sizeof (boardCfg));
	if (boardCfg.macmagic != MAC_MAGIC)
		memmove(&boardCfg.mac[0][0], ADM5120SW_DEFAULT_MAC , 6);
	printk("ADM5120 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
			boardCfg.mac[0][0], boardCfg.mac[0][1],
			boardCfg.mac[0][2], boardCfg.mac[0][3],
			boardCfg.mac[0][4], boardCfg.mac[0][5]);

	for (i = 0; i < sw_context->nr_if; i++) {
		memcpy( adm5120sw_devs[i].dev_addr, &boardCfg.mac[0][0], 6);
		adm5120sw_devs[i].dev_addr[5] += i;

		if ((result = register_netdev(adm5120sw_devs + i ))) {
			printk("am5120sw: error %i registering device \"%s\"\n",
					result, adm5120sw_devs[i].name);
		} else
			device_present++;
	}
	result = request_irq(SW_IRQ, swdrv_ProcessInt, SA_SHIRQ | SA_SAMPLE_RANDOM, "adm5120_sw", &sw_context);
	if (result)
		return -ENODEV;


	if (device_present != sw_context->nr_if)
		return (-ENODEV);

	init_adm5120_in_procfs();

	return 0;

}

static void __exit adm5120switch_cleanup(void)
{
	int i;

	free_irq( SW_IRQ , &sw_context );	
	for ( i = 0; i < sw_context->nr_if; i++)
	{

		kfree(adm5120sw_devs[i].priv);
		unregister_netdev(adm5120sw_devs + i);
	}
	del_adm5120_from_procfs();
}

module_init(adm5120switch_init);
module_exit(adm5120switch_cleanup);

