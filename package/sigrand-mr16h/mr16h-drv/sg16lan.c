/* sg16lan.c:  Sigrand SG-16PCI SHDSL modem driver for linux (kernel 2.6.x)
 *
 *	Written 2005-2006 by Artem U. Polyakov (artpol@sigrand.ru)
 *
 *	This driver presents SG-16PCI modem 
 *	to system as common ethernet-like netcard.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 *
 *	10.11.2005	initial revision of Granch SBNI16 modem driver v1.0 
 *                      wtitten by Denis I. Timofeev
 *	11.12.2005	Version 2.0 ( sysfs, firmware hotplug support )
 */

#include "cx28975.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <net/arp.h>
#include <linux/pci.h>
#include <linux/random.h>
#include<linux/firmware.h>
#include<linux/dma-mapping.h>
#include <linux/vermagic.h>
#include <linux/config.h>

#include <asm/tlbdebug.h>

MODULE_DESCRIPTION( "Sigrand SG-16PCI driver Version 2.0\n" );
MODULE_AUTHOR( "Maintainer: Polyakov Artem artpol@sigrand.ru\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION("2.0");

/* -------------------------------------------------------------------------- */

/* CR bits */
#define	TXEN	0x01		/* transmitter enable */
#define	RXEN	0x02		/* receiver  enable */
#define	NCRC	0x04		/* ignore received CRC */
#define	DLBK	0x08		/* digital loopback */
#define CMOD	0x10		/* 0 - use CRC-32, 1 - CRC-16 */
#define FMOD	0x20		/* interframe fill: 0 - all ones, 1 - 0xfe */
#define PMOD	0x40		/* data polarity: 0 - normal, 1 - invert */
#define XRST	0x80		/* reset the transceiver */

/* CRB bits */
#define RDBE	0x01		/* read burst enable */
#define WTBE	0x02		/* write burst enable */
#define RODD	0x04		/* receive 2-byte alignment */
#define RXDE	0x08		/* receive data enable */

/* SR and IMR bits */
#define	TXS	0x01		/* transmit success */
#define	RXS	0x02		/* receive success */
/* SR only */
#define	CRC	0x04		/* CRC error */
#define	OFL	0x08		/* fifo overflow error */
#define	UFL	0x10		/* fifo underflow error */
#define	EXT	0x20		/* interrupt from sk70725 */
/* IMR only */
#define	TSI	0x80		/* generate test interrupt */

#define LAST_FRAG 0x00008000

/* We don't have official vendor id yet... */
#define SG16_PCI_VENDOR 0x55 
#define SG16_PCI_DEVICE 0x9d

// Frame parameters
#define SG16_MAX_FRAME 1536

// Send timeout
#define TX_TIMEOUT	400

/* SHDSL parameters*/
#define MAX_AUTO_RATE	2304
#define MIN_AUTO_RATE	192
#define MAX_RATE	6016
#define MIN_RATE	64
#define MAX_REMCFGF_RATE	5696
#define MAX_REMCFGAB_RATE	2304
#define MIN_REMCFG_RATE		192


#define ANNEX_A		0x01
#define ANNEX_B		0x02
#define ANNEX_F		0x03


/*Debug parameters*/
//#define DEBUG_ON
#define PDEBUG(fmt,args...) 
#ifdef DEBUG_ON
#	undef PDEBUG
#	define PDEBUG(fmt,args...) \
		    printk(KERN_NOTICE "sg16lan.c: %s " fmt " \n", __FUNCTION__, ## args  )
#endif

static int dnum=0;
struct net_device *devs[10];

//static int FLG=0;

/* Internal consts */
#define EFWDLOAD 0x20
#define iotype u8*

/* SG-16PCI ioctl params */
#define SIOCDEVLOADFW	 	SIOCDEVPRIVATE
#define SIOCDEVGETSTATS	 	SIOCDEVPRIVATE+1
#define SIOCDEVCLRSTATS	 	SIOCDEVPRIVATE+2

/* Portability */
//#define IO_READ_WRITE
#ifndef IO_READ_WRITE
#	define iowrite8(val,addr)  writeb(val,addr)
#	define iowrite32(val,addr)  writel(val,addr)
#	define ioread8(addr) readb(addr)
#	define ioread32(addr) readl(addr)
#endif
/* -------------------------------------------------------------------------- */

struct sg16_hw_regs {
	u8  CRA, CRB, SR, IMR, CTDR, LTDR, CRDR, LRDR;
};

struct hw_descr {
	u32  address;
	u32  length;
};

struct cx28975_cmdarea {
	u8  intr_host;
	u8  intr_8051;
	u8  map_version;

	u8  in_dest;
	u8  in_opcode;
	u8  in_zero;
	u8  in_length;
	u8  in_csum;
	u8  in_data[ 75 ];
	u8  in_datasum;

	u8  out_dest;
	u8  out_opcode;
	u8  out_ack;
	u8  out_length;
	u8  out_csum;
	u8  out_data[ 75 ];
	u8  out_datasum;
};

#define XQLEN	8
#define RQLEN	8

/* net_device private data */

#define FW_NAME_SIZE 255
struct shdsl_config
{
    char fw_name[FW_NAME_SIZE];
    u16 lrate:	10;
    u16 master:	1;
    u16 mod:	2;
    u16 autob:	1;
    u16 autob_en: 1;    
    u16 need_preact: 1;
    u8 remcfg :1;
    u8 annex :2;
    u8 :5;
};

struct hdlc_config
{
    u8  crc16: 1;
    u8  fill_7e: 1;
    u8  inv: 1;
    u8  rburst: 1;
    u8  wburst: 1;
};

struct net_local{
    
    struct net_device_stats	stats;
    wait_queue_head_t  wait_for_intr;


    struct device *dev;
    /*Configuration structures*/
    struct hdlc_config hdlc_cfg;
    struct shdsl_config shdsl_cfg;
    u8 irqret;

    /* SG-16PCI controller statistics */
    struct sg16_stats {
    	u32  sent_pkts, rcvd_pkts;
	u32  crc_errs, ufl_errs, ofl_errs, last_time;
    } in_stats;

    spinlock_t rlock,xlock;
    
    void *mem_base;		/* mapped memory address */

    volatile struct sg16_hw_regs	*regs;
    volatile struct hw_descr	*tbd;
    volatile struct hw_descr	*rbd;
    volatile struct cx28975_cmdarea	*cmdp;

    /* transmit and reception queues */
    struct sk_buff *xq[ XQLEN ], *rq[ RQLEN ];
    unsigned head_xq, tail_xq, head_rq, tail_rq;

    /* the descriptors mapped onto the first buffers in xq and rq */
    unsigned head_tdesc, head_rdesc;
    u8 fw_state;
    
    /* deffered link check */
    struct work_struct wqueue;    
};

/* SHDSL transceiver statistics */
struct dsl_stats {
	u8	status_1, status_3;
	u8	attenuat, nmr, tpbo, rpbo;
	u16	losw, segd, crc, sega, losd;
	u16	all_atmpt,atmpt;
};

/*- Driver initialisation -*/
static int  sg16_init( void );
static void sg16_exit( void );
module_init(sg16_init);
module_exit(sg16_exit);
static int __devinit  sg16_init_one( struct pci_dev *,
					const struct pci_device_id * );
static void __devexit  sg16_remove_one( struct pci_dev * );

/*- Net device specific functions -*/
static int __init  sg16_probe( struct net_device * );
static irqreturn_t  sg16_interrupt( int, void *, struct pt_regs * );
static int  sg16_open( struct net_device * );
static int  sg16_close( struct net_device * );
static int  sg16_start_xmit( struct sk_buff *, struct net_device * );
static struct net_device_stats  *sg16_get_stats( struct net_device * );
static void  set_multicast_list( struct net_device * );

/*-----------------------------------------------------------------------------

static int  sg16_ioctl( struct net_device *, struct ifreq *, int );

/*-----------------------------------------------------------------------------*/

/*- Functions serving SG-16PCI control -*/

static void hdlc_init( struct net_local *nl);
static void hdlc_shutdown( struct net_local *nl );

static int  shdsl_ready( struct net_local *nl, u16 expect_state);

/*-----------------------------------------------------------------------------*/

//static int  shdsl_dload_fw(struct device *dev);
static int  shdsl_dload_fw(struct net_device *dev, struct firmware *fw);

/*-----------------------------------------------------------------------------*/

static int  shdsl_preactivation(struct net_local *nl);
static int  shdsl_get_stat(struct net_local *nl, struct dsl_stats *ds);
static int  shdsl_clr_stat( struct net_local  *nl );
static int  shdsl_issue_cmd( struct net_local *, u8, u8 *, u8 );
static void shdsl_interrupt( struct net_device * );
static void shdsl_link_chk( void * );


#ifdef SG16_USERMODE_EVENTS
#define USERMODE_HELPER "/sbin/__sg16_link_hndl"
static int usermode_link_event(struct net_device *ndev,int link_up);

#else

static inline int
usermode_link_event(struct net_device *ndev,int link_up){ return 0; }

#endif


/*- Functions, serving transmit-receive process -*/
static void  recv_init_frames( struct net_device * );
static void  recv_alloc_buffs( struct net_device * );
static void  recv_free_buffs( struct net_device * );
static void  xmit_free_buffs( struct net_device * );
static void  sg16_tx_timeout( struct net_device * );

/*- Sysfs specific functions -*/

#define ADDIT_ATTR
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
#undef ADDIT_ATTR
#define ADDIT_ATTR struct device_attribute *attr,
#endif    

static int init_sg16_in_sysfs( struct device *);
static void del_sg16_from_sysfs(struct device *dev);
// shdsl attribs

static ssize_t show_rate( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_rate( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(rate,0644,show_rate,store_rate);	

static ssize_t show_crate( struct device *dev, ADDIT_ATTR char *buff );
static DEVICE_ATTR(crate,0644,show_crate,NULL);

static ssize_t show_master( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_master( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(master,0644,show_master,store_master);	

static ssize_t show_remcfg( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_remcfg( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(remcfg,0644,show_remcfg,store_remcfg);	

static ssize_t show_annex( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_annex( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(annex,0644,show_annex,store_annex);	

static ssize_t show_mod( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_mod( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(mod,0644,show_mod,store_mod);	

static ssize_t show_autob( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_autob( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(autobaud,0644,show_autob,store_autob);	

static ssize_t show_download( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_download( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(download,0644,show_download,store_download);	

// hdlc attribs 
static ssize_t show_crc16( struct device *dev, ADDIT_ATTR char *buff );
static ssize_t store_crc16( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(crc16,0644,show_crc16,store_crc16);	

static ssize_t show_fill_7e( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_fill_7e( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(fill_7e,0644,show_fill_7e,store_fill_7e);	

static ssize_t show_inv( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_inv( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(inv,0644,show_inv,store_inv);	

static ssize_t show_rburst( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_rburst( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(rburst,0644,show_rburst,store_rburst);

static ssize_t show_wburst( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_wburst( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(wburst,0644,show_wburst,store_wburst);	

//address
static ssize_t store_maddr( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(maddr,0200,NULL,store_maddr);	

// Statistic
static ssize_t show_dev_state( struct device *dev, ADDIT_ATTR char *buff ); 
static DEVICE_ATTR(state,0644,show_dev_state,NULL);	

static ssize_t show_statistic( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_statistic( struct device *dev, ADDIT_ATTR const char *buff, size_t size ); 
static DEVICE_ATTR(statistic,0644,show_statistic,store_statistic);	


//debug
static ssize_t show_debug( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_debug( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(debug,0644,show_debug,store_debug);	

static ssize_t show_skb_stat(struct device *dev, ADDIT_ATTR char *buf );
static DEVICE_ATTR(skb_stat,0444,show_skb_stat,NULL);	



//---- Memory leak bug catch ----//

static spinlock_t rx_alloc_lock,rx_free_lock;
static int rx_buffs_alloc=0, rx_buffs_free=0;
static spinlock_t tx_alloc_lock,tx_free_lock;
static int tx_buffs_alloc=0, tx_buffs_free=0;
static unsigned long tmp_flags;
static int less_than_ethmin = 0;

#define ALLOC_RX() \
spin_lock_irqsave(&rx_alloc_lock,tmp_flags); \
rx_buffs_alloc++; \
spin_unlock_irqrestore(&rx_alloc_lock,tmp_flags);

#define ALLOC_TX() \
spin_lock_irqsave(&tx_alloc_lock,tmp_flags); \
tx_buffs_alloc++; \
spin_unlock_irqrestore(&tx_alloc_lock,tmp_flags);

#define FREE_RX() \
spin_lock_irqsave(&rx_free_lock,tmp_flags); \
rx_buffs_free++; \
spin_unlock_irqrestore(&rx_free_lock,tmp_flags);

#define FREE_TX() \
spin_lock_irqsave(&tx_free_lock,tmp_flags); \
tx_buffs_free++; \
spin_unlock_irqrestore(&tx_free_lock,tmp_flags);

//-------------------------------//


/* pci-driver initialisation block */
/*----------------------------------------------------------------------------*/

#define SG16_PCI_VENDOR 	0x55 
#define SG16_PCI_DEVICE 	0x9d
static struct pci_device_id  sg16_pci_tbl[] __devinitdata = {
	{ PCI_DEVICE(SG16_PCI_VENDOR,SG16_PCI_DEVICE) },
	{ 0 }
};
MODULE_DEVICE_TABLE( pci, sg16_pci_tbl );

static struct pci_driver  sg16_driver = {
	name:		"sg16lan",
	probe:		sg16_init_one,
	remove:		sg16_remove_one,
	id_table:	sg16_pci_tbl
};

int
sg16_init( void ){
    spin_lock_init( &rx_alloc_lock);
    spin_lock_init( &rx_free_lock );
    spin_lock_init( &tx_alloc_lock );
    spin_lock_init( &tx_free_lock );

    pci_module_init( &sg16_driver );
    return 0;
}

void
sg16_exit( void ){
    pci_unregister_driver( &sg16_driver );
}

void
dsl_init( struct net_device *ndev)
{
    ether_setup(ndev);    
    ndev->init = sg16_probe;
}


static int __devinit
sg16_init_one( struct pci_dev  *pdev,  const struct pci_device_id  *ent )
{
    struct net_device  *ndev;
    struct device *dev_dev=(struct device*)&(pdev->dev);
    struct device_driver *dev_drv=(struct device_driver*)(dev_dev->driver);
    struct net_local * nl;
    u8 err;

    if( pci_enable_device( pdev ) )
    	return  -EIO;
    pci_set_master(pdev); 
    /* register network device */
	if( !( ndev = alloc_netdev( sizeof(struct net_local),"dsl%d",dsl_init)) )
	    return  -ENOMEM;
    /* set some net device fields */
    pci_set_drvdata( pdev, ndev );
    ndev->mem_start = pci_resource_start( pdev, 1 );
    ndev->mem_end = pci_resource_end( pdev,1 );
    ndev->irq = pdev->irq;

    /* network interface initialisation */
    if( register_netdev( ndev ) ) {
	err=ENODEV;
	goto err1;
    }
    devs[dnum]=ndev;
    dnum++;    
    /* device private data initialisation */
    nl=(struct net_local *)netdev_priv( ndev);
    nl->dev=dev_dev;
    /* shutdown device before startup*/
    hdlc_shutdown(nl);

    /* Create symlink from driver dir to device dir in sysfs  */
    sysfs_create_link( &(dev_drv->kobj),&(dev_dev->kobj),ndev->name );

    /* Create sysfs entires */
    if( init_sg16_in_sysfs( dev_dev ) )
    {
        printk( KERN_ERR "%s: unable to create sysfs entires\n",ndev->name);
        err=-EPERM;	    
        goto err2;
    }

    nl->shdsl_cfg.need_preact=1;
/*
#ifndef AUTOSTART_OFF

    if( shdsl_dload_fw(dev_dev)  )
	printk(KERN_NOTICE"%s: cannot download firmware\n",ndev->name);
    else
    {
	// check that autobaud is aviable in this firmware 
	if( shdsl_preactivation(nl) )
	    printk(KERN_ERR"%s, I/O error\n",ndev->name);
	else
	{
	    // Starting device activation 
            t=0x42;
	    if( ( err=shdsl_issue_cmd(nl,_DSL_ACTIVATION,&t,1) ) )
    		return -EIO;
	    nl->shdsl_cfg.need_preact=0;
	}
    }
#endif	
/*
    /* timered link chk entire */
    INIT_WORK( &nl->wqueue,shdsl_link_chk,(void*)ndev);    
    
    return  0;

err2:
    unregister_netdev( ndev );
    free_irq( ndev->irq, ndev );
    release_mem_region( ndev->mem_start, 0x1000 );
    iounmap( ((struct net_local *)ndev->priv)->mem_base );
err1:	    
    free_netdev(ndev);    
    return err;
}

static void __devexit
sg16_remove_one( struct pci_dev  *pdev )
{
    struct net_device  *ndev = pci_get_drvdata( pdev );
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);    
    struct device *dev_dev=(struct device*)&(pdev->dev);
    struct device_driver *dev_drv=(struct device_driver*)(dev_dev->driver);

    /* timer entry */
    cancel_delayed_work(&nl->wqueue);

    /* shutdown device */
    hdlc_shutdown(nl);
    
    /* Remove symlink on device from driver dir in sysfs */
    sysfs_remove_link(&(dev_drv->kobj),ndev->name);
    /* Remove sysfs entires */
    del_sg16_from_sysfs(dev_dev);

    /* Remove network device from OS */
    unregister_netdev( ndev );
    free_irq( ndev->irq, ndev );
    release_mem_region( ndev->mem_start, 0x1000 );
    iounmap( ((struct net_local *)ndev->priv)->mem_base );
    free_netdev( ndev );
}

/* Network interface specific functions */
/* -------------------------------------------------------------------------- */

static int __init
sg16_probe( struct net_device  *ndev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);

    // Carrier off
    netif_carrier_off( ndev );
    
    /* generate 'unique' MAC address */
    *(u16 *)ndev->dev_addr = htons( 0x00ff );
    *(u32 *)(ndev->dev_addr + 2) = htonl( 0x01a39000 | ((u32)ndev->priv & 0x00000fff) );

    /* Init net device handler functions */
    ndev->open		= &sg16_open;
    ndev->stop		= &sg16_close;
    ndev->hard_start_xmit	= &sg16_start_xmit;
    ndev->get_stats		= &sg16_get_stats;
    ndev->set_multicast_list	= &set_multicast_list;
    ndev->tx_timeout		= &sg16_tx_timeout;
    ndev->watchdog_timeo	= TX_TIMEOUT;
//    ndev->do_ioctl           	= &sg16_ioctl;
	    

    if( !request_mem_region( ndev->mem_start, 0x1000, ndev->name ) )
	return  -ENODEV;

    /* set network device private data */
    memset( nl, 0, sizeof(struct net_local) );
    spin_lock_init( &nl->rlock );
    spin_lock_init( &nl->xlock );
    init_waitqueue_head( &nl->wait_for_intr );
    nl->mem_base = (void *) ioremap( ndev->mem_start, 0x1000 );
    nl->tbd  = (struct hw_descr *) nl->mem_base;
    nl->rbd  = (struct hw_descr *) ((u8 *)nl->mem_base + 0x400);
    nl->regs = (struct sg16_hw_regs *) ((u8 *)nl->mem_base + 0x800);
    nl->cmdp = (struct cx28975_cmdarea *) ((u8 *)nl->mem_base + 0xc00);
    memset( &nl->in_stats, 0, sizeof(struct sg16_stats) );    

    if( request_irq(ndev->irq, sg16_interrupt, SA_SHIRQ, ndev->name, ndev) ) 
    {
        printk( KERN_ERR "%s: unable to get IRQ %d.\n",
    		ndev->name, ndev->irq );
        goto err_exit;
    }
	
    printk( KERN_NOTICE "%s: Sigrand SG-16PCI SHDSL (irq %d, mem %#lx)\n",
		ndev->name, ndev->irq, ndev->mem_start );
    SET_MODULE_OWNER( ndev );

    return  0;

err_exit:
    iounmap( nl->mem_base );
    release_mem_region( ndev->mem_start, 0x1000 );
    return  -ENODEV;
}

static irqreturn_t
sg16_interrupt( int  irq,  void  *dev_id,  struct pt_regs  *regs )
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct net_local  *nl  = (struct net_local *)netdev_priv(dev);		
	u8  status;
	u8  mask = ioread8((iotype)&(nl->regs->IMR));	

	if( (mask & ioread8((iotype)&(nl->regs->SR)) ) == 0 )
		return IRQ_NONE;

	status = ioread8((iotype)&(nl->regs->SR));
	iowrite8(status,(iotype)&(nl->regs->SR));
	iowrite8(0,(iotype)&(nl->regs->IMR));	
	
	if( status & EXT ){
	        shdsl_interrupt( dev );
	}
	/*
	 * Whether transmit error is occured, we have to re-enable the
	 * transmitter. That's enough, because linux doesn't fragment
	 * packets.
	 */
	if( status & UFL ){
		iowrite8( ioread8((iotype)&(nl->regs->CRA)) | TXEN,
	    	        (iotype)&(nl->regs->CRA) );
		++nl->in_stats.ufl_errs;
		++nl->stats.tx_errors;
		++nl->stats.tx_fifo_errors;
	}
	if( status & RXS ){
		if( spin_trylock( &(nl->rlock) ) ){
			recv_init_frames( dev );
			recv_alloc_buffs( dev );
			spin_unlock( &(nl->rlock) );
		}
	}
	if( status & TXS ){
	        xmit_free_buffs( dev );
	}
	if( status & CRC ){
		++nl->in_stats.crc_errs;
		++nl->stats.rx_errors;
    		++nl->stats.rx_crc_errors;
	}
	if( status & OFL ){
		++nl->in_stats.ofl_errs;
		++nl->stats.rx_errors;
		++nl->stats.rx_over_errors;
	}

	PDEBUG("%02x end",status & 0xff);
	iowrite8(mask,(iotype)&(nl->regs->IMR));	
	return IRQ_HANDLED;
}

/* Open/initialize the board */
static int
sg16_open( struct net_device  *ndev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
    int err;
    u8 t;    
    /* shdsl preactivation */
    if( nl->shdsl_cfg.need_preact )
    {
/*TODO: is it correctly just reset SHDLS without 
	deactivation?
*/
	iowrite8( EXT, (iotype)&(nl->regs->IMR) ); 
        iowrite8( 0xff, (iotype)&(nl->regs->SR) ); 
	iowrite8( XRST , (iotype)&(nl->regs->CRA));       			    
	iowrite8( RXDE , (iotype)&(nl->regs->CRB));       			    
	t=0;
	if( shdsl_issue_cmd( nl, _DSL_RESET_SYSTEM, &t, 1 ) )
    	    return -EBUSY;

	if( !shdsl_ready(nl, _ACK_OPER_WAKE_UP )  )
	{
		printk(KERN_NOTICE"%s, firmware wasn't loaded\n",ndev->name);	    
		return -EBUSY;
	}
	if( shdsl_preactivation(nl) )
	{
		printk(KERN_ERR"%s, I/O error\n",ndev->name);
		return -EBUSY;
	}
        /* Starting device activation */
	t=0x42;
        if( ( err=shdsl_issue_cmd(nl,_DSL_ACTIVATION,&t,1) ) )
	    return -EIO;
	nl->shdsl_cfg.need_preact=0;	
    }
    
    /* init descripts, allocate receiving buffers */
    nl->head_xq = nl->tail_xq = nl->head_rq = nl->tail_rq = 0;
    nl->head_tdesc = nl->head_rdesc = 0;
    iowrite8( 0, (iotype)&(nl->regs->CTDR));
    iowrite8( 0, (iotype)&(nl->regs->LTDR));
    iowrite8( 0, (iotype)&(nl->regs->CRDR));
    iowrite8( 0, (iotype)&(nl->regs->LRDR));
    recv_alloc_buffs( ndev );
    /* enable receive/transmit functions */
    hdlc_init(nl);
    memset( &nl->stats, 0, sizeof(struct net_device_stats) );
    return 0;
}

static int
sg16_close( struct net_device  *ndev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);
    unsigned long  flags;    

    /* disable receive/transmit functions */        
    iowrite8( XRST ,(iotype)&(nl->regs->CRA));       			    
    /* drop receive/transmit queries */
    spin_lock_irqsave( &nl->rlock, flags );
    recv_free_buffs( ndev );
    spin_unlock_irqrestore( &nl->rlock, flags );

    iowrite8( ioread8( (iotype)&(nl->regs->LTDR) ), ( iotype)&(nl->regs->CTDR) );
    xmit_free_buffs( ndev );
    return 0;
}


static struct net_device_stats *
sg16_get_stats( struct net_device  *dev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(dev);		
	return  &(nl)->stats;
}

static void
set_multicast_list( struct net_device  *dev )
{
	return;		/* SG-16PCI always operate in promiscuos mode */
}


struct cx28975_fw {
    u8   *firmw_image;
    u32  firmw_len;
};
	
/*
static int
sg16_ioctl( struct net_device  *dev,  struct ifreq  *ifr,  int  cmd )
{
    struct firmware fw;
    u8 mas[100*1024];
    struct cx28975_fw fw_in;
    int  error = 0;
    int err;
		
    if( cmd ==  SIOCDEVLOADFW ){
		    
        if( current->euid != 0 )        /* root only */
/*	    return  -EPERM;
        if( (dev->flags & IFF_UP) == IFF_UP )
	    return  -EBUSY;
	PDEBUG("verify_area for addr");			    
        if( (error = verify_area( VERIFY_READ, ifr->ifr_data,
		            sizeof(struct cx28975_fw) )) != 0 )
	    return  error;
	PDEBUG("cp to fw_in");
        copy_from_user( &fw_in, ifr->ifr_data, sizeof fw_in );
	PDEBUG("verify_area for fw");		
        if( (error = verify_area( VERIFY_READ, fw_in.firmw_image,
                      fw_in.firmw_len )) != 0 )
	    return  error;
/*
        if( !(fw.data = kmalloc( fw_in.firmw_len, GFP_KERNEL )) )
	      return  -ENOMEM;
*/
/*	fw.data=mas;
	PDEBUG("kmalloced");
	copy_from_user( fw.data, fw_in.firmw_image, fw_in.firmw_len );
	fw.size= fw_in.firmw_len;
	printk("%s: buf allocated and filled, addr=%08x\n",fw.data); 	
	PDEBUG("start dload");	
	if( shdsl_dload_fw(dev, &fw ) )
	    error=-1;
	kfree( fw.data );
    }
    return  error;
}
*/																																      

/* Control device functions */
/*----------------------------------------------------------------------------*/

static void
hdlc_shutdown(struct net_local *nl)
{
    iowrite8( 0, (iotype)&( nl->regs->CRA));    
    iowrite8( RXDE , (iotype)&( nl->regs->CRB));
    iowrite8( 0, (iotype)&( nl->regs->IMR));
    iowrite8( 0xff, (iotype)&( nl->regs->SR));
}

static void
hdlc_init( struct net_local *nl)
{
    u8 cfg_byte;

    cfg_byte=XRST | RXEN | TXEN;			
    if( nl->hdlc_cfg.crc16 )		
        cfg_byte|=CMOD;			
    if( nl->hdlc_cfg.fill_7e )		
        cfg_byte|=FMOD;			
    if( nl->hdlc_cfg.inv )		
        cfg_byte|=PMOD;			
    iowrite8(cfg_byte,(iotype)&(nl->regs->CRA));

    cfg_byte=ioread8( (iotype)&(nl->regs->CRB)) | RODD;		
    if( nl->hdlc_cfg.rburst )		
        cfg_byte|=RDBE;			
    if( nl->hdlc_cfg.wburst )		
        cfg_byte|=WTBE;			
    iowrite8(cfg_byte,(iotype)&(nl->regs->CRB)); 
}


static int 
shdsl_ready( struct net_local *nl, u16 expect_state)
{
#ifdef DEBUG_ON
    volatile struct cx28975_cmdarea  *p = nl->cmdp;
#endif
    u8 ret_val=1;
    u32 ret;
    PDEBUG("start");
    if( (nl->irqret & 0x1f) != expect_state ){
	ret=interruptible_sleep_on_timeout( &nl->wait_for_intr, HZ*10 );
        PDEBUG("after ret");	
	if( ( nl->irqret & 0x1f) != expect_state ){
	    PDEBUG("fail wait, irqret=%02x expect=%02x",nl->irqret & 0x1f,0xff & expect_state );
	    ret_val=0;
	}
    }
    nl->irqret=0;
    PDEBUG("end");    
    return ret_val;
}

static int
shdsl_dload_fw(struct net_device *ndev, struct firmware *fw)
{
//    struct net_device *ndev= (struct net_device *)dev_get_drvdata(dev);
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);			
    struct shdsl_config *cfg = (struct shdsl_config *)&(nl->shdsl_cfg);
    volatile struct cx28975_cmdarea  *p = nl->cmdp;
//    struct firmware *fw;
    size_t img_len;
    u32   t;
    int  i;
    u8 cksum = 0;

    PDEBUG("%s before loading",ndev->name);
    
    for(i=0;i< fw->size;i++)
        cksum += fw->data[i];
				    

/* 1.Prepare to download process */
    hdlc_shutdown(nl);
    udelay(10);
    iowrite8( 0, (iotype)&(p->intr_host) );
    ioread8( (iotype)&(p->intr_host) );    
    iowrite8(XRST,(iotype)&(nl->regs->CRA));
    iowrite8(EXT,(iotype)&(nl->regs->IMR)); 
    udelay(10);
    PDEBUG("%s: hdlc settings",ndev->name);    
    if( !shdsl_ready(nl,_ACK_BOOT_WAKE_UP) )
	goto err_exit;

    PDEBUG("%s: before dload start",ndev->name);
/* 2.Download process */
    t = fw->size;
    if( shdsl_issue_cmd( nl, _DSL_DOWNLOAD_START, (u8 *) &t, 4 ) )
    	goto err_exit;
    PDEBUG("%s: dload start",ndev->name);
    
    for( i = 0, img_len=fw->size;  img_len >= 75;  i += 75, img_len -= 75 ){
	
	if( shdsl_issue_cmd( nl, _DSL_DOWNLOAD_DATA, fw->data + i, 75 ) ){
	    printk("%s: %s cmd error on %d byte\n",__FUNCTION__,ndev->name,i);                
    	    goto err_exit;
	}
    }

    PDEBUG("%s: dload tail",ndev->name);
    if( img_len
        &&  shdsl_issue_cmd( nl, _DSL_DOWNLOAD_DATA, fw->data + i, img_len ) )
    	goto err_exit;
    PDEBUG("%s: dload end",ndev->name);
    t = (cksum ^ 0xff) + 1;
    if( shdsl_issue_cmd( nl, _DSL_DOWNLOAD_END, (u8 *) &t, 1 ) )
    	goto err_exit;
    PDEBUG("%s: dload complete",ndev->name);
/* 3.Check that donload is successfull */
    if( !shdsl_ready(nl,_ACK_OPER_WAKE_UP) ){
	PDEBUG("no _ACK_OPER_WAKE_UP");	
	goto err_exit;
    }
    PDEBUG("%s: dload successfull",ndev->name);	
    
/* 4. Check that auto rate selection supported */
    udelay(10);
    t=0;
    if( shdsl_issue_cmd( nl,_DSL_VERSIONS, (u8 *)&t, 1 ) )
	goto err_exit;    

    if( !( ioread8( (iotype)&(p->out_data) + 4 ) & 1 ) )
    {
        cfg->autob_en=0;
	cfg->autob=0;
    }
    else
	cfg->autob_en=1;

    PDEBUG("%s: %s exit\n",__FUNCTION__,ndev->name);
    return  0;

err_exit:
    PDEBUG("%s: %s err exit\n",__FUNCTION__,ndev->name);
    return -EIO;
}

static int
shdsl_preactivation(struct net_local *nl)
{
    static char  thresh[] = { +8, -4, -16, -40 };
    struct shdsl_config *cfg = (struct shdsl_config*)&(nl->shdsl_cfg);
    u8 t, parm[ 36 ];
    u8 iter;
    u16 tmp;
    u32 max_rate=0 ,min_rate=0;

/* Start preactivation process */    

//----_DSL_SYSTEM_ENABLE----//

    t = cfg->master ? 1 : 9;
    if( shdsl_issue_cmd( nl, _DSL_SYSTEM_ENABLE, &t, 1 ) )
    	return  -EIO;
	
//----_DSL_SYSTEM_CONFIG----//
	
    t = 0x63;
    if( shdsl_issue_cmd( nl, _DSL_SYSTEM_CONFIG, &t, 1 ) )
    	return  -EIO;

//---- _DSL_MULTI_RATE_CONFIG----//

    // Check rate value
    if( cfg->autob ){
	max_rate=MAX_AUTO_RATE;
	min_rate=MIN_AUTO_RATE;
    }else if( cfg->remcfg ){
	if( cfg->master ){
	    if( cfg->annex==ANNEX_F )
		max_rate=MAX_REMCFGF_RATE;
	    else if( cfg->master )
		max_rate=MAX_REMCFGAB_RATE;
	    min_rate=MIN_REMCFG_RATE;
	}
    }else{
	max_rate=MAX_RATE;
	min_rate=MIN_RATE;
    }

    if( max_rate ){
	tmp=(min_rate >> 3) & 0x3ff;
	cfg->lrate= ( cfg->lrate < tmp ) ? tmp : cfg->lrate;
	tmp=(max_rate >> 3) & 0x3ff;
	cfg->lrate= ( cfg->lrate > tmp ) ? tmp : cfg->lrate;
    }
    else
	cfg->lrate= 192 >> 3;

    memset( parm, 0, 8 );
    *(u16 *)parm = ( cfg->lrate >> 3 ) & 0x7f ;
    parm[2] = parm[3] = parm[0];
    parm[5] = cfg->lrate & 7;
    parm[4] = parm[7] = 1;
    parm[6] = 0;
    if( shdsl_issue_cmd( nl, _DSL_MULTI_RATE_CONFIG, parm, 8 ) )
    	return  -EIO;    

//----_DSL_PREACT_RATE_LIST----//

    // if DSL configured with automatic rate select
    if( cfg->autob )	
    {
        // Set List of aviable Rates	
	memset( parm, 0, 36 );
	parm[0]=0;
	parm[1]=( u8 )( ((cfg->lrate - ( (MIN_AUTO_RATE >> 3) & 0x7f) ) >> 3 ) & 0x7f )+ 1;
    	for(iter=0; iter<parm[1] ; iter++)
	    parm[iter+2]=3+iter;	    
	if( shdsl_issue_cmd( nl, _DSL_PREACT_RATE_LIST, parm, iter+2 ) )
    	    return  -EIO;
    }

//----_DSL_TRAINING_MODE----//

    if( cfg->autob )
        parm[0] = 0x02 | 0x01<<4;	// In auto rate mode using only TCPAM16
    else if( cfg->remcfg ){
	if( ( cfg->mod==0x00 || cfg->mod==0x01 ) &&
		cfg->master && cfg->annex==ANNEX_F )
	    parm[0] = 0x02 |(cfg->mod << 4);
	else
	    parm[0] = 0x02 | 0x01 << 4;
	
    }
    else
        parm[0] = 0x02 | (cfg->mod << 4);
    parm[1] = 0;

    if( shdsl_issue_cmd( nl, _DSL_TRAINING_MODE, parm, 2 ) )
    	return  -EIO;

//----_DSL_PREACTIVATION_CFG----//

    memset( parm, 0, 12 );
    parm[0] = 0x04;	// pre-activation: G.hs
    if( cfg->autob )
        parm[1] = 0x01;	// Line probe Enabled
    else	
        parm[1] = 0x00;	// Line probe Disabled	
	
    if( cfg->autob || cfg->remcfg )
        parm[4] = 0x00;	// HTU-C send Mode Select message
    else
        parm[4] = 0x04;	// No remote configuration

    parm[5] = 0x01;	// TPS-TC Config= Clear Channel
    parm[6] = 0x00;
    
    parm[7]=cfg->annex; // annex A,B,F
    parm[8] = 0xff;	// i-bit mask (all bits)
    if( shdsl_issue_cmd( nl, _DSL_PREACTIVATION_CFG, parm, 12 ) )
    	return  -EIO;

//----_DSL_THRESHOLDS----//

    parm[0] = 0x03;	// dying gasp time - 3 frames
    parm[1] = thresh[ cfg->mod ];	
    parm[2] = 0xff;	// attenuation
    parm[3] = 0x04;	// line probe NMR (+2 dB)
    parm[4] = 0x00;	// reserved 
    parm[5] = 0x00;
    if( shdsl_issue_cmd( nl, _DSL_THRESHOLDS, parm, 6 ) )
    	return  -EIO;

//----_DSL_FR_PCM_CONFIG----//

    t = cfg->master ? 0x23 : 0x21;
    if( shdsl_issue_cmd( nl, _DSL_FR_PCM_CONFIG, &t, 1 ) )
    	return  -EIO;

//----_DSL_INTR_HOST_MASK----//

    t = 0x02;
    if( shdsl_issue_cmd( nl, _DSL_INTR_HOST_MASK, &t, 1 ) )
    	return  -EIO;

    return 0;
}

static int
shdsl_get_stat(struct net_local *nl, struct dsl_stats *ds)
{
    u8 t;
    int  i;    
    volatile struct cx28975_cmdarea  *p = nl->cmdp;    

    t = 0;
    if( shdsl_issue_cmd( nl, _DSL_FAR_END_ATTEN, &t, 1 ) )
    	return -EIO;
    ds->attenuat = ioread8( (iotype)&(p->out_data) );

    if( shdsl_issue_cmd( nl, _DSL_NOISE_MARGIN, &t, 1 ) )
	return -EIO;
    ds->nmr = ioread8( (iotype)&(p->out_data) );

    if( shdsl_issue_cmd( nl, _DSL_POWER_BACK_OFF_RESULT, &t, 1 ) )
	return -EIO;
    ds->tpbo = ioread8( (iotype)&(p->out_data) );
    ds->rpbo = ioread8( (iotype)&(p->out_data) + 1 );

    if( !shdsl_issue_cmd( nl, _DSL_SYSTEM_PERF_ERR_CTRS, &t, 1 ) ) {
	for( i = 0;  i < 4;  ++i )
	    ((u8 *)&(ds->all_atmpt))[i] = ioread8( (iotype)&(p->out_data) + i );
    }
    else
	return -EIO;
    if( !shdsl_issue_cmd( nl, _DSL_HDSL_PERF_ERR_CTRS, &t, 1 ) ) {
	for( i = 0;  i < 10;  ++i )
	    ((u8 *)&(ds->losw))[i] = ioread8( (iotype)&(p->out_data) + i );
    }
    else
	return -EIO;
    return 0;
    
}

static int
shdsl_clr_stat( struct net_local  *nl )
{
    u8 t;
    memset( &nl->in_stats, 0, sizeof(struct sg16_stats) );
    t =0;
    if( shdsl_issue_cmd( nl, _DSL_CLEAR_ERROR_CTRS, &t, 1 ) )
	return -EIO;
    t =0x04;
    if( shdsl_issue_cmd( nl, _DSL_CLEAR_ERROR_CTRS, &t, 1 ) )
	return -EIO;

    return 0;
}

static int
shdsl_issue_cmd( struct net_local  *nl,  u8  cmd,  u8  *data,  u8  size )
{
    volatile struct cx28975_cmdarea  *p = nl->cmdp;
    int  i;
    u8  cksum = 0, tmp;
    u8 *databuf = p->in_data;

/*
if( cmd==_DSL_DOWNLOAD_DATA)
    printk("%s: start\n",__FUNCTION__);                
*/
    iowrite8( 0xf0, (iotype)&(p->in_dest) );
    iowrite8( cmd, (iotype)&(p->in_opcode ) );
    iowrite8( 0, (iotype)&(p->in_zero ) );
    iowrite8(--size, (iotype)&(p->in_length ) );
    iowrite8( ( 0xf0 ^ cmd ^ size ^ 0xaa ),(iotype)&(p->in_csum) ); 

/*
if( cmd==_DSL_DOWNLOAD_DATA)
    printk("%s: send data to chip\n",__FUNCTION__);                
*/
    for( i = 0;  i <= size;  ++i )
    {
	cksum ^= *data;
	tmp=(u8)*data++;
	iowrite8( tmp,(iotype)(databuf++));	// only 1 byte per cycle!
    }
/*
if( cmd==_DSL_DOWNLOAD_DATA)
    printk("%s: send data to chip complete\n",__FUNCTION__);                
*/
    iowrite8( cksum^0xaa, (iotype)&(p->in_datasum) );
    iowrite8( _ACK_NOT_COMPLETE, (iotype)&(p->out_ack) );
    iowrite8( 0xfe, (iotype)&(p->intr_8051) );
    if( shdsl_ready(nl,_ACK_PASS ) ){
	PDEBUG("return successfully");                
	return  0;
    }else
	return  -EIO;
}

static void
shdsl_interrupt( struct net_device  *ndev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);	
    volatile struct cx28975_cmdarea  *p = nl->cmdp;
    
    if(  ioread8( (iotype)&(p->intr_host) ) != 0xfe )
    	return;

    if( ioread8( (iotype)&(p->out_ack) ) & 0x80  ){
	schedule_delayed_work(&nl->wqueue,HZ/2);    
    }
    // inquiry answer     
    else
    {
	nl->irqret=ioread8( (iotype)&(p->out_ack) );
	wake_up( &nl->wait_for_intr );	
	PDEBUG("wake up %s",ndev->name);	
    }
    // Clear acknoledgement register
    iowrite8(0, (iotype)&(p->out_ack));
    // interrupt served
    iowrite8( 0, (u8 *)&(p->intr_host) );
    ioread8( (u8 *)&(p->intr_host) );
}


static void
shdsl_link_chk( void *data )
{
    struct net_device *ndev=(struct net_device *)data;
    struct net_local  *nl  =(struct net_local *)netdev_priv(ndev);	
    volatile struct cx28975_cmdarea  *p = nl->cmdp;
    struct timeval tv;    


    // Link state
    if( ioread8( (iotype)((u8*)p + 0x3c7) ) & 2 ) 
    {
        // link up
	if( ( ioread8( (iotype)( (u8*)p + 0x3c0) ) & 0xc0) == 0x40 )
	{
	    PDEBUG("Activate:");
	    PDEBUG("%02x %02x %02x %02x %02x %02x %02x %02x",
		    ioread8((iotype)&(nl->regs->CRA)),
		    ioread8((iotype)&(nl->regs->CRB)),
		    ioread8((iotype)&(nl->regs->SR)),
		    ioread8((iotype)&(nl->regs->IMR)),
		    ioread8((iotype)&(nl->regs->CTDR)),
		    ioread8((iotype)&(nl->regs->LTDR)),
		    ioread8((iotype)&(nl->regs->CRDR)),
		    ioread8((iotype)&(nl->regs->LRDR))
		);
		

	    // set hdlc registers
	    iowrite8( 0xff, (iotype)&(nl->regs->SR) );    				    
	    iowrite8( ioread8( (iotype)&(nl->regs->CRB) ) & ~RXDE,
			(iotype)&(nl->regs->CRB) );
	    iowrite8( EXT | UFL | OFL | RXS | TXS ,
			(iotype)&(nl->regs->IMR) );    			

	    do_gettimeofday( &tv );
	    nl->in_stats.last_time = tv.tv_sec;
	    //reset Rx FIFO	    
/*	    iowrite8( 0xf0, (iotype)&(p->in_dest) );
	    iowrite8( _DSL_FR_RX_RESET, (iotype)&(p->in_opcode ) );
	    iowrite8( 0, (iotype)&(p->in_zero ) );
	    iowrite8(0, (iotype)&(p->in_length ) );
	    iowrite8( ( 0xf0 ^ _DSL_FR_RX_RESET ^ 0 ^ 0xaa ),(iotype)&(p->in_csum) ); 
	    cksum ^= 0x8;
	    iowrite8( 0x8,(iotype)&(p->in_data));	// only 1 byte per cycle!
	    iowrite8( cksum^0xaa, (iotype)&(p->in_datasum) );
	    iowrite8( _ACK_NOT_COMPLETE, (iotype)&(p->out_ack) );
	    iowrite8( 0xfe, (iotype)&(p->intr_8051) );
*/	    // enable packet receiving-transmitting
	    netif_wake_queue( ndev );
	    netif_carrier_on( ndev );
	    usermode_link_event(ndev,1);
	}
	// link down
	else 
	if( ( ioread8( (iotype)((u8*)p + 0x3c0) ) & 0xc0) != 0x40 )
	{
	    PDEBUG("Deactivate");
		
	    iowrite8( ioread8( (iotype)&(nl->regs->CRB) ) | RXDE,
			(iotype)&(nl->regs->CRB) );
	    iowrite8( EXT, (iotype)&(nl->regs->IMR) );    			
	    netif_stop_queue( ndev );
	    netif_carrier_off( ndev );
	    usermode_link_event(ndev,0);
	}
    }
}

#ifdef SG16_USERMODE_EVENTS
static int
usermode_link_event(struct net_device *ndev,int link_up)
{
	char *argv[4] = { USERMODE_HELPER, NULL, NULL,NULL };
	char *envp[3] = { NULL, NULL, NULL };
	char ifname[256];
	char lstate[256];
	sprintf(ifname,"%s",ndev->name);
	sprintf(lstate,"%d",link_up);
	argv[1] = ifname;
	argv[2] = lstate;
	envp[0] = "HOME=/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
//	printk("Start usermode helper\n");		
	return call_usermodehelper(argv[0],argv,envp,1);
}

#endif


/* --------------------------------------------------------------------------
 *   Functions, serving transmit-receive process   *
 * -------------------------------------------------------------------------- */




static int
sg16_start_xmit( struct sk_buff *skb, struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);	
	unsigned long  flags;
	dma_addr_t bus_addr;
	unsigned  cur_tbd;
	unsigned pad;

	if ( !netif_carrier_ok(ndev) ){
	    dev_kfree_skb_any( skb );
	    return 0;
	}

	/* fix concurent racing in transmit */
	spin_lock_irqsave( &nl->xlock, flags );

        if( nl->tail_xq == ((nl->head_xq - 1) & (XQLEN - 1)) ) {
		netif_stop_queue( ndev );
		goto  err_exit;
        }

	/*
         * we don't have to check if the descriptor queue was overflowed,
	 * because of XQLEN < 128
         */
	if( skb->len < ETH_ZLEN ){
		less_than_ethmin++;
	        pad = ETH_ZLEN - skb->len;
        	if( !(skb = skb_pad(skb,pad)) ){
	    	        return 0;
		}
		skb->len = ETH_ZLEN;
        }

	/* Map the buffer for DMA */
        bus_addr = dma_map_single(nl->dev,skb->data, skb->len , DMA_TO_DEVICE );

	nl->xq[ nl->tail_xq++ ] = skb;
        nl->tail_xq &= (XQLEN - 1);
	cur_tbd = ioread8( (iotype)&(nl->regs->LTDR)) & 0x7f;
        iowrite32( cpu_to_le32( bus_addr ) , (iotype)&(nl->tbd[ cur_tbd ].address) ) ;
	iowrite32( cpu_to_le32( skb->len | LAST_FRAG ),(iotype)&(nl->tbd[ cur_tbd ].length ) ) ;

        cur_tbd = (cur_tbd + 1) & 0x7f;
	iowrite8(cur_tbd,(iotype)&(nl->regs->LTDR));

        /*
	 * Probably, it's the best place to increment statistic counters
         * though those frames hasn't been actually transferred yet.
	 */
        ++nl->in_stats.sent_pkts;
	++nl->stats.tx_packets;
        nl->stats.tx_bytes += skb->len;
	ndev->trans_start = jiffies;

        spin_unlock_irqrestore( &nl->xlock, flags );

	return 0;
err_exit:
        spin_unlock_irqrestore( &nl->xlock, flags );
	return  -EBUSY;
}


static void
recv_init_frames( struct net_device *dev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(dev);		
    unsigned  cur_rbd = ioread8( (iotype)&(nl->regs->CRDR) ) & 0x7f;
    dma_addr_t bus_addr;    
    unsigned  len;

    while( nl->head_rdesc != cur_rbd ) {

	PDEBUG("Get packet");
	struct sk_buff  *skb = nl->rq[ nl->head_rq++ ];
	nl->head_rq &= (RQLEN - 1);

	bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->rbd[ nl->head_rdesc ].address)) );
	dma_unmap_single(nl->dev,bus_addr, SG16_MAX_FRAME , DMA_FROM_DEVICE );

	len = nl->rbd[ nl->head_rdesc ].length & 0x7ff;
	if( len < ETH_ZLEN )
	    len = ETH_ZLEN;
	
	skb_put( skb, len );
	skb->protocol = eth_type_trans( skb, dev );
	netif_rx( skb );
	FREE_RX();
		
	++nl->in_stats.rcvd_pkts;
	++nl->stats.rx_packets;
	nl->stats.rx_bytes += len;
	nl->head_rdesc = (nl->head_rdesc + 1) & 0x7f;
    }
    return;
    
}

static void
recv_alloc_buffs( struct net_device *dev )
{

    struct net_local  *nl  = (struct net_local *)netdev_priv(dev);		
    unsigned  cur_rbd = ioread8((iotype)&(nl->regs->LRDR)) & 0x7f;
    dma_addr_t bus_addr;
    struct sk_buff  *skb;

    
    while( nl->tail_rq != ((nl->head_rq -1) & (RQLEN - 1)) )
    {
	ALLOC_RX();
	skb = dev_alloc_skb( SG16_MAX_FRAME );
	skb->dev = dev;
	skb_reserve( skb, 2 );	// align ip on longword boundaries

	nl->rq[ nl->tail_rq++ ] = skb;
	nl->tail_rq &= (RQLEN - 1);

	/* DMA memory */
	bus_addr = dma_map_single(nl->dev,skb->data, 
		    SG16_MAX_FRAME , DMA_FROM_DEVICE );
	iowrite32( cpu_to_le32( bus_addr ),
		    (u32 *)&(nl->rbd[ cur_rbd ].address) ) ;
	iowrite32( 0, (u32*)&(nl->rbd[ cur_rbd ].length ) ) ;
	cur_rbd = (cur_rbd + 1) & 0x7f;
	iowrite8(cur_rbd,(iotype)&(nl->regs->LRDR));
    }
}

static void
recv_free_buffs( struct net_device *ndev)
{
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);		
    unsigned  last_rbd = ioread8( (iotype)&(nl->regs->LRDR)) & 0x7f;
    dma_addr_t bus_addr;    
    struct sk_buff  *skb;
    
    iowrite8( ioread8( (iotype)&(nl->regs->CRDR)), (iotype)&(nl->regs->LRDR));
    while( nl->head_rdesc != last_rbd )
    {
	skb = nl->rq[ nl->head_rq++ ];
	nl->head_rq &= (RQLEN - 1);
	bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->rbd[ nl->head_rdesc ].address) ) );
	nl->head_rdesc = (nl->head_rdesc + 1) & 0x7f;
	dma_unmap_single(nl->dev,bus_addr, SG16_MAX_FRAME , DMA_FROM_DEVICE );
        dev_kfree_skb_any( skb );
	FREE_RX();	
    }
    return;
}

static void
xmit_free_buffs( struct net_device *dev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(dev);		
    unsigned  cur_tbd = ioread8((iotype)&(nl->regs->CTDR));
    dma_addr_t bus_addr;

    spin_lock( &nl->xlock );

    if( netif_queue_stopped( dev )  &&  nl->head_tdesc != cur_tbd ){
    	netif_wake_queue( dev );
    }
		    
    while( nl->head_tdesc != cur_tbd )
    {
	/* unmap DMA memory */
	bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->tbd[ nl->head_tdesc ].address)) );
	dma_unmap_single(nl->dev,bus_addr, nl->xq[ nl->head_xq]->len, DMA_TO_DEVICE );
	dev_kfree_skb_any( nl->xq[ nl->head_xq++ ] );
	FREE_TX();	
	nl->head_xq &= (XQLEN - 1);
	nl->head_tdesc = (nl->head_tdesc + 1) & 0x7f;
    }

    spin_unlock( &nl->xlock );
    return;
}

/*
 * xmit_free_buffs may also be used to drop the queue - just turn
 * the transmitter off, and set CTDR == LTDR
 */

static void
sg16_tx_timeout( struct net_device  *ndev )
{
    struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
    
    printk( KERN_ERR "%s: transmit timeout\n", ndev->name );
    if( ioread8( (iotype)&(nl->regs->SR)) & TXS )
    {
        iowrite8( TXS,(iotype)&(nl->regs->SR));
        printk( KERN_ERR "%s: interrupt posted but not delivered\n",
    		ndev->name );
    }
    xmit_free_buffs( ndev );
}


/*----------------------------------------------------------------------------
 *Sysfs specific functions
 *----------------------------------------------------------------------------*/

static int __devinit 
init_sg16_in_sysfs(struct device *dev)
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct shdsl_config *shcfg=&(nl->shdsl_cfg);
    struct hdlc_config *hdcfg=&(nl->hdlc_cfg);

    /* initialising default parameters */
    //shdsl
    strcpy(shcfg->fw_name,"sg16.bin");

// set default role
#ifdef SG16_MASTER
    shcfg->master=1;
#else
    shcfg->master=0;
#endif    

// set remote master configuration mode
#ifdef SG16_REMCFG
    shcfg->remcfg=1;
    shcfg->annex=ANNEX_F;    
#else
    shcfg->remcfg=0;
    shcfg->annex=ANNEX_A;    
#endif    

// set default rate
#ifdef SG_RATE
    shcfg->lrate=SG16_RATE>>3;
#else    
    shcfg->lrate=128>>3;
#endif

// set coding type
#ifdef SG16_MOD 
#if SG16_MOD == 1
    shcfg->mod=1;
#elif SG16_MOD == 2
    shcfg->mod=2;
#elif SG16_MOD == 3
    shcfg->mod=3;
#else
    shcfg->mod=0;
#endif
#endif

#ifdef SG16_AUTOB
    shcfg->autob=1;
#else
    shcfg->autob=0;
#endif
    
//hdlc
#ifdef SG16_NORBURST
    hdcfg->rburst=0;
#else    
    hdcfg->rburst=1;
#endif
    
#ifdef SG16_NOWBURST
    hdcfg->wburst=0;
#else
    hdcfg->wburst=1;
#endif    

#ifdef SG16_CRC16
    hdcfg->crc16=1;
#else
    hdcfg->crc16=0;
#endif
    
#ifdef SG16_FILL7E
    hdcfg->fill_7e=1;
#else    
    hdcfg->fill_7e=0;
#endif

#ifdef SG16_INV
    hdcfg->inv=1;    
#else    
    hdcfg->inv=0;
#endif    

    /* creating attributes of device in sysfs */
    //shdsl
    if( device_create_file(dev,&dev_attr_rate) )	goto err_ext;
    if( device_create_file(dev,&dev_attr_crate))	goto err_ext0;
    if( device_create_file(dev,&dev_attr_master)) 	goto err_ext1;
    if( device_create_file(dev,&dev_attr_remcfg))	goto err_ext2;    
    if( device_create_file(dev,&dev_attr_annex)) 	goto err_ext3;
    if( device_create_file(dev,&dev_attr_mod)) 		goto err_ext4;
    if( device_create_file(dev,&dev_attr_autobaud)) 	goto err_ext5;
    if( device_create_file(dev,&dev_attr_download)) 	goto err_ext6;    
    //hdlc
    if( device_create_file(dev,&dev_attr_crc16))	goto err_ext7;
    if( device_create_file(dev,&dev_attr_fill_7e))	goto err_ext8;
    if( device_create_file(dev,&dev_attr_inv))		goto err_ext9;
    if( device_create_file(dev,&dev_attr_rburst))	goto err_ext10;
    if( device_create_file(dev,&dev_attr_wburst))	goto err_ext11;
    //addr
    if( device_create_file(dev,&dev_attr_maddr))	goto err_ext12;    
    //stat
    if( device_create_file(dev,&dev_attr_state)) 	goto err_ext13;        
    if( device_create_file(dev,&dev_attr_statistic))	goto err_ext14;        
    //debug
    if( device_create_file(dev,&dev_attr_debug))	goto err_ext15;
    // SKB memory leak
    device_create_file(dev,&dev_attr_skb_stat);

    return 0;

err_ext15:
    device_remove_file(dev,&dev_attr_statistic);        
err_ext14:
    device_remove_file(dev,&dev_attr_state);        
err_ext13:
    device_remove_file(dev,&dev_attr_maddr);        
err_ext12:
    device_remove_file(dev,&dev_attr_wburst);
err_ext11:
    device_remove_file(dev,&dev_attr_rburst);
err_ext10:
    device_remove_file(dev,&dev_attr_inv);
err_ext9:
    device_remove_file(dev,&dev_attr_fill_7e);
err_ext8:
    device_remove_file(dev,&dev_attr_crc16);
err_ext7:
    device_remove_file(dev,&dev_attr_download);    
err_ext6:
    device_remove_file(dev,&dev_attr_autobaud);
err_ext5:
    device_remove_file(dev,&dev_attr_mod);    
err_ext4:
    device_remove_file(dev,&dev_attr_annex);    
err_ext3:
    device_remove_file(dev,&dev_attr_remcfg);        
err_ext2:
    device_remove_file(dev,&dev_attr_master);
err_ext1:
    device_remove_file(dev,&dev_attr_crate);	            
err_ext0:    
    device_remove_file(dev,&dev_attr_rate);	
err_ext:
    printk("%s: error\n",__FUNCTION__);
    return -1;
}

static void __devexit
del_sg16_from_sysfs(struct device *dev)
{
    /* removing attributes of device from sysfs */
    //shdsl
    device_remove_file(dev,&dev_attr_rate);	
    device_remove_file(dev,&dev_attr_crate);	    
    device_remove_file(dev,&dev_attr_master);
    device_remove_file(dev,&dev_attr_remcfg);    
    device_remove_file(dev,&dev_attr_annex);    
    device_remove_file(dev,&dev_attr_mod);
    device_remove_file(dev,&dev_attr_autobaud);
    device_remove_file(dev,&dev_attr_download);    
    //hdlc
    device_remove_file(dev,&dev_attr_crc16);
    device_remove_file(dev,&dev_attr_fill_7e);
    device_remove_file(dev,&dev_attr_inv);
    device_remove_file(dev,&dev_attr_rburst);
    device_remove_file(dev,&dev_attr_wburst);
    //addr
    device_remove_file(dev,&dev_attr_maddr);        
    //stat
    device_remove_file(dev,&dev_attr_state);        
    device_remove_file(dev,&dev_attr_statistic);        
    //debug
    device_remove_file(dev,&dev_attr_debug);
    device_remove_file(dev,&dev_attr_skb_stat);
}

/* rate attributes */
static ssize_t
show_rate( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    u8 t=0;
    u16 tmp=0;
    
    if( !(cfg->remcfg && !cfg->master) )
	return snprintf(buf,PAGE_SIZE,"%u\n",(cfg->lrate)<<3);
    else{
	if( netif_carrier_ok(ndev) )
	{
	    t=_DSL_DATA_RATE;
	    if( !shdsl_issue_cmd(nl,_DSL_READ_CONTROL,&t,0) )
	    {
		tmp=ioread8( (u8 *)(nl->cmdp->out_data+1)) & 0x3;
		tmp=(tmp<<8)+ioread8( (u8 *)(nl->cmdp->out_data) );
		tmp--;
	    } 
	}
    }
    return snprintf(buf,PAGE_SIZE,"%u\n",tmp<<3);    
}

static ssize_t
store_rate( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    char *endp;
    u16 tmp;
    
    /* if interface is up*/
    if( (ndev->flags & IFF_UP) )
	return size;
    if( !size ) return size;

    tmp=simple_strtoul( buf,&endp,0);
    if( !tmp )
	return size;
    cfg->lrate=(tmp >> 3) & 0x3ff;
    nl->shdsl_cfg.need_preact=1;

    return size;
}


static ssize_t
show_crate( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    u8 t=0;
    u32 tmp=0;
    
    if( !cfg->autob && !( cfg->remcfg && !cfg->master ) )
	tmp=cfg->lrate;    	
    else
    {
	if( cfg->autob  && netif_carrier_ok(ndev) ){
	    if( !shdsl_issue_cmd(nl,_DSL_GHS_GET_FINAL_RATE,&t,0) )    
	    {
		tmp=ioread8( (u8 *)(nl->cmdp->out_data+2));
		tmp=(tmp<<3)+ioread8( (u8 *)(nl->cmdp->out_data+3) );
	    }
	}
	else if( cfg->remcfg && netif_carrier_ok(ndev) )
	{
	    t=_DSL_DATA_RATE;
	    if( !shdsl_issue_cmd(nl,_DSL_READ_CONTROL,&t,0) )
	    {
		tmp=ioread8( (u8 *)(nl->cmdp->out_data+1)) & 0x3;
		tmp=(tmp<<8)+ioread8( (u8 *)(nl->cmdp->out_data) );
		tmp--;		
	    } 
	}
    }
    return snprintf(buf,PAGE_SIZE,"%u\n",tmp<<3);
}

/* master attribute */
static ssize_t
show_master( struct device *dev, ADDIT_ATTR char *buf ) 
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct shdsl_config *cfg=&(nl->shdsl_cfg);

    if( cfg->master )
	return snprintf(buf,PAGE_SIZE,"master");
    else
	return snprintf(buf,PAGE_SIZE,"slave"); 
}

static ssize_t
store_master( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);

    /* if interface is up */
    if( ndev->flags & IFF_UP )
	return size;

    if( size > 0 ){
	if( buf[0]=='0' )
	    cfg->master=0;
	else
	if( buf[0]=='1' )
	    cfg->master=1;
    }    
    nl->shdsl_cfg.need_preact=1;
    return strnlen(buf,PAGE_SIZE);
}


/* Remote configuration setup */
static ssize_t
show_remcfg( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct shdsl_config *cfg=&(nl->shdsl_cfg);

    if( cfg->remcfg )
	return snprintf(buf,PAGE_SIZE,"preact");
    else
	return snprintf(buf,PAGE_SIZE,"local"); 
}

static ssize_t
store_remcfg( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);

    /* if interface is up */
    if( ndev->flags & IFF_UP )
	return size;

    if( size > 0 ){
	if( buf[0]=='0' )
	    cfg->remcfg=0;
	else if( buf[0]=='1' )
	    cfg->remcfg=1;
    }    

    nl->shdsl_cfg.need_preact=1;
    return strnlen(buf,PAGE_SIZE);
}
/* Annex setup */
static ssize_t 
show_annex( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    u8 t=cfg->annex;

    switch( t ){
    case ANNEX_A:
	return snprintf(buf,PAGE_SIZE,"Annex=A");
    case ANNEX_B:
	return snprintf(buf,PAGE_SIZE,"Annex=B"); 
    case ANNEX_F:
	return snprintf(buf,PAGE_SIZE,"Annex=F"); 
    default:
	cfg->annex=ANNEX_A;
	return snprintf(buf,PAGE_SIZE,"Annex=A");    
    }
}

static ssize_t
store_annex( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    u8 tmp;

    /* if interface is up */
    if( ndev->flags & IFF_UP )
	return size;

    if( !size )	return size;

    tmp=buf[0];
    switch(tmp)
    {
    case '0':
	cfg->annex=ANNEX_A;
	break;
    case '1':
	cfg->annex=ANNEX_B;
	break;
    case '2':
	cfg->annex=ANNEX_F;
	break;
    default:
	cfg->annex=ANNEX_A;
	break;
    }

    nl->shdsl_cfg.need_preact=1;
    return strnlen(buf,PAGE_SIZE);
}

/* mod attribute */
static ssize_t
show_mod( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    char  *modstr[] = {"TCPAM32", "TCPAM16", "TCPAM8", "TCPAM4"};    
    u8 t=0;
    u16 tmp=0;

    if( cfg->remcfg ){
	if( !netif_carrier_ok(ndev) )
	    return snprintf(buf,PAGE_SIZE,"Unknown\n");	
	else{
	    t=_DSL_DATA_RATE;
	    if( !shdsl_issue_cmd(nl,_DSL_READ_CONTROL,&t,0) )
	    {
		tmp=ioread8( (u8 *)(nl->cmdp->out_data+1)) & 0x3;
		tmp=(tmp<<8)+ioread8( (u8 *)(nl->cmdp->out_data) );
		tmp--;		
	    } 
	    if( !tmp )
		return snprintf(buf,PAGE_SIZE,"Unknown\n");	
	    else if( (tmp<<3) > 2304 )
		return snprintf(buf,PAGE_SIZE,"TCPAM32\n");	
	    else
		return snprintf(buf,PAGE_SIZE,"TCPAM16\n");	
	}
    }
    return snprintf(buf,PAGE_SIZE,"%s\n",modstr[cfg->mod]);
}

static ssize_t
store_mod( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    char tmp;    

    /* if interface is up */
    if( ndev->flags & IFF_UP )
	return size;

    if( !size )	return size;
	
    tmp=buf[0];
    switch(tmp)
    {
    case '0':
	cfg->mod=0;
	break;
    case '1':
	cfg->mod=1;
	break;
    case '2':
	cfg->mod=2;
	break;
    case '3':
	cfg->mod=3;
	break;
    }
    nl->shdsl_cfg.need_preact=1;
    return size;
}

/* autobaud attribute */
static ssize_t
show_autob( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct shdsl_config *cfg=&(nl->shdsl_cfg);

    if( cfg->autob )
	return snprintf(buf,PAGE_SIZE,"auto\n");
    else if( cfg->remcfg && !cfg->master )
	return snprintf(buf,PAGE_SIZE,"remote\n");    
    else
	return snprintf(buf,PAGE_SIZE,"static\n");
}

static ssize_t
store_autob( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{

    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct shdsl_config *cfg=&(nl->shdsl_cfg);
    u8 tmp;
    
    /* if interface is up */
    if( ndev->flags & IFF_UP )
	return size;
    /* correct input data */
    if( !size )
	return 0;

    /* check that autobaud is aviable in this firmware */
    if( !cfg->autob_en )
    {
	cfg->autob=0;
	return size;
    }
	
    /* change auto rate state */
    tmp=buf[0];
    switch(tmp)
    {
    case '0':
	cfg->autob=0;
	break;
    case '1':
	cfg->autob=1;
	break;
    }
    nl->shdsl_cfg.need_preact=1;
    return size;
}


/* download attribute */
static ssize_t
show_download( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    u8 t;

    /* if UP => fw is downloaded */
    if( ndev->flags & IFF_UP )
	return snprintf(buf,PAGE_SIZE,"1");

    /* Reset of system */
    udelay(2);
    t=0;
    if( shdsl_issue_cmd( nl, _DSL_RESET_SYSTEM, &t, 1 ) )
	return snprintf(buf,PAGE_SIZE,"0");
    udelay(2);
    if( !shdsl_ready( nl, _ACK_OPER_WAKE_UP ) )
	return snprintf(buf,PAGE_SIZE,"0");

    return snprintf(buf,PAGE_SIZE,"1");
}	

static ssize_t
store_download( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    
struct firmware *fw;    
struct shdsl_config *cfg = (struct shdsl_config *)&(nl->shdsl_cfg);

    PDEBUG("start");    
    
    if( !size )	return 0;
    if( ndev->flags & IFF_UP )
	return size;

    PDEBUG("before dload");    
    if( buf[0] == '1' ){
	if( request_firmware((const struct firmware **)&fw,cfg->fw_name,dev) ){
	    printk(KERN_ERR"%s: firmware file not found\n",ndev->name); 
	    release_firmware(fw);	
	    return -ENOENT;
	}
	else
	    shdsl_dload_fw(ndev,fw);
    }

    nl->shdsl_cfg.need_preact=1;
    return size;
}

/* CRC count attribute */
static ssize_t
show_crc16( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct hdlc_config *cfg=&(nl->hdlc_cfg);

    if( cfg->crc16 )
	return snprintf(buf,PAGE_SIZE,"crc16");
    else
	return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t
store_crc16( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct hdlc_config *cfg=&(nl->hdlc_cfg);
    u8 cfg_bt;

    if( ndev->flags & IFF_UP )
	return size;

    if( !size )	return 0;
    
    switch(buf[0]){
    case '1':
	if( cfg->crc16 )
    	    break;
	cfg->crc16=1;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) | CMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    case '0':
	if( !(cfg->crc16) )
	    break;
	cfg->crc16=0;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) & ~CMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    }	

    return size;	
}

static ssize_t
show_fill_7e( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct hdlc_config *cfg=&(nl->hdlc_cfg);

    if( cfg->fill_7e )
	return snprintf(buf,PAGE_SIZE,"on");
    else
	return snprintf(buf,PAGE_SIZE,"off");    
}

static ssize_t
store_fill_7e( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct hdlc_config *cfg=&(nl->hdlc_cfg);
    u8 cfg_bt;

    if( ndev->flags & IFF_UP )
	return size;
    
    if( !size )	return 0;
    
    switch(buf[0]){
    case '1':
	if( cfg->fill_7e )
    	    break;
	cfg->fill_7e=1;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) | FMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    case '0':
	if( !(cfg->fill_7e) )
	    break;
	cfg->fill_7e=0;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) & ~FMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    }	
    return size;	
}

static ssize_t 
show_inv( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct hdlc_config *cfg=&(nl->hdlc_cfg);

    if( cfg->inv )
	return snprintf(buf,PAGE_SIZE,"on");
    else
	return snprintf(buf,PAGE_SIZE,"off");    
}

static ssize_t
store_inv( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct hdlc_config *cfg=&(nl->hdlc_cfg);
    u8 cfg_bt;

    if( ndev->flags & IFF_UP )
	return size;

    if( !size )
	return 0;
    
    switch(buf[0]){
    case '1':
	if( cfg->inv )
    	    break;
	cfg->inv=1;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) | PMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    case '0':
	if( !(cfg->inv) )
	    break;
	cfg->inv=0;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRA)) & ~PMOD;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRA));
	break;
    }	
    return size;	
}

static ssize_t
show_rburst( struct device *dev, ADDIT_ATTR char *buf )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct hdlc_config *cfg=&(nl->hdlc_cfg);

    if( cfg->rburst )
	return snprintf(buf,PAGE_SIZE,"on");
    else
	return snprintf(buf,PAGE_SIZE,"off");    

}

static ssize_t
store_rburst( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct hdlc_config *cfg=&(nl->hdlc_cfg);
    u8 cfg_bt;

    if( ndev->flags & IFF_UP )
	return size;
    
    if( !size )	return 0;
    
    switch(buf[0]){
    case '1':
	if( cfg->rburst )
    	    break;
	cfg->rburst=1;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRB)) | RDBE;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRB));
	break;
    case '0':
	if( !(cfg->rburst) )
	    break;
	cfg->rburst=0;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRB)) & ~RDBE;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRB));
	break;
    }	

    return size;	
}

static ssize_t
show_wburst( struct device *dev, ADDIT_ATTR char *buf ) 
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct hdlc_config *cfg=&(nl->hdlc_cfg);

    if( cfg->wburst )
	return snprintf(buf,PAGE_SIZE,"on");
    else
	return snprintf(buf,PAGE_SIZE,"off");    
}

static ssize_t
store_wburst( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
    struct net_device *ndev=(struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);
    struct hdlc_config *cfg=&(nl->hdlc_cfg);
    u8 cfg_bt;

    if( ndev->flags & IFF_UP )
	return size;
    
    if( !size )	return 0;

    switch(buf[0]){
    case '1':
	if( cfg->wburst )
    	    break;
	cfg->wburst=1;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRB)) | WTBE;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRB));
	break;
    case '0':
	if( !(cfg->wburst) )
	    break;
	cfg->wburst=0;
	cfg_bt=ioread8( (iotype)&(nl->regs->CRB)) & ~WTBE;
	iowrite8( cfg_bt,(iotype)&(nl->regs->CRB));
	break;
    }	

    return size;	
}

/* MAC address less significant value */
static ssize_t
store_maddr( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_device *ndev= (struct net_device *)dev_get_drvdata(dev);
    u16 tmp;
    char *endp;

    if( ndev->flags & IFF_UP )
	return size;

    if( !size ) return 0;

    tmp=simple_strtoul( buf,&endp,16) & 0xfff;
    *(u16 *)ndev->dev_addr = htons( 0x00ff ),
    *(u32 *)(ndev->dev_addr + 2) = htonl( 0x014aa000 | tmp );     

    return size;
}

/* Statistic */

static ssize_t
show_dev_state( struct device *dev, ADDIT_ATTR char *buf ) 
{
    struct net_device *ndev= (struct net_device *)dev_get_drvdata(dev);
    struct net_local *nl=(struct net_local *)netdev_priv(ndev);    
    volatile struct cx28975_cmdarea  *p = nl->cmdp;    
    u8 stat_1,stat_3;
    char ret_ad[30];
    
    stat_1=ioread8((iotype)( (u8*)p+0x3c0) );
    stat_3=ioread8((iotype)( (u8*)p+0x3c2) );
    switch( (stat_1 >> 6) ) {
    case  0 :
	*ret_ad=0;
	switch( stat_3 & 0x0f ) {
	case  1 :	
	    snprintf(ret_ad,30,"(Bad NMR)" );
	    break;
	case  2 :	
	    snprintf(ret_ad,30, "(Frequency lock failed)" );	
	    break;
	case  3 :
	    snprintf(ret_ad,30, "(Pre-activation failed)" );
	    break;
	case  4 :
	    snprintf(ret_ad,30, "(Sync word detect failed)" );
	    break;
	}
	return snprintf(buf,42,"not ready %s",ret_ad);
    case  1 :
	return snprintf(buf,PAGE_SIZE,"online");    
    default:
	return snprintf(buf,PAGE_SIZE,"offline");    
    } 
    return 0;
}

static ssize_t
show_statistic( struct device *dev, ADDIT_ATTR char *buf ) 
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    struct sg16_stats *sb_stat=&(nl->in_stats);
    struct dsl_stats ds;

    if( shdsl_get_stat(nl,&ds) )
	return snprintf(buf,PAGE_SIZE,"err");
    
    return snprintf(buf,PAGE_SIZE,
			"%u %u %u %u  %u %u %u %u  %u %u",
			sb_stat->sent_pkts,sb_stat->rcvd_pkts,
			sb_stat->crc_errs,
			ds.atmpt,ds.all_atmpt,
			ds.losw,
			ds.crc,	ds.attenuat,
			ds.nmr,	sb_stat->last_time );
    
}

static ssize_t
store_statistic( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    
    if( !size )	return size;

    if( buf[0]=='1')
	shdsl_clr_stat(nl);
    return size;
}

/* debug */
static u8 mem_ret[PAGE_SIZE]="";

static ssize_t
show_debug( struct device *dev, ADDIT_ATTR char *buf ) 
{
    strncat(buf,mem_ret,PAGE_SIZE);
    return strlen(buf);
}

static ssize_t
store_debug( struct device *dev, ADDIT_ATTR const char *buff, size_t size )
{
    struct net_local *nl=(struct net_local *)netdev_priv(dev_get_drvdata(dev));
    volatile struct cx28975_cmdarea  *p = nl->cmdp;    
    u8 cmd,args[200],tmp1;
    u16 len;
    int i=0;
    char *endp,*ptr,bf[PAGE_SIZE];
    

    if( !size )	return size;
    
    len= (PAGE_SIZE-1 > size) ? size : PAGE_SIZE-1;
    strncpy(bf,buff,len);
    bf[len]=0;

    ptr=bf;
    cmd=simple_strtoul( ptr,&endp,16);
    PDEBUG("DEBUG: cmd=%x",cmd);    
    PDEBUG("DEBUG: endp=%x",endp);    

    while( ( ptr-bf < len) && *endp ){
	ptr=endp;
	while( (ptr-bf <len) && ( *ptr<'0' || *ptr>'9')  )
	    ptr++;
	if( (ptr-bf)>=len )
	    break;
	args[i]=(u8)simple_strtoul( ptr,&endp,16);
	PDEBUG("DEBUG: args[i]=%x",args[i]);    	
	i++;
    }
    PDEBUG("DEBUG: i=%d",i);    		
    if( shdsl_issue_cmd(nl,cmd,args,i) ){
	sprintf(mem_ret,"cmd: error");
	return size;
    }
    
    sprintf(mem_ret,"cmd: %x",cmd);
    for( i=0;i<(p->out_length)+1;i++){
	tmp1 = ioread8( (iotype)&(p->out_data) +i );
	sprintf(bf," %x",tmp1);
	strcat(mem_ret,bf);
    }
    return size;
}

static ssize_t
show_skb_stat(struct device *dev, ADDIT_ATTR char *buf )
{
    return sprintf(buf,"rx_alloc = %d\nrx_free = %d\ntx_alloc = %d\ntx_free = %d\n"
			"less_than_ethmin = %d\n",
		    rx_buffs_alloc,rx_buffs_free, tx_buffs_alloc,tx_buffs_free,
		    less_than_ethmin);
}

