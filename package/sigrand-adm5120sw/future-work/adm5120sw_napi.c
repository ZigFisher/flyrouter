/*
 *	ADM5120 built in ethernet switch driver
 *
 *	Copyright Jeroen Vreeken (pe1rxq@amsat.org), 2005
 *
 *	Inspiration for this driver came from the original ADMtek 2.4 
 *	driver, Copyright ADMtek Inc.
 *
 *      Ported back to 2.4(.18-adm) and added default MAC address
 *      table by Sergio Aguayo.
 *
 *      Andrey Ivanov <andrey-s-ivanov@yandex.ru> 07/2006
 *        Improved performance: added support for NAPI.
 *        Revised init logic.
 *
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in6.h>

#include <linux/delay.h>
#include <linux/proc_fs.h>

#include <asm/checksum.h>
#include "adm5120sw.h"

MODULE_AUTHOR("Andrey Ivanov (andrey-s-ivanov@yandex.ru), Jeroen Vreeken (pe1rxq@amsat.org) and Sergio Aguayo (webmaster@qmailhosting.net)");
MODULE_DESCRIPTION("ADM5120 ethernet switch driver");
MODULE_LICENSE("GPL");

#define PFX "adm5120sw: "

#if 1
#define DEBUG_PRINT(args...) printk( KERN_DEBUG PFX args )
#else
#define DEBUG_PRINT(args...) (void) 0
#endif


/*
 *	The ADM5120 uses an internal matrix to determine which ports
 *	belong to which VLAN.
 *	The default generates a VLAN (and device) for each port 
 *	(including MII port) and the CPU port is part of all of them.
 *
 *	Another example, one big switch and everything mapped to eth0:
 *	0x7f, 0x00, 0x00, 0x00, 0x00, 0x00
 */
static unsigned char vlan_matrix[SW_DEVS] = {
   	0x41, 0x42, 0x44, 0x48, 0x50, 0x60     /* 6 different interfaces, not connected */
/*   0x5e, 0x41, 0, 0, 0, 0  /* Edimax layout 4+1 */
};

/*
 *      The ADM5120 Demo board has its own MAC address on flash
 *      memory but for some reason Edimax and others decided not to
 *      use that space for such purpose. They instead have their
 *      own structure for this and other data. Understanding
 *      that structure is outside the kernel's scope and anyway
 *      the initscripts of such devices will certainly set the
 *      MAC addresses to the correct value before using any
 *      net interface.
 *
 *      Just in case we have an initscript that doesn't set any
 *      MAC address, we have this table to associate each device
 *      with its own address.
 */
static unsigned char default_macaddrs[SW_DEVS][6] __initdata = 
{
     { 0x00, 0x50, 0xfc, 0x11, 0x22, 0x33 },
     { 0x00, 0x50, 0xfc, 0x33, 0x22, 0x11 },
     { 0x00, 0x50, 0xfc, 0x11, 0x22, 0x34 },
     { 0x00, 0x50, 0xfc, 0x33, 0x22, 0x12 },
     { 0x00, 0x50, 0xfc, 0x11, 0x22, 0x35 },
     { 0x00, 0x50, 0xfc, 0x33, 0x22, 0x13 },

};

static struct net_device *adm5120_devs[SW_DEVS];
static int adm5120_nrdevs = SW_DEVS;

static int int_enabled = 0;

static spinlock_t init_tx_lock = SPIN_LOCK_UNLOCKED; /* lock for tx rings and init code */
static spinlock_t rx_lock = SPIN_LOCK_UNLOCKED; /* lock for rx rings */
static spinlock_t int_lock = SPIN_LOCK_UNLOCKED;

struct timer_list tx_watchdog_timer; /* timer for tx ring */
static int tx_queues_stopped = 0;
static u32 enable_vlan_mask = 0;
static int port2vlan[SW_MAX_PORTS];

static struct adm5120_dma
	adm5120_dma_txh_v[ADM5120_DMA_TXH] __attribute__((aligned(16))),
	adm5120_dma_txl_v[ADM5120_DMA_TXL] __attribute__((aligned(16))),
	adm5120_dma_rxh_v[ADM5120_DMA_RXH] __attribute__((aligned(16))),
	adm5120_dma_rxl_v[ADM5120_DMA_RXL] __attribute__((aligned(16)));
static struct sk_buff
	*adm5120_skb_txh[ADM5120_DMA_TXH],
	*adm5120_skb_txl[ADM5120_DMA_TXL],
	*adm5120_skb_rxh[ADM5120_DMA_RXH],
	*adm5120_skb_rxl[ADM5120_DMA_RXL];

static struct tx_ring tx_h_ring, tx_l_ring;
static struct rx_ring rx_h_ring, rx_l_ring;

static int adm5120_poll(struct net_device *dev, int *budget);

static struct net_device poll_dev = {
	refcnt: ATOMIC_INIT(1),
	state: (1UL << __LINK_STATE_START),
	poll: adm5120_poll,
/*	quota: 0, */
	weight: 64,
/*	poll_list: LIST_HEAD_INIT(poll_dev.poll_list) */
};

static void adm5120_restart(void);


static inline void adm5120_set_reg(unsigned int reg, unsigned long val)
{
	*(volatile unsigned long*) (SW_BASE + reg) = val;
}

static inline unsigned long adm5120_get_reg(unsigned int reg)
{
	return *(volatile unsigned long*) (SW_BASE + reg);
}

static void adm5120_print_regs(void)
{
	unsigned int reg;

	for (reg = 0; reg < 0x100; reg += 4)
	{
		DEBUG_PRINT("%.2x: 0x%.8lx\n", reg, adm5120_get_reg(reg));
	}
}

static void adm5120_print_tx_ring(struct tx_ring *ring)
{
	DEBUG_PRINT("num_desc: %d\n", ring->num_desc);
	DEBUG_PRINT("   avail: %d\n", ring->avail);
	DEBUG_PRINT("head_idx: %d\n", ring->head_idx);
	DEBUG_PRINT("tail_idx: %d\n", ring->tail_idx);
}


static void inline _adm5120_mask_int(void)
{
#if 0
	adm5120_set_reg(ADM5120_INT_MASK,
			adm5120_get_reg(ADM5120_INT_MASK) | ADM5120_INTHANDLE);
#else
	adm5120_set_reg(ADM5120_INT_MASK, ADM5120_INTMASKALL);
#endif
}

static void inline _adm5120_unmask_int(void)
{
#if 0
	adm5120_set_reg(ADM5120_INT_MASK,
			adm5120_get_reg(ADM5120_INT_MASK) & ~ADM5120_INTHANDLE);
#else
	adm5120_set_reg(ADM5120_INT_MASK, ~ADM5120_INTHANDLE);
#endif
}

static void inline _adm5120_try_unmask_int(void)
{
	if (int_enabled)
		_adm5120_unmask_int();
}

static void inline adm5120_disable_int(void)
{
	unsigned long flag;

	spin_lock_irqsave(&int_lock, flag);
	int_enabled = 0;
	_adm5120_mask_int();
	spin_unlock_irqrestore(&int_lock, flag);
}

static void inline adm5120_enable_int(void)
{
	unsigned long flag;

	spin_lock_irqsave(&int_lock, flag);
	int_enabled = 1;
	if (!test_bit(__LINK_STATE_RX_SCHED, &poll_dev.state)) /* not enable ints while in poll mode */
		_adm5120_unmask_int();
	spin_unlock_irqrestore(&int_lock, flag);
}





static int adm5120_rx(struct rx_ring *ring, int *budget)
{
	struct sk_buff *skb, *skbn;
	struct adm5120_sw *priv;
	struct net_device *dev;
	int port, vlan, len;
	int work = 0;
	int empty = 0;
	int idx = ring->idx;
	volatile struct adm5120_dma *descl = ring->desc;
	volatile struct adm5120_dma *desc;
	struct sk_buff **skbl = ring->skb;

	for (;;)
	{

		desc = &descl[idx];

		if (desc->data & ADM5120_DMA_OWN)
		{
			empty = 1;
			break;
		}

		if (work >= *budget)
			break;
		
		skb = skbl[idx];
		skbn = NULL;

		port = (desc->status & ADM5120_DMA_PORTID) >> ADM5120_DMA_PORTSHIFT;
		vlan = port2vlan[port];
		if (vlan == -1)
		{
			printk( KERN_ERR PFX "rx error: port not mapped to vlan.\n");
			goto next;
		}

		dev = adm5120_devs[vlan];
		priv = dev->priv;

		len = ((desc->status & ADM5120_DMA_LEN) >> ADM5120_DMA_LENSHIFT) - ETH_FCS;
		if (len <= 0 || len > ADM5120_DMA_RXSIZE ||
		    desc->status & ADM5120_DMA_FCSERR) 
		{
			priv->stats.rx_errors++;
			goto next;
		}

		skbn = dev_alloc_skb(ADM5120_DMA_RXSIZE + 16);
		if (!skbn) 
		{
			DEBUG_PRINT("can't alloc rx buf: out of memory.\n");
			priv->stats.rx_dropped++;
			goto next;
		}

		skb_reserve(skbn, 2);
		skbl[idx] = skbn;

		skb_put(skb, len);
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	next:
		desc->status = 0;
		desc->cntl = 0;
		desc->len = ADM5120_DMA_RXSIZE;
		wmb();
		desc->data = ADM5120_DMA_ADDR(skbl[idx]->data) | ADM5120_DMA_OWN |
			(idx == ring->num_desc - 1 ? ADM5120_DMA_RINGEND : 0);

		if (skbn)
		{
			dev->last_rx = jiffies;
			priv->stats.rx_packets++;
			priv->stats.rx_bytes += len;
			netif_receive_skb(skb);
		}

		if (++idx == ring->num_desc)
			idx = 0;

		++work;
	}

	ring->idx = idx;
	*budget -= work;

	return empty;
}

static inline void adm5120_stop_tx_queues(void)
{
	int i;
	for (i = 0; i < adm5120_nrdevs; i++)
		if (enable_vlan_mask & (1 << i))
			netif_stop_queue(adm5120_devs[i]);

	mod_timer(&tx_watchdog_timer, jiffies + ETH_TX_TIMEOUT);

	tx_queues_stopped = 1;
	DEBUG_PRINT("tx queues stopped\n");
}

static inline void adm5120_wake_tx_queues(void)
{
	int i;
	for (i = 0; i < adm5120_nrdevs; i++)
		if (enable_vlan_mask & (1 << i))
			netif_wake_queue(adm5120_devs[i]);

	del_timer(&tx_watchdog_timer);

	tx_queues_stopped = 0;
	DEBUG_PRINT("tx queues waked\n");
}

static inline void adm5120_start_tx_queues(void)
{
	int i;
	for (i = 0; i < adm5120_nrdevs; i++)
		if (enable_vlan_mask & (1 << i))
			netif_start_queue(adm5120_devs[i]);

	del_timer(&tx_watchdog_timer);

	tx_queues_stopped = 0;
	DEBUG_PRINT("tx queues started\n");
}


static inline void adm5120_tx(struct tx_ring *ring)
{
	volatile struct adm5120_dma *descl = ring->desc;
	struct sk_buff **skbl = ring->skb;
	int idx = ring->tail_idx;

	while (ring->avail < ring->num_desc && 
	       !(descl[idx].data & ADM5120_DMA_OWN)) {
		dev_kfree_skb_any(skbl[idx]);
		skbl[idx] = NULL;

		++ring->avail;

		if (++idx == ring->num_desc)
			idx = 0;
	}

	ring->tail_idx = idx;
}

static unsigned long saved_int_st = 0; /* saves not completed ints for next poll iteration */

static int adm5120_poll(struct net_device *dev, int *budget)
{
	unsigned long int_st;
	int work = 0;
	int start_quota = min(dev->quota, *budget);
	int quota = start_quota;


	int_st = adm5120_get_reg(ADM5120_INT_ST);
/*	DEBUG_PRINT("      int_st = 0x%.8lx, start_quota = %d\n", int_st, start_quota); */
	int_st &= ADM5120_INTHANDLE;
	/* acknowledge ints */
	adm5120_set_reg(ADM5120_INT_ST, int_st);
	int_st |= saved_int_st;
	saved_int_st = 0;

	if (int_st & (ADM5120_INT_TXH | ADM5120_INT_TXL)) {
		spin_lock_bh(&init_tx_lock);

		/* tx high priority packets to cpu */
		if (int_st & ADM5120_INT_TXH)	
			adm5120_tx(&tx_h_ring);

		/* tx normal priority packets to cpu */
		if (int_st & ADM5120_INT_TXL)	
			adm5120_tx(&tx_l_ring);

		if( tx_queues_stopped && 
		    tx_h_ring.avail > ADM5120_TXH_WAKEUP_THRESH && 
		    tx_l_ring.avail > ADM5120_TXL_WAKEUP_THRESH )
			adm5120_wake_tx_queues();

		spin_unlock_bh(&init_tx_lock);
	}

	if (int_st & (ADM5120_INT_RXH | ADM5120_INT_RXL)) {
		spin_lock_bh(&rx_lock);

		if (int_st & ADM5120_INT_RXH) {
			if (adm5120_rx(&rx_h_ring, &quota))
				DEBUG_PRINT("rx hring empty.\n");
			else
				saved_int_st |= ADM5120_INT_RXH; /* rx hring not empty */
		}

		if (int_st & ADM5120_INT_RXL) {
			if (adm5120_rx(&rx_l_ring, &quota))
				DEBUG_PRINT("rx lring empty.\n");
			else
				saved_int_st |= ADM5120_INT_RXL; /* rx lring not empty */
		}
		spin_unlock_bh(&rx_lock);
	}

	if (int_st & ADM5120_INT_HFULL)
		DEBUG_PRINT("rx hring full.\n");

	if (int_st & ADM5120_INT_LFULL)
		DEBUG_PRINT("rx lring full.\n");


	work = start_quota - quota;
	dev->quota -= work;
	*budget -= work;

/*	DEBUG_PRINT("saved_int_st = 0x%.8lx, work = %d\n", saved_int_st, work); */

	if (!saved_int_st) { /* if all rx rings empty */
		unsigned long flag;

		DEBUG_PRINT("exiting poll mode.\n");
		spin_lock_irqsave(&int_lock, flag);

		netif_rx_complete(dev);
		/* enable ints */
		_adm5120_try_unmask_int();

		spin_unlock_irqrestore(&int_lock, flag);

		return 0;
	}

	return 1;
}

static irqreturn_t
adm5120_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long flag;

	spin_lock_irqsave(&int_lock, flag);

	if (netif_rx_schedule_prep(&poll_dev)) {
		/* disable interrupts */
		_adm5120_mask_int();
		/* tell system we have work to be done. */
		__netif_rx_schedule(&poll_dev);
		DEBUG_PRINT("entering poll mode.\n");
	} else {
		printk(KERN_ERR PFX "error!: interrupt while in poll mode"
		       ", mask: 0x%.8lx, state: 0x%.8lx\n", 
		       adm5120_get_reg(ADM5120_INT_MASK), adm5120_get_reg(ADM5120_INT_ST));
	}

	spin_unlock_irqrestore(&int_lock, flag);
	return IRQ_HANDLED;
}

static int adm5120_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret = 0;
	struct tx_ring *ring = &tx_l_ring;
	volatile struct adm5120_dma *desc;
	struct adm5120_sw *priv = dev->priv;
	int trigger = ADM5120_SEND_TRIG_L;
	int idx;
	u32 len;

	spin_lock_bh(&init_tx_lock);

	idx = ring->head_idx;
	desc = &ring->desc[idx];

	dev->trans_start = jiffies;
	if (ring->avail == 0) {
/*		dev_kfree_skb(skb);*/
		priv->stats.tx_dropped++;
#if 0
		adm5120_print_regs();
		adm5120_print_tx_ring(ring);
#endif
/*		printk(KERN_INFO PFX "warning: tx ring full when queue awake!\n");*/

		ret = 1;
		goto out;
	}

	ring->skb[ring->head_idx] = skb;

	len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;

/*	desc->cntl = 0;*/
	desc->len = skb->len; /* len; */
	desc->status = (len << ADM5120_DMA_LENSHIFT) | priv->vlan_mask;
	wmb();
	desc->data = (ADM5120_DMA_ADDR(skb->data) | ADM5120_DMA_OWN |
		(idx == ring->num_desc - 1 ? ADM5120_DMA_RINGEND : 0));
	wmb();
/*	udelay(10);*/
	adm5120_set_reg(ADM5120_SEND_TRIG, trigger);

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;

	--ring->avail;
	if (ring->avail == 0 && !tx_queues_stopped)
		adm5120_stop_tx_queues();
	
	if (++idx == ring->num_desc)
		idx = 0;

	ring->head_idx = idx;

out:	
	spin_unlock_bh(&init_tx_lock);

	return ret;
}

#if 0
static void adm5120_tx_timeout(struct net_device *dev)
{
	/* restart switch */
	adm5120_restart();
}
#endif

static void tx_watchdog(unsigned long arg)
{
	printk(KERN_ERR PFX "error: tx ring hangup, will reset switch\n");
	adm5120_restart();
}

static void adm5120_set_vlan(char *matrix)
{
	unsigned long val;

	val = matrix[0] + (matrix[1]<<8) + (matrix[2]<<16) + (matrix[3]<<24);
	adm5120_set_reg(ADM5120_VLAN_GI, val);
	val = matrix[4] + (matrix[5]<<8);
	adm5120_set_reg(ADM5120_VLAN_GII, val);
}

static void adm5120_set_vlan_mask(int vlan, unsigned long vlan_mask)
{
	unsigned long reg;
	int shift;
	unsigned long mask = 0x7f;

	if (vlan < 0 || vlan > 6) 
		return;
	
	vlan_mask &= 0x7f;
	if (vlan <= 3)
	{
		shift = 8 * vlan;
		reg = adm5120_get_reg(ADM5120_VLAN_GI) & ~(mask << shift);
		reg |= vlan_mask << shift;
		adm5120_set_reg(ADM5120_VLAN_GI, reg);
	}
	else
	{
		shift = 8 * (vlan - 4);
		reg = adm5120_get_reg(ADM5120_VLAN_GII) & ~(mask << shift);
		reg |= vlan_mask << shift;
		adm5120_set_reg(ADM5120_VLAN_GII, reg);
	}
}
static void adm5120_clear_vlan_mask(int vlan)
{
	unsigned long reg;
	int shift;
	unsigned long mask = 0x7f;

	if (vlan < 0 || vlan > 6) 
		return;
	
	if (vlan <= 3)
	{
		shift = 8 * vlan;
		reg = adm5120_get_reg(ADM5120_VLAN_GI) & ~(mask << shift);
		adm5120_set_reg(ADM5120_VLAN_GI, reg);
	}
	else
	{
		shift = 8 * (vlan - 4);
		reg = adm5120_get_reg(ADM5120_VLAN_GII) & ~(mask << shift);
		adm5120_set_reg(ADM5120_VLAN_GII, reg);
	}
}

static struct net_device_stats *adm5120_stats(struct net_device *dev)
{
	return &((struct adm5120_sw *)dev->priv)->stats;
}

static void adm5120_set_multicast_list(struct net_device *dev)
{
	struct adm5120_sw *priv = dev->priv;
	int portmask;

	portmask = vlan_matrix[priv->vlan] & 0x3f;

	if (dev->flags & IFF_PROMISC)
		adm5120_set_reg(ADM5120_CPUP_CONF,
		    adm5120_get_reg(ADM5120_CPUP_CONF) &
		    ~((portmask << ADM5120_DISUNSHIFT) & ADM5120_DISUNALL));
	else
		adm5120_set_reg(ADM5120_CPUP_CONF,
		    adm5120_get_reg(ADM5120_CPUP_CONF) |
		    (portmask << ADM5120_DISUNSHIFT));

	if (dev->flags & IFF_PROMISC || dev->flags & IFF_ALLMULTI ||
	    dev->mc_count)
		adm5120_set_reg(ADM5120_CPUP_CONF,
		    adm5120_get_reg(ADM5120_CPUP_CONF) &
		    ~((portmask << ADM5120_DISMCSHIFT) & ADM5120_DISMCALL));
	else
		adm5120_set_reg(ADM5120_CPUP_CONF,
		    adm5120_get_reg(ADM5120_CPUP_CONF) |
		    (portmask << ADM5120_DISMCSHIFT));
}

static void adm5120_write_mac(struct net_device *dev)
{
	struct adm5120_sw *priv = dev->priv;
	unsigned char *mac = dev->dev_addr;

	adm5120_set_reg(ADM5120_MAC_WT1,
	    mac[2] | (mac[3] << 8) | (mac[4] << 16) | (mac[5] << 24));
	adm5120_set_reg(ADM5120_MAC_WT0, 
			(mac[0] << 16) | (mac[1] << 24) | 
			(priv->vlan << 3) | ADM5120_MAC_WRITE | ADM5120_VLAN_EN);

	while (!(adm5120_get_reg(ADM5120_MAC_WT0) & ADM5120_MAC_WRITE_DONE));
}

static int adm5120_set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	adm5120_write_mac(dev);
	return 0;
}

static void make_port2vlan(void)
{
	int vlan, port;

	for (port = 0; port < SW_MAX_PORTS; ++port)
		port2vlan[port] = -1;
	
	for (vlan = 0; vlan < adm5120_nrdevs; vlan++) {
		for (port = 0; port < SW_MAX_PORTS; ++port) {
			if (vlan_matrix[vlan] & (1 << port))
				port2vlan[port] = vlan;
		}
	}
}

static int adm5120_do_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int err;
	struct adm5120_info info;
	struct adm5120_sw *priv = dev->priv;

	switch(cmd) {
		case SIOCGADMINFO:
			info.magic = 0x5120;
			info.ports = adm5120_nrdevs;
			info.vlan = priv->vlan;
			err = copy_to_user(rq->ifr_data, &info, sizeof(info));
			if (err)
				return -EFAULT;
			break;
		case SIOCSMATRIX:
			if (!capable(CAP_NET_ADMIN))
				return -EPERM;
			err = copy_from_user(vlan_matrix, rq->ifr_data,
			    sizeof(vlan_matrix));
			if (err)
				return -EFAULT;
			make_port2vlan();
			adm5120_set_vlan(vlan_matrix);
			break;
		case SIOCGMATRIX:
			err = copy_to_user(rq->ifr_data, vlan_matrix,
			    sizeof(vlan_matrix));
			if (err)
				return -EFAULT;
			break;
		default:
			return -EOPNOTSUPP;
	}
	return 0;
}

static void adm5120_dma_tx_init(struct tx_ring *ring, struct adm5120_dma *dma, 
				struct sk_buff **skb, int num)
{
	memset(dma, 0, sizeof(struct adm5120_dma) * num);
	dma[num - 1].data |= ADM5120_DMA_RINGEND;
	memset(skb, 0, sizeof(struct skb*) * num);

	ring->desc = (void *) KSEG1ADDR((u32)dma);
	ring->skb = skb;
	ring->num_desc = num;
	ring->avail = num;
	ring->head_idx = 0;
	ring->tail_idx = 0;
}

static void adm5120_dma_tx_free(struct tx_ring *ring)
{
	struct sk_buff **skbl = ring->skb;
	int idx = ring->tail_idx;

	while (ring->avail < ring->num_desc) {
		dev_kfree_skb_any(skbl[idx]);
		skbl[idx] = NULL;

		++ring->avail;

		if (++idx == ring->num_desc)
			idx = 0;
	}

/*	ring->tail_idx = idx; */
}

static void adm5120_dma_tx_reinit(struct tx_ring *ring)
{
	adm5120_dma_tx_free(ring);

	memset(ring->desc, 0, sizeof(struct adm5120_dma) * ring->num_desc);
	ring->desc[ring->num_desc - 1].data |= ADM5120_DMA_RINGEND;
	memset(ring->skb, 0, sizeof(struct skb*) * ring->num_desc);

	ring->tail_idx = 0;
	ring->head_idx = 0;
}

static int adm5120_dma_rx_init(struct rx_ring *ring, struct adm5120_dma *dma, 
				struct sk_buff **skb, int num)
{
	int i;
	int ret = 0;

	memset(dma, 0, sizeof(struct adm5120_dma) * num);
	for (i = 0; i < num; i++) {
		skb[i] = dev_alloc_skb(ADM5120_DMA_RXSIZE + 16);
		if (!skb[i])
		{
			printk(KERN_INFO PFX "error: can't alloc rx buffers\n");
			ret = -ENOMEM;
			goto error;
		}

		skb_reserve(skb[i], 2);
		dma[i].data = ADM5120_DMA_ADDR(skb[i]->data) | ADM5120_DMA_OWN;
		dma[i].cntl = 0;
		dma[i].len = ADM5120_DMA_RXSIZE;
		dma[i].status = 0;
	}
	dma[i - 1].data |= ADM5120_DMA_RINGEND;

	ring->desc = (void *) KSEG1ADDR((u32)dma);
	ring->skb = skb;
	ring->num_desc = num;
	ring->idx = 0;

exit:
	return ret;
error:
	while (i--)
		dev_kfree_skb(skb[i]);
/*	
	memset(dma, 0, sizeof(struct adm5120_dma) * num);
	memset(skb, 0, sizeof(struct sk_buff *) * num);
	memset(ring, 0, sizeof(struct rx_ring));
*/
	goto exit;
}

static void adm5120_dma_rx_reinit(struct rx_ring *ring)
{
	int idx = rx_h_ring.idx;
	volatile struct adm5120_dma *descl = rx_h_ring.desc;
	volatile struct adm5120_dma *desc;
	struct sk_buff **skbl = ring->skb;

	idx = rx_h_ring.idx;
	desc = rx_h_ring.desc;
	while (!(descl[idx].data & ADM5120_DMA_OWN)) {
		desc = &descl[idx];

		desc->status = 0;
		desc->cntl = 0;
		desc->len = ADM5120_DMA_RXSIZE;
		wmb();
		desc->data = ADM5120_DMA_ADDR(skbl[idx]->data) | ADM5120_DMA_OWN |
			(idx == ring->num_desc - 1 ? ADM5120_DMA_RINGEND : 0);

		if (++idx == rx_h_ring.num_desc)
			idx = 0;
	}

	ring->idx = 0;
}

static void adm5120_dma_rx_free(struct rx_ring *ring)
{
	int i;
	struct sk_buff **skb = ring->skb;
	int num = ring->num_desc;

	for (i = 0; i < num; i++)
		dev_kfree_skb(skb[i]);
}

static int adm5120_dma_init_rings(void)
{
	int ret;

	ret = adm5120_dma_rx_init(&rx_h_ring, adm5120_dma_rxh_v, adm5120_skb_rxh, ADM5120_DMA_RXH);
	if (ret)
		goto exit;

	ret = adm5120_dma_rx_init(&rx_l_ring, adm5120_dma_rxl_v, adm5120_skb_rxl, ADM5120_DMA_RXL);
	if (ret)
		goto exit;

	adm5120_dma_tx_init(&tx_h_ring, adm5120_dma_txh_v, adm5120_skb_txh, ADM5120_DMA_TXH);
	adm5120_dma_tx_init(&tx_l_ring, adm5120_dma_txl_v, adm5120_skb_txl, ADM5120_DMA_TXL);
exit:
	return ret;
}

static void adm5120_dma_reinit_rings(void)
{
	adm5120_dma_rx_reinit(&rx_h_ring); 
	adm5120_dma_rx_reinit(&rx_l_ring);

	adm5120_dma_tx_reinit(&tx_h_ring);
	adm5120_dma_tx_reinit(&tx_l_ring);
}

static void adm5120_dma_free_rings(void)
{
	adm5120_dma_rx_free(&rx_h_ring);
	adm5120_dma_rx_free(&rx_l_ring);

	adm5120_dma_tx_free(&tx_h_ring);
	adm5120_dma_tx_free(&tx_l_ring);
}

static void _adm5120_reset(void)
{
	DEBUG_PRINT("regs before reset:\n");
	adm5120_print_regs();

	/* Resetting PHY before switch reset */
	adm5120_set_reg(ADM5120_PHY_CNTL2,
			adm5120_get_reg(ADM5120_PHY_CNTL2) & ~ADM5120_NORMAL); 
/*	adm5120_set_reg(ADM5120_PHY_CNTL2,
		adm5120_get_reg(ADM5120_PHY_CNTL2) | ADM5120_NORMAL); */

	/* Switch reset */
	adm5120_set_reg(ADM5120_SWITCH_RESET, 0x01);

	udelay(1000);


	DEBUG_PRINT("regs after reset:\n");
	adm5120_print_regs();
}

static void _adm5120_up(void)
{
	/* disable ints */
	adm5120_disable_int();
	adm5120_set_reg(ADM5120_INT_ST, ADM5120_INTMASKALL);


	adm5120_set_reg(ADM5120_CPUP_CONF,
			ADM5120_DISCCPUPORT | ADM5120_CRC_PADDING |
			ADM5120_DISUNALL | ADM5120_DISMCALL);
	adm5120_set_reg(ADM5120_PORT_CONF0, ADM5120_ENMC | ADM5120_ENBP | ADM5120_DISALL);

	adm5120_set_reg(ADM5120_PHY_CNTL2, adm5120_get_reg(ADM5120_PHY_CNTL2) |
			ADM5120_AUTONEG | ADM5120_NORMAL | ADM5120_AUTOMDIX);
	adm5120_set_reg(ADM5120_PHY_CNTL3, adm5120_get_reg(ADM5120_PHY_CNTL3) |
			ADM5120_PHY_NTH);

	/* clear all vlan settings */
	adm5120_set_reg(ADM5120_VLAN_GI, 0);
	adm5120_set_reg(ADM5120_VLAN_GII, 0);

	/* set tx/rx rings */
	adm5120_set_reg(ADM5120_SEND_HBADDR, (u32) adm5120_dma_txh_v);
	adm5120_set_reg(ADM5120_SEND_LBADDR, (u32) adm5120_dma_txl_v);
	adm5120_set_reg(ADM5120_RECEIVE_HBADDR, (u32) adm5120_dma_rxh_v);
	adm5120_set_reg(ADM5120_RECEIVE_LBADDR, (u32) adm5120_dma_rxl_v);

	/* enable ints */
	adm5120_enable_int();

	/* enable CPU port */
	adm5120_set_reg(ADM5120_CPUP_CONF,
			ADM5120_CRC_PADDING | ADM5120_DISUNALL | ADM5120_DISMCALL);

/*	adm5120_set_vlan(vlan_matrix); */
}

static void _adm5120_down(void)
{
	/* disable ints */
	adm5120_disable_int();

	/* Disable all ports */
	adm5120_set_reg(ADM5120_PORT_CONF0,
			adm5120_get_reg(ADM5120_PORT_CONF0) | ADM5120_DISALL);
	/* Disable CPU port */
	adm5120_set_reg(ADM5120_CPUP_CONF,
			adm5120_get_reg(ADM5120_CPUP_CONF) | ADM5120_DISCCPUPORT);

	/* Wait until switch DMA idle. At least 1ms is required!!!! */
	mdelay(2);

	/* reset switch  */
	_adm5120_reset();
}


static void adm5120_disable_all_ports(u32 vlan_mask)
{
	adm5120_set_reg(ADM5120_PORT_CONF0, 
			adm5120_get_reg(ADM5120_PORT_CONF0) | ADM5120_DISALL);
}

static void adm5120_enable_ports(u32 vlan_mask)
{
	int i;
	unsigned long enable_ports;

	enable_ports = 0;
	for (i = 0; i < adm5120_nrdevs; i++) {
		if (vlan_mask & (1 << i))
			enable_ports |= vlan_matrix[i];
	}
	/* enable ports of active vlans */
	adm5120_set_reg(ADM5120_PORT_CONF0,
			(adm5120_get_reg(ADM5120_PORT_CONF0) & ~ADM5120_DISALL) | (~enable_ports & 0x3f));
}

static int adm5120_open(struct net_device *dev)
{
	int ret = 0;
	struct adm5120_sw *priv = dev->priv;

	spin_lock_bh(&init_tx_lock);
	
	if (!enable_vlan_mask) {
		spin_lock_bh(&rx_lock);

		ret = adm5120_dma_init_rings();
		if (ret)
			goto exit;

		_adm5120_up();

		tx_queues_stopped = 0;

		spin_unlock_bh(&rx_lock);
	}

	enable_vlan_mask |= priv->vlan_mask;

	/* enable ports of active vlans */
	adm5120_enable_ports(enable_vlan_mask);

	/* enable vlan */
	adm5120_set_vlan_mask(priv->vlan, vlan_matrix[priv->vlan]);

	/* set mac address */
	adm5120_write_mac(dev);

	if (tx_queues_stopped)
		netif_stop_queue(dev);
	else
		netif_start_queue(dev);
	
exit:
	spin_unlock_bh(&init_tx_lock);

	return ret;
}

static int adm5120_stop(struct net_device *dev)
{
	struct adm5120_sw *priv = dev->priv;

	spin_lock_bh(&init_tx_lock);

	if (!tx_queues_stopped)
		netif_stop_queue(dev);

	enable_vlan_mask &= ~priv->vlan_mask;

	/* enable ports of active vlans */
	adm5120_enable_ports(enable_vlan_mask);
	/* disable vlan */
	adm5120_clear_vlan_mask(priv->vlan);


	if (!enable_vlan_mask) {
		spin_lock_bh(&rx_lock);

		del_timer(&tx_watchdog_timer);

		_adm5120_down();
		adm5120_dma_free_rings();

		spin_unlock_bh(&rx_lock);
	}

	spin_unlock_bh(&init_tx_lock);

	return 0;
}

static void adm5120_restart(void)
{
	int i;

	spin_lock_bh(&init_tx_lock);

	if (!enable_vlan_mask || !tx_queues_stopped)
		goto out;

	spin_lock_bh(&rx_lock);

	_adm5120_down();
	adm5120_dma_reinit_rings();
	_adm5120_up();

	spin_unlock_bh(&rx_lock);

	/* enable ports of active vlans */
	adm5120_enable_ports(enable_vlan_mask);

	for (i = 0; i < adm5120_nrdevs; i++) {
		if (enable_vlan_mask & (1 << i)) {
			/* enable vlan */
			adm5120_set_vlan_mask(i, vlan_matrix[i]);
			/* set mac address */
			adm5120_write_mac(adm5120_devs[i]);
		}
	}


	adm5120_wake_tx_queues();

out:
	spin_unlock_bh(&init_tx_lock);
}



static int adm5120_get_regs(char *buf)
{
	unsigned int reg;
	char *p = buf;

	for (reg = 0; reg <= 0x110; reg += 4)
		p += sprintf(p, "%3.2x: 0x%.8lx\n", reg, adm5120_get_reg(reg));

	return p - buf;
}

static int adm5120_get_state(char *buf)
{
	char *p = buf;

	p += sprintf(p, "      int_enabled: %d\n", int_enabled);
	p += sprintf(p, "tx_queues_stopped: %d\n", tx_queues_stopped);
	p += sprintf(p, " enable_vlan_mask: 0x%.2x\n", enable_vlan_mask);

	return p - buf;
}

static int adm5120_get_tx_ring(char *buf, struct tx_ring *ring)
{
	int i;
	volatile struct adm5120_dma *desc = ring->desc;
	struct sk_buff **skb = ring->skb;
	char *p = buf;

	p += sprintf(p, "    desc: 0x%.8p\n", ring->desc);
	p += sprintf(p, "     skb: 0x%.8p\n", ring->skb);
	p += sprintf(p, "num_desc: %d\n", ring->num_desc);
	p += sprintf(p, "   avail: %d\n", ring->avail);
	p += sprintf(p, "head_idx: %d\n", ring->head_idx);
	p += sprintf(p, "tail_idx: %d\n\n", ring->tail_idx);

	for (i = 0; i < ring->num_desc; ++i) {
		p += sprintf(p, "%4d: skb: 0x%.8p%s%s%s%s\n", i, 
			     skb[i],
			     desc[i].data & ADM5120_DMA_OWN ? " OWN" : "",
			     desc[i].data & ADM5120_DMA_RINGEND ? " RINGEND" : "",
			     i == ring->head_idx ? " head" : "",
			     i == ring->tail_idx ? " tail" : "");
	}

	return p - buf;
}

static int adm5120_get_rx_ring(char *buf, struct rx_ring *ring)
{
	int i;
	volatile struct adm5120_dma *desc = ring->desc;
	struct sk_buff **skb = ring->skb;
	char *p = buf;

	p += sprintf(p, "    desc: 0x%.8p\n", ring->desc);
	p += sprintf(p, "     skb: 0x%.8p\n", ring->skb);
	p += sprintf(p, "num_desc: %d\n", ring->num_desc);
	p += sprintf(p, "     idx: %d\n", ring->idx);

	for (i = 0; i < ring->num_desc; ++i) {
		p += sprintf(p, "%4d: skb: 0x%.8p%s%s%s\n", i, 
			     skb[i],
			     desc[i].data & ADM5120_DMA_OWN ? " OWN" : "",
			     desc[i].data & ADM5120_DMA_RINGEND ? " RINGEND" : "",
			     i == ring->idx ? " idx" : "" );
	}

	return p - buf;
}

static int proc_calc_metrics(char *page, char **start, off_t off,
				 int count, int *eof, int len)
{
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int adm5120_read_state_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;

	spin_lock_bh(&init_tx_lock);
	len = adm5120_get_state(page);
	spin_unlock_bh(&init_tx_lock);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int adm5120_read_regs_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len = adm5120_get_regs(page);
	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int adm5120_read_tx_hring_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;

	spin_lock_bh(&init_tx_lock);
	len = adm5120_get_tx_ring(page, &tx_h_ring);
	spin_unlock_bh(&init_tx_lock);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int adm5120_read_tx_lring_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;

	spin_lock_bh(&init_tx_lock);
	len = adm5120_get_tx_ring(page, &tx_l_ring);
	spin_unlock_bh(&init_tx_lock);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int adm5120_read_rx_hring_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;

	spin_lock_bh(&rx_lock);
	len = adm5120_get_rx_ring(page, &rx_h_ring);
	spin_unlock_bh(&rx_lock);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int adm5120_read_rx_lring_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int len;

	spin_lock_bh(&rx_lock);
	len = adm5120_get_rx_ring(page, &rx_l_ring);
	spin_unlock_bh(&rx_lock);

	return proc_calc_metrics(page, start, off, count, eof, len);
}

static int __init adm5120_init(void)
{
	int i, err;
	struct net_device *dev;

	printk("adm5120 switch driver version 1.1\n");
	printk(PFX "initialized with %d vlans.\n", adm5120_nrdevs);
	printk(PFX "using NAPI.\n" ); 
	printk(PFX "ring sizes:\n" );
	printk(PFX "rxh = %d, rxl = %d\n", ADM5120_DMA_RXH, ADM5120_DMA_RXL );
	printk(PFX "txh = %d, txl = %d\n", ADM5120_DMA_TXH, ADM5120_DMA_TXL );

	adm5120_disable_int();
	err = request_irq(SW_IRQ, adm5120_irq, 0, "ethernet switch", NULL);
	if (err) {
		printk(KERN_INFO PFX "error: can't register interrupt handler\n");
		goto out;
	}

	make_port2vlan();

	init_timer(&tx_watchdog_timer);
	tx_watchdog_timer.data = (unsigned long) 0;
	tx_watchdog_timer.function = tx_watchdog;

	for (i = 0; i < adm5120_nrdevs; i++) {
		dev = alloc_etherdev(sizeof(struct adm5120_sw));
		if (!dev) {
			err = -ENOMEM;
			goto out_int;
		}
		adm5120_devs[i] = dev;

		strcpy(dev->name, IFACE_NAME_PFX "%d");
		
		SET_MODULE_OWNER(dev);
		memset(dev->priv, 0, sizeof(struct adm5120_sw));
		((struct adm5120_sw*)dev->priv)->vlan = i;
		((struct adm5120_sw*)dev->priv)->vlan_mask = (u32) 1 << i;
		dev->base_addr = SW_BASE;
		dev->irq = SW_IRQ;

		dev->open = adm5120_open;
		dev->stop = adm5120_stop;
		dev->hard_start_xmit = adm5120_start_xmit;

		dev->get_stats = adm5120_stats;
		dev->set_multicast_list = adm5120_set_multicast_list;
		dev->set_mac_address = adm5120_set_mac_address;
		dev->do_ioctl = adm5120_do_ioctl;

#if 0
		dev->tx_timeout = adm5120_tx_timeout;
		dev->watchdog_timeo = ETH_TX_TIMEOUT;
#endif
		/* HACK alert!!!  In the original admtek driver it is asumed
		   that you can read the MAC addressess from flash, but edimax
		   decided to leave that space intentionally blank...
		 */
/*		memcpy(dev->dev_addr, "\x00\x50\xfc\x11\x22\x01", 6);
		dev->dev_addr[5] += i;*/
	        memcpy(dev->dev_addr, default_macaddrs[i], 6);
		
		err = register_netdev(dev);
		if (err) {
		        unregister_netdev(dev);
			goto out_int;
		}
	}

	create_proc_read_entry("adm5120sw-regs", 0, NULL, adm5120_read_regs_proc, NULL);
	create_proc_read_entry("adm5120sw-state", 0, NULL, adm5120_read_state_proc, NULL);
	create_proc_read_entry("adm5120sw-txh", 0, NULL, adm5120_read_tx_hring_proc, NULL);
	create_proc_read_entry("adm5120sw-txl", 0, NULL, adm5120_read_tx_lring_proc, NULL);
	create_proc_read_entry("adm5120sw-rxh", 0, NULL, adm5120_read_rx_hring_proc, NULL);
	create_proc_read_entry("adm5120sw-rxl", 0, NULL, adm5120_read_rx_lring_proc, NULL);
	

	return 0;

out_int:
	/* Undo everything that did succeed */
	while (i--)
		unregister_netdev(adm5120_devs[i]);

	free_irq(SW_IRQ, NULL);
out:
	printk(KERN_ERR PFX "init failed.\n");
	return err;
}

static void __exit adm5120_exit(void)
{
	int i;

	remove_proc_entry("adm5120sw-regs", NULL);
	remove_proc_entry("adm5120sw-state", NULL);
	remove_proc_entry("adm5120sw-txh", NULL);
	remove_proc_entry("adm5120sw-txl", NULL);
	remove_proc_entry("adm5120sw-rxh", NULL);
	remove_proc_entry("adm5120sw-rxl", NULL);

	for (i = 0; i < adm5120_nrdevs; i++) {
		unregister_netdev(adm5120_devs[i]);
	}

	free_irq(SW_IRQ, NULL);

}

module_init(adm5120_init);
module_exit(adm5120_exit);
