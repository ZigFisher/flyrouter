/* mr16g_hdlc.c,v 1.00 04.09.2006
 *  	Sigrand MR16G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2006 by Artem U. Polyakov (art@sigrand.ru)
 *
 *	This driver presents MR16G modem 
 *	to system as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 *	04.09.2006	version 1.0
 */

/* TODO
(+)	1. Handle insmod when driver is compiled into the kernel
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/random.h>
#include<linux/firmware.h>
#include <linux/vermagic.h>
#include <linux/config.h>

#include <asm/io.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include<linux/dma-mapping.h>

#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/hdlc.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/arp.h>

#include "ds2155_regs.h"
#include "sg_hdlc_ctrl.h"
#define DEBUG_ON
#define DEFAULT_LEV 1
#include "sg_debug.h"

MODULE_DESCRIPTION( "Sigrand E1 PCI adapter driver Version 1.0\n" );
MODULE_AUTHOR( "Maintainer: Polyakov Artem art@sigrand.ru\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION("1.0");

struct net_device *bkp_dev=NULL;

// Driver initialisation
static int  mr16g_init( void );
static void mr16g_exit( void );
module_init(mr16g_init);
module_exit(mr16g_exit);

// PCI related functions
static int __devinit  mr16g_init_one( struct pci_dev *,
                                const struct pci_device_id * );
static void __devexit mr16g_remove_one( struct pci_dev * );
					
// Net device specific functions
static int __init  mr16g_probe( struct net_device * );
static int  mr16g_open( struct net_device * );
static int  mr16g_close( struct net_device * );
static struct net_device_stats  *mr16g_get_stats( struct net_device * );
static irqreturn_t  mr16g_int( int, void *, struct pt_regs * );
static void mr16g_setup_carrier(struct net_device *ndev,u8 *mask);
static int mr16g_ioctl(struct net_device *, struct ifreq *, int );
static int mr16g_attach(struct net_device *, unsigned short ,unsigned short );
static u32 mr16g_get_rate(struct net_device *ndev);
static u32 mr16g_get_slotmap(struct net_device *ndev);
static u32 mr16g_get_clock(struct net_device *ndev);

			

// Functions serving tx/rx
static void mr16g_txrx_up(struct net_device *);
static void mr16g_txrx_down(struct net_device *);
static int mr16g_start_xmit( struct sk_buff*, struct net_device* );
static void xmit_free_buffs( struct net_device * );
static void recv_init_frames( struct net_device * );
static void recv_alloc_buffs( struct net_device * );
static void recv_free_buffs( struct net_device * );

// HDLC controller functions
inline void mr16g_hdlc_down(struct net_local *nl);
inline void mr16g_hdlc_up( struct net_local *nl);
inline void mr16g_hdlc_open( struct net_local *nl);
inline void mr16g_hdlc_close( struct net_local *nl);

// DS2155 control/setup 
inline void ds2155_setreg(struct net_local *nl,u8 regname,u8 regval);
inline u8 ds2155_getreg(struct net_local *nl,u8 regname);
static int mr16g_E1_int_setup(struct net_local *nl);
static int mr16g_E1_setup(struct net_local *nl);
static u8 ds2155_carrier(struct net_local *nl);
static int ds2155_interrupt( struct net_device *ndev, u8 *mask );


// Sysfs related functions
#define ADDIT_ATTR
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
#undef ADDIT_ATTR
#define ADDIT_ATTR struct device_attribute *attr,
#endif
static void mr16g_defcfg(struct net_local *nl);
static int mr16g_sysfs_init( struct device *);
static void mr16g_sysfs_del(struct device *);

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

// E1
static ssize_t show_slotmap( struct device *dev, ADDIT_ATTR char *buff ); 
static ssize_t store_slotmap( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(slotmap,0644,show_slotmap,store_slotmap);

static ssize_t show_framed( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_framed( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(framed,0644,show_framed,store_framed);

static ssize_t show_clck( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_clck( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(clck,0644,show_clck,store_clck);

static ssize_t show_hdb3( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_hdb3( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(hdb3,0644,show_hdb3,store_hdb3);

static ssize_t show_lhaul( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_lhaul( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(long_haul,0644,show_lhaul,store_lhaul);

static ssize_t show_crc4( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_crc4( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(crc4,0644,show_crc4,store_crc4);

static ssize_t show_cas( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_cas( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(cas,0644,show_cas,store_cas);

static ssize_t show_map_ts16( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_map_ts16( struct device *dev, ADDIT_ATTR const char *buf, size_t size );
static DEVICE_ATTR(map_ts16,0644,show_map_ts16,store_map_ts16);

//debug
static ssize_t show_getreg( struct device *dev, ADDIT_ATTR char *buf );
static ssize_t store_getreg( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(getreg,0644,show_getreg,store_getreg);
static ssize_t store_setreg( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(setreg,0644,NULL,store_setreg);
static ssize_t store_chk_carrier( struct device *dev, ADDIT_ATTR const char *buff, size_t size );
static DEVICE_ATTR(chk_carrier,0644,NULL,store_chk_carrier);

static ssize_t show_hdlcregs( struct device *dev, ADDIT_ATTR char *buf );
static DEVICE_ATTR(hdlc_regs,0644,show_hdlcregs,NULL);



/*----------------------------------------------------------
 * Driver initialisation 
 *----------------------------------------------------------*/
static struct pci_device_id  mr16g_pci_tbl[] __devinitdata = {
	{ PCI_DEVICE(MR16G_PCI_VENDOR,MR16G_PCI_DEVICE) },
	{ 0 }
};

MODULE_DEVICE_TABLE( pci, mr16g_pci_tbl );
	
static struct pci_driver  mr16g_driver = {
        name:           "mr16g",
        probe:          mr16g_init_one,
        remove:         mr16g_remove_one,
        id_table:       mr16g_pci_tbl
};


static int
mr16g_init( void )
{
	printk(KERN_NOTICE"Sigrand MR-16G E1 driver\n");
	return pci_module_init( &mr16g_driver );
}

static void
mr16g_exit( void ){
	printk(KERN_NOTICE"Sigrand MR-16G E1 driver Unload\n");
	pci_unregister_driver( &mr16g_driver );
}

/*----------------------------------------------------------
 * PCI related functions 
 *----------------------------------------------------------*/
static int __devinit
mr16g_init_one( struct pci_dev *pdev,const struct pci_device_id *ent )
{

	struct device *dev_dev = (struct device*)&(pdev->dev);
        struct device_driver *dev_drv = (struct device_driver*)(dev_dev->driver);
        int err = 0;
	struct net_local *nl = NULL;
        struct net_device *ndev = NULL;
	
	PDEBUG(2,"start");
        if( pci_enable_device( pdev ) )
	        return -EIO;
	pci_set_master(pdev);
	
	// init device private data
	nl = (struct net_local *)kmalloc(sizeof(struct net_local),GFP_KERNEL);
	memset(nl,0,sizeof(struct net_local));

	// create network device structure
	if( !(ndev=alloc_hdlcdev(nl)) ){
		err = -ENOMEM;
		goto err1;
	}
    	
	// set some net device fields 
	pci_set_drvdata(pdev,ndev);
	ndev->init = mr16g_probe;
	ndev->mem_start = pci_resource_start( pdev, 1 );
    	ndev->mem_end = pci_resource_end( pdev,1 );
	ndev->irq = pdev->irq;
		
	// register net device
	err = register_hdlc_device(ndev);
	if( err < 0 ){
		printk("%s: cannot register net device, err=%d",__FILE__,err);
		goto err2;
	}
		
	// Init control through sysfs
	nl->dev = dev_dev;
	if( (err = sysfs_create_link( &(dev_drv->kobj),&(dev_dev->kobj),ndev->name )) ){
		printk(KERN_NOTICE"%s: error in sysfs_create_link\n",__FUNCTION__);	
		goto err3;
	}
	if( (err = mr16g_sysfs_init( nl->dev )) ){
		printk(KERN_NOTICE"%s: error in mr16g_sysfs_init\n",__FUNCTION__);	
	        goto err4;
	}

	PDEBUG(2,"end");
	return 0;
		
err4:			     
        sysfs_remove_link(&(dev_drv->kobj),ndev->name);
err3:
	iounmap( nl->mem_base );
	release_mem_region( ndev->mem_start, 0x1000 );
	free_irq( ndev->irq, ndev );
	unregister_netdev( ndev );
err2:
	free_netdev(ndev);	
err1:	
	kfree(nl);
	pci_disable_device(pdev);
	PDEBUG(2,"fail");
	return err;
}
				
				
static void __devexit mr16g_remove_one( struct pci_dev *pdev )
{
        struct net_device  *ndev = pci_get_drvdata( pdev );
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	struct device_driver *dev_drv=nl->dev->driver;
	
	PDEBUG(2,"start");
	// shutdown device
        mr16g_hdlc_down(nl);
	
	// Remove sysfs entires
	mr16g_sysfs_del(nl->dev);
	
	// Remove network device from OS
        sysfs_remove_link(&(dev_drv->kobj),ndev->name);
	iounmap( nl->mem_base );
	release_mem_region( ndev->mem_start, 0x1000 );
	free_irq( ndev->irq, ndev );
	unregister_netdev( ndev );
	free_netdev(ndev);	
	kfree(nl);
	pci_disable_device(pdev);
	PDEBUG(2,"end");
}


/*----------------------------------------------------------
 * Network device specific functions
 *----------------------------------------------------------*/

static int __init
mr16g_probe( struct net_device  *ndev )
{
	int err=0;
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	u8 mask = EXT;
	
	PDEBUG(2,"start");
	// Set net device fields
	netif_carrier_off( ndev );
	netif_stop_queue(ndev);
	ndev->open	= &mr16g_open;
        ndev->stop	= &mr16g_close;
        ndev->get_stats	= &mr16g_get_stats;
	ndev->do_ioctl	= &mr16g_ioctl;
        ndev->tx_queue_len  = 50;   
	// Set hdlc device fields	
        hdlc->attach = &mr16g_attach;
        hdlc->xmit   = &mr16g_start_xmit;

	// setup IO mem
	if( !(request_mem_region(ndev->mem_start,0x1000,ndev->name)) ){
	    	printk("%s:error requesting mem region",__FILE__);
	        return err;
	}
	nl->mem_base = (void *) ioremap(ndev->mem_start, 0x1000);
	nl->tbd  = (struct mr16g_hw_descr *) nl->mem_base;
	nl->rbd  = (struct mr16g_hw_descr *) ((u8 *)nl->mem_base + 0x400);
	nl->hdlc_regs = (struct mr16g_hw_regs *) ((u8 *)nl->mem_base + 0x800);
	nl->ds2155_regs = (u8 *) ((u8 *)nl->mem_base + 0xc00);		
	
	//setup tx/rx related net local fields
	spin_lock_init( &nl->xlock );

	// shutdown device before startup
	mr16g_hdlc_down(nl);
		
	// register interrupt
        if( request_irq(ndev->irq, mr16g_int, SA_SHIRQ, ndev->name, ndev) )
        {
	        printk( KERN_ERR "%s: unable to get IRQ %d.\n",ndev->name, ndev->irq );
		goto err_exit;
        }
	SET_MODULE_OWNER( ndev );
	
	// success
	printk( KERN_NOTICE "%s: Sigrand MR16G E1 PCI adapter (irq %d, mem %#lx)\n",
    				ndev->name, ndev->irq, ndev->mem_start );
	
	//initial setup	
	mr16g_defcfg(nl);
	mr16g_hdlc_up(nl);
	mdelay(1);
	mr16g_setup_carrier(ndev,&mask);
	iowrite8(mask,(iotype)&(nl->hdlc_regs->IMR));
	
        return  0;
err_exit:
        iounmap( nl->mem_base );
        release_mem_region( ndev->mem_start, 0x1000 );
	PDEBUG(2,"fail");
	return  -ENODEV;
}

// Open/initialize the board 
static int
mr16g_open( struct net_device  *ndev )
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
        int err;
	u8 *mask = EXT;

        if( (err=hdlc_open(ndev)) )
		return err;
	PDEBUG(5,"netif_carrier_ok=%d\n\tnetif_queue_stopped=%d",(int)netif_carrier_ok(ndev),(int)netif_queue_stopped(ndev));
	mr16g_txrx_up(ndev);		
        mr16g_hdlc_open(nl);	
	mr16g_E1_setup(nl);
	// tune linkindication
	udelay(50);
	PDEBUG(0,"TEST carrier");
	mr16g_setup_carrier(ndev,&mask);
	iowrite8(mask,(iotype)&(nl->hdlc_regs->IMR));
	PDEBUG(0,"TEST carrier: msk = %02x",mask);
	// start network if queuing
	netif_start_queue(ndev);	
        return 0;
}

static int
mr16g_close( struct net_device  *ndev )
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;

        PDEBUG(2,"start");

	hdlc_close(ndev);
	netif_tx_disable(ndev);		

        mr16g_hdlc_close(nl);	
	mr16g_txrx_down(ndev);

	PDEBUG(2,"end");
        return 0;
}

static irqreturn_t
mr16g_int( int  irq,void  *dev_id, struct pt_regs  *regs)
{
        struct net_device *ndev = (struct net_device *) dev_id;
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
        u8  mask = ioread8((iotype)&(nl->hdlc_regs->IMR));	
        u8  status = ioread8((iotype)&(nl->hdlc_regs->SR)) & mask;
	PDEBUG(0,"(%s) start SR(%02x) IMR(%02x) mask(%02x)",ndev->name,(nl->hdlc_regs->SR),(nl->hdlc_regs->IMR),mask);
	iowrite8(0,(iotype)&(nl->hdlc_regs->IMR));
	iowrite8(0xff,(iotype)&(nl->hdlc_regs->SR));	
		
        if( status == 0 ){
		PDEBUG(0,"status = 0");
		iowrite8(mask,(iotype)&(nl->hdlc_regs->IMR));
                return IRQ_NONE;
	}

	PDEBUG(0,"start,status=%08x",status);				

        if( status & EXT ){
		PDEBUG(8,"EXT");
		ds2155_interrupt( ndev,&mask );
        }
        /*
         * Whether transmit error is occured, we have to re-enable the
	 * transmitter. That's enough, because linux doesn't fragment
	 * packets.
	 */
																	    
        if( status & UFL )
        {
		PDEBUG(8,"UFL");
		iowrite8( ioread8((iotype)&(nl->hdlc_regs->CRA)) | TXEN,
				    (iotype)&(nl->hdlc_regs->CRA) );
		++nl->stats.tx_errors;
		++nl->stats.tx_fifo_errors;
	}
	
	if( status & RXS )
        {
		PDEBUG(8,"RXS");	
	        recv_init_frames( ndev );
	        recv_alloc_buffs( ndev );
	}
	
        if( status & TXS )
        {
		spin_lock( &nl->xlock );
		xmit_free_buffs( ndev );
		spin_unlock( &nl->xlock );
		PDEBUG(8,"TXS");
	}
	
	if( status & CRC )
        {
                ++nl->stats.rx_errors;
		++nl->stats.rx_crc_errors;
		PDEBUG(8,"CRC");		
	}

	if( status & OFL )
	{
	        ++nl->stats.rx_errors;
		++nl->stats.rx_over_errors;
		PDEBUG(8,"OFL");		
	}
	
	iowrite8(mask,(iotype)&(nl->hdlc_regs->IMR));	
	PDEBUG(0,"end IMR(%02x) mask(%02x)",(nl->hdlc_regs->IMR),mask);	
	return IRQ_HANDLED;
}	

static int
mr16g_ioctl_get_iface(struct net_device *ndev,struct ifreq *ifr)
{
	te1_settings sets;
	
	// Setup interface type. For us - only E1
	ifr->ifr_settings.type = IF_IFACE_E1;
	
        if (ifr->ifr_settings.size == 0){
		return 0;       /* only type requested */
	}
        if (ifr->ifr_settings.size < sizeof (sets)){
                return -ENOMEM;
        }
						

        sets.clock_rate = mr16g_get_rate(ndev);
	sets.clock_type = mr16g_get_clock(ndev);
        sets.loopback = 0;
	sets.slot_map = mr16g_get_slotmap(ndev);
						    
        if (copy_to_user(ifr->ifr_settings.ifs_ifsu.sync, &sets, sizeof (sets)))
                return -EFAULT;
										    
        ifr->ifr_settings.size = sizeof (sets);
        return 0;
}

/*
static int
mr16g_ioctl_set_iface(struct net_device *ndev,struct ifreq *ifr)
{
	te1_settings sets;

        if (ifr->ifr_settings.size != sizeof (sets)) {
                return -ENOMEM;
        }

        if (copy_from_user
            (&sets, ifr->ifr_settings.ifs_ifsu.sync, sizeof (sets))) {
                return -EFAULT;
        }
	return 0;
}
*/						       														
static int
mr16g_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	switch (cmd) {
	case SIOCWANDEV:
		switch (ifr->ifr_settings.type) {
		case IF_GET_IFACE:
			return mr16g_ioctl_get_iface(ndev,ifr);		

		case IF_IFACE_E1:
			return 0;
//			return mr16g_ioctl_set_iface(ndev,ifr);					

		case IF_GET_PROTO:
		default:
			return hdlc_ioctl(ndev, ifr, cmd);
		}

	default:
		/* Not one of ours. Pass through to HDLC package */
		return hdlc_ioctl(ndev, ifr, cmd);
	}
}


static int
mr16g_attach(struct net_device *dev, unsigned short encoding, unsigned short parity)
{
	PDEBUG(5,"");
	return 0;
}



static struct net_device_stats *
mr16g_get_stats( struct net_device  *dev )
{
	hdlc_device *hdlc = dev_to_hdlc(dev);
	struct net_local *nl=hdlc->priv;
	PDEBUG(5,"");	
	return  &(nl)->stats;
}

static void 
mr16g_setup_carrier(struct net_device *ndev,u8 *mask)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local *)hdlc->priv;
	u8 carrier=ds2155_carrier(nl);

	PDEBUG(0,"(%s) carrier = %d\n",ndev->name,carrier);

	// iface status control
	if( !carrier ){
		iowrite8( ioread8( (iotype)&(nl->hdlc_regs->CRB) ) | RXDE,
	        	    (iotype)&(nl->hdlc_regs->CRB) );
		*mask = EXT;
		hdlc_set_carrier(0,ndev);
		netif_carrier_off(ndev);
		ds2155_setreg(nl,CCR4,0x00);
	}else{
		iowrite8( ioread8( (iotype)&(nl->hdlc_regs->CRB) ) & ~RXDE,
	        	    (iotype)&(nl->hdlc_regs->CRB) );
		*mask = EXT | UFL | OFL | RXS | TXS | CRC;
		hdlc_set_carrier(1,ndev);
		netif_carrier_on(ndev);
		ds2155_setreg(nl,CCR4,0x01);
		
	}
}

static u32
mr16g_get_rate(struct net_device *ndev)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local *)hdlc->priv;
	struct ds2155_config *cfg= (struct ds2155_config *)&nl->e1_cfg;		
	u32 rate=0, storage=cfg->slotmap;
	
	if( !cfg->framed )
	    return 64000*32;
	    
	while(storage){
		if( storage & 0x1 )
			rate+=64000;
		storage= (storage>>1)&0x7fffffff;
	}
	return rate;
}

static u32
mr16g_get_slotmap(struct net_device *ndev)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local *)hdlc->priv;
	struct ds2155_config *cfg= (struct ds2155_config *)&nl->e1_cfg;	
	if( cfg->framed )
		return cfg->slotmap;
	return 0xffffffff;
}


static u32
mr16g_get_clock(struct net_device *ndev)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local *)hdlc->priv;
	struct ds2155_config *cfg= (struct ds2155_config *)&nl->e1_cfg;
	
	if( cfg->int_clck )
		return CLOCK_INT;
	else
		return CLOCK_EXT;
}

/* --------------------------------------------------------------------------
 *   Functions, serving transmit-receive process   *
 * -------------------------------------------------------------------------- */
 
// Enable DS2155 link status declaration
// Disable transceiver, reset all ring pointers
static void
mr16g_txrx_up(struct net_device *ndev)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local *)hdlc->priv;

	// reset rings
	nl->head_xq = nl->tail_xq = nl->head_rq = nl->tail_rq = 0;
        nl->head_tdesc = nl->head_rdesc = 0;
	iowrite8( 0, (iotype)&(nl->hdlc_regs->CTDR));
	iowrite8( 0, (iotype)&(nl->hdlc_regs->LTDR));
	iowrite8( 0, (iotype)&(nl->hdlc_regs->CRDR));
	iowrite8( 0, (iotype)&(nl->hdlc_regs->LRDR));
	recv_alloc_buffs(ndev);	
}

static void
mr16g_txrx_down(struct net_device *ndev)
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	unsigned long  flags;
	recv_free_buffs(ndev);
	spin_lock_irqsave( &nl->xlock, flags );	
	xmit_free_buffs(ndev);
	spin_unlock_irqrestore( &nl->xlock, flags );	
}

static int
mr16g_start_xmit( struct sk_buff *skb, struct net_device *ndev )
{
        hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	unsigned long  flags;
	dma_addr_t bus_addr;
	unsigned  cur_tbd;

	PDEBUG(8,"start");			  
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

	/* Map the buffer for DMA */
        bus_addr = dma_map_single(nl->dev,skb->data, skb->len , DMA_TO_DEVICE );
	
	nl->xq[ nl->tail_xq++ ] = skb;
	nl->tail_xq &= (XQLEN - 1);
	cur_tbd = ioread8( (iotype)&(nl->hdlc_regs->LTDR)) & 0x7f;
	iowrite32( cpu_to_le32( virt_to_bus(skb->data) ) , (iotype)&(nl->tbd[ cur_tbd ].address) ) ;
	iowrite32( cpu_to_le32( skb->len | LAST_FRAG ),(iotype)&(nl->tbd[ cur_tbd ].length ) ) ;
	cur_tbd = (cur_tbd + 1) & 0x7f;
	iowrite8(cur_tbd,(iotype)&(nl->hdlc_regs->LTDR));
				
	/*
	 * Probably, it's the best place to increment statistic counters
	 * though those frames hasn't been actually transferred yet.
	 */
//	++nl->in_stats.sent_pkts;
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
xmit_free_buffs( struct net_device *ndev )
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	unsigned  cur_tbd = ioread8((iotype)&(nl->hdlc_regs->CTDR));
        dma_addr_t bus_addr;

        if( netif_queue_stopped( ndev )  &&  nl->head_tdesc != cur_tbd ){
	    	netif_wake_queue( ndev );
	}
		    
        while( nl->head_tdesc != cur_tbd ){
		// unmap DMA memory 
		bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->tbd[ nl->head_tdesc ].address)) );
		dma_unmap_single(nl->dev,bus_addr, nl->xq[ nl->head_xq]->len, DMA_TO_DEVICE );
		dev_kfree_skb_any( nl->xq[ nl->head_xq++ ] );
		nl->head_xq &= (XQLEN - 1);
		nl->head_tdesc = (nl->head_tdesc + 1) & 0x7f;
	}
}

static void
recv_init_frames( struct net_device *ndev )
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
	unsigned  cur_rbd = ioread8( (iotype)&(nl->hdlc_regs->CRDR) ) & 0x7f;
        dma_addr_t bus_addr;    
	unsigned  len;

        PDEBUG(8,"start");			  
	while( nl->head_rdesc != cur_rbd ) {
		struct sk_buff  *skb = nl->rq[ nl->head_rq++ ];
		nl->head_rq &= (RQLEN - 1);

		bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->rbd[ nl->head_rdesc ].address)) );
		dma_unmap_single(nl->dev,bus_addr, SG16_MAX_FRAME , DMA_FROM_DEVICE );

		len = nl->rbd[ nl->head_rdesc ].length & 0x7ff;
		skb_put( skb, len );
		skb->protocol = hdlc_type_trans(skb, ndev);
		
		netif_rx( skb );
		
		++nl->stats.rx_packets;
		nl->stats.rx_bytes += len;
		nl->head_rdesc = (nl->head_rdesc + 1) & 0x7f;
	}
}

static void
recv_alloc_buffs( struct net_device *ndev )
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
        unsigned  cur_rbd = ioread8((iotype)&(nl->hdlc_regs->LRDR)) & 0x7f;
	dma_addr_t bus_addr;
        struct sk_buff  *skb;

	PDEBUG(8,"start");			  
        while( nl->tail_rq != ((nl->head_rq -1) & (RQLEN - 1)) ){
		skb = dev_alloc_skb( SG16_MAX_FRAME );
		skb->dev = ndev;
		skb_reserve( skb, 2 );	// align ip on longword boundaries

		nl->rq[ nl->tail_rq++ ] = skb;
		nl->tail_rq &= (RQLEN - 1);

		// DMA memory 
		bus_addr = dma_map_single(nl->dev,skb->data, 
			    SG16_MAX_FRAME , DMA_FROM_DEVICE );
		iowrite32( cpu_to_le32( bus_addr ),
			    (u32 *)&(nl->rbd[ cur_rbd ].address) ) ;
		iowrite32( 0, (u32*)&(nl->rbd[ cur_rbd ].length ) ) ;
		cur_rbd = (cur_rbd + 1) & 0x7f;
		iowrite8(cur_rbd,(iotype)&(nl->hdlc_regs->LRDR));
	}
}

static void
recv_free_buffs( struct net_device *ndev)
{
	hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct net_local *nl=(struct net_local*)hdlc->priv;
        unsigned  last_rbd = ioread8( (iotype)&(nl->hdlc_regs->LRDR)) & 0x7f;
	dma_addr_t bus_addr;    
        struct sk_buff  *skb;
    
        PDEBUG(8,"start");			  
	iowrite8( ioread8( (iotype)&(nl->hdlc_regs->CRDR)), (iotype)&(nl->hdlc_regs->LRDR));
        while( nl->head_rdesc != last_rbd ){
		skb = nl->rq[ nl->head_rq++ ];
		nl->head_rq &= (RQLEN - 1);
		bus_addr=le32_to_cpu( ioread32( (u32*)&(nl->rbd[ nl->head_rdesc ].address) ) );
		nl->head_rdesc = (nl->head_rdesc + 1) & 0x7f;
		dma_unmap_single(nl->dev,bus_addr, SG16_MAX_FRAME , DMA_FROM_DEVICE );
    		dev_kfree_skb_any( skb );
        }
}

/*----------------------------------------------------------
 *  HDLC controller control/setup 
 *----------------------------------------------------------*/

// Disable device
inline void
mr16g_hdlc_down(struct net_local *nl)
{
        iowrite8( XRST , (iotype)&(nl->hdlc_regs->CRA));
	iowrite8( RXDE , (iotype)&(nl->hdlc_regs->CRB));
	iowrite8( 0, (iotype)&( nl->hdlc_regs->IMR));
	iowrite8( 0xff, (iotype)&(nl->hdlc_regs->SR));
	ds2155_setreg(nl,CCR4,0x00);
}

inline void	
mr16g_hdlc_up( struct net_local *nl)
{
        iowrite8( EXT, (iotype)&(nl->hdlc_regs->IMR) );                              
        iowrite8( 0xff, (iotype)&(nl->hdlc_regs->SR) );                              
        iowrite8( XRST , (iotype)&(nl->hdlc_regs->CRA));                             
        iowrite8( 0 , (iotype)&(nl->hdlc_regs->CRB));
	PDEBUG(0,"SR(%02x) IMR(%02x)",nl->hdlc_regs->SR ,nl->hdlc_regs->IMR);
}	

inline void	
mr16g_hdlc_open( struct net_local *nl)
{
        u8 cfg_byte;

	// Control register A
        cfg_byte= XRST | RXEN | TXEN;
	if( nl->hdlc_cfg.crc16 )
	        cfg_byte|=CMOD;
	if( nl->hdlc_cfg.fill_7e )
	        cfg_byte|=FMOD;
	if( nl->hdlc_cfg.inv )
	        cfg_byte|=PMOD;
	iowrite8(cfg_byte,(iotype)&(nl->hdlc_regs->CRA));

	// Control register B
	cfg_byte=ioread8( (iotype)&(nl->hdlc_regs->CRB)) | RODD;
	if( nl->hdlc_cfg.rburst )
	        cfg_byte|=RDBE;
	if( nl->hdlc_cfg.wburst )
	        cfg_byte|=WTBE;
	iowrite8(cfg_byte,(iotype)&(nl->hdlc_regs->CRB));
}

inline void	
mr16g_hdlc_close( struct net_local *nl)
{
        u8 cfg_byte;

	// Control register A
	iowrite8(XRST,(iotype)&(nl->hdlc_regs->CRA));
	// Control register B
	iowrite8(0,(iotype)&(nl->hdlc_regs->CRB));
}

/*----------------------------------------------------------
 *  DS2155 control/setup 
 *----------------------------------------------------------*/
inline void ds2155_setreg(struct net_local *nl,u8 regoffs,u8 regval)
{
	iowrite8(regval,nl->ds2155_regs+regoffs);
}
inline u8 ds2155_getreg(struct net_local *nl,u8 regoffs)
{
	return ioread8(nl->ds2155_regs+regoffs);
}

static u8
ds2155_carrier(struct net_local *nl)
{
	u8 ret=0;
	u8 tmp;
	tmp = ds2155_getreg(nl,SR1);
	ds2155_setreg(nl,SR1,LRCL);
	udelay(5);
	tmp = ds2155_getreg(nl,SR1);
	ds2155_setreg(nl,SR1,LRCL);
	udelay(5);	
	tmp = ds2155_getreg(nl,SR1);
	if( !( tmp & LRCL) ){
		ret=1;
	}
	return ret;
}


// Configuration of E1 channel
static int
mr16g_E1_int_setup(struct net_local *nl)
{
	// enable link interrupts
	ds2155_setreg(nl,SR1,LRCL);
	ds2155_getreg(nl,SR1);
	ds2155_setreg(nl,SR1,LRCL);
	ds2155_setreg(nl,IMR1,LRCL);
	return 0;
}


// Configuration of E1 channel
static int
mr16g_E1_setup(struct net_local *nl)
{
	struct ds2155_config *cfg= (struct ds2155_config *)&(nl->e1_cfg);
	u8 tmp,*smap;
	int i;


	// Check inter-option depends
	if( !cfg->framed ){
		cfg->cas=0;
		cfg->crc4=0;
		cfg->slotmap=0;		
	}else{
		if( cfg->cas )
			cfg->ts16=0;
	}
	
	// software reset. Note: DS2155 has no hardware reset.
	ds2155_setreg(nl,MSTRREG,0x01);
	i=0;
	while( ds2155_getreg(nl,MSTRREG) && i< 10000)
		i++;
	
	// general configuration
	ds2155_setreg(nl,MSTRREG,E1T1); 
	ds2155_setreg(nl,IOCR1,0x00);	
	ds2155_setreg(nl,IOCR2,(TSCLKM|RSCLKM));
	
	// E1 Registers
	// E1 Receive Control Register 1
	tmp=0;
	if( !cfg->framed )	// SYNCE=1 (OFF) for unframed mode	
		tmp |=SYNCE;
	if( cfg->hdb3 )		// RHDB3=1 for HDB3 mode		
		tmp |=RHDB3;
	if( cfg->crc4 )		// CRC4=1 for CRC4 multiframe		
		tmp |=RCRC4;
	if( !cfg->cas )		// RSIGM=0 for CAS multiframe		
		tmp |=RSIGM;
	ds2155_setreg(nl,E1RCR1,tmp);		
	
	// E1 Receive Control Register 2
	ds2155_setreg(nl,E1RCR2,0x00);
	
	// E1 Transmit Control Register 1
	// tfpt-t16s-tua1-tsis-tsa1-THDB3-tg802-tcrc4
	tmp=0;
	if( cfg->hdb3 )		// THDB3=1 for HDB3 mode
		tmp |=THDB3;
	if( cfg->crc4 )		// TCRC4=1 for CRC4 multiframe
		tmp |=TCRC4;
	if( !cfg->framed )	// TFPT=1 if timeslot 0 is mapped to HDLC (should never be mapped in framed mode!)
		tmp |=TFPT;
	if( cfg->framed && !cfg->ts16 )	// T16S=0 if timeslot 16 is mapped to HDLC or for unframed mode
		tmp |= T16S;
	ds2155_setreg(nl,E1TCR1,tmp);		

	// E1 Transmit Control Register 2
	ds2155_setreg(nl,E1TCR2,0x00);	// sa8s-sa7s-sa6s-sa5s-sa4s-aebe-aais-ara
	
	// Common Control Registers
	// Common Control Register 1
	ds2155_setreg(nl,CCR1,0x80);	// MCLKS-crc4r-sie-odm-dicai-tcss1-tcss0-rlosf
	
	// Common Control Register 3
	ds2155_setreg(nl,CCR3,0x00);	// tmss-intdis-cttui-tdatfmt-tgpcken-rdatfmt-rgpcken
	// INTDIS=1 to disable interrupts
	
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         	
	// Line Interface Unit Registers
	// Line Interface Control 1
	tmp=TPD|L0;
	if( cfg->long_haul )	// EGL=1 for long haul
		tmp |=EGL;
	ds2155_setreg(nl,LIC1,tmp);

	// Line Interface Control 2
	// ETS-lirst-ibpv-TUA1-jamux-0-scld-clds
	ds2155_setreg(nl,LIC2,0x90);
	
	// Line Interface Control 3
	// 0-tces-rces-mm1-mm0-rsclke-tsclke-taoz
	ds2155_setreg(nl,LIC3,0x00); 
	
	// Line Interface Control 4
	// cmie-cmii-mps1-mps0-TT1-TT0-RT1-RT0
	ds2155_setreg(nl,LIC4,0x0F);
	
	// Error counters config
	// 0-mecu-ecus-EAMS-vcrfs-fsbe-moscrf-lvcrf
	tmp=EAMS;
	if( cfg->hdb3 )		// VCRFS=1 for HDB3 mode
		tmp |=VCRFS;
	ds2155_setreg(nl,ERCNT,tmp);
	
	// Idle code for all channels
	ds2155_setreg(nl,IAAR,0x41);	// all channels
	ds2155_setreg(nl,PCICR,0xFF);	// all 1's idle code

	// Sa, Si bits - non-CRC4 mode
	ds2155_setreg(nl,TAF,0x9B);	// Si-0-0-1-1-0-1-1
	ds2155_setreg(nl,TNAF,0xDF);	// Si-1-A-Sa4-Sa5-Sa6-Sa7-Sa8
	
	// Sa, Si bits - CRC4 mode
	ds2155_setreg(nl,TSiAF,0xFF);	// Si bits for align frames
	ds2155_setreg(nl,TSiNAF,0xFF);	// Si bits for non-align frames
	ds2155_setreg(nl,TRA,0x00);	// Remote Alarm bits
	for(i=0;i<5;i++)		//Fill TSa4 to TSa8 with all 1s
		ds2155_setreg(nl,TSa4+i,0xFF); 
		
	// TS16 signalling default values (A=1, B=1, C=0, D=1; X=1, Y=0)
	// TS16 frame 1
	ds2155_setreg(nl,TS01,0x0B);	//0-0-0-0-X-y-X-X
	for(i=0;i<15;i++)		// set TS02-TS16 to A-B-c-D-A-B-c-D
		ds2155_setreg(nl,TS02+i,0xDD);
		
	// Line Interface reset
	ds2155_setreg(nl,LIC2,0xD0);	// ETS-LIRST-ibpv-TUA1-jamux-0-scld-clds  (set LIRST)
	ds2155_setreg(nl,LIC2,0x90);	// ETS-lirst-ibpv-TUA1-jamux-0-scld-clds  (clear LIRST)
	
	//interrupt setup
	mr16g_E1_int_setup(nl);

	if( cfg->framed ){
		// enable framed mode
		tmp = FRM | ioread8((iotype)&(nl->hdlc_regs->CRB));
		iowrite8(tmp,(iotype)&(nl->hdlc_regs->CRB));
	}
	else{
		// disable framed mode
		tmp = ioread8((iotype)&(nl->hdlc_regs->CRB)) & (~FRM);
		iowrite8(tmp,(iotype)&(nl->hdlc_regs->CRB));
		
	}
	// set slotmap	
	smap=(u8*)&cfg->slotmap;						
	iowrite8(smap[0],(iotype)&(nl->hdlc_regs->MAP0));
	iowrite8(smap[1],(iotype)&(nl->hdlc_regs->MAP1));	
	iowrite8(smap[2],(iotype)&(nl->hdlc_regs->MAP2));	
	iowrite8(smap[3],(iotype)&(nl->hdlc_regs->MAP3));	


	if( !cfg->int_clck ){
		tmp = EXTC | ioread8((iotype)&(nl->hdlc_regs->CRB));
		iowrite8(tmp,(iotype)&(nl->hdlc_regs->CRB));
	}
	
	return 0;
} 

// TODO - check who is a source of Interrupt 
static int
ds2155_interrupt( struct net_device *ndev, u8 *mask )
{
	PDEBUG(5,"start");
//	printk("%s: start\n",__FUNCTION__);
	mr16g_setup_carrier( ndev,mask );
//	printk("%s: end\n",__FUNCTION__);	
	return 0;
}


/*----------------------------------------------------------
 * Sysfs related functions
 *----------------------------------------------------------*/

static void
mr16g_defcfg(struct net_local *nl)
{
	//E1 initial setup			
	nl->e1_cfg.framed=1;
	nl->e1_cfg.hdb3=1;
	nl->e1_cfg.int_clck=1;
	//HDLC initial setup				
	nl->hdlc_cfg.crc16=1;
	nl->hdlc_cfg.fill_7e=1;	
	nl->hdlc_cfg.rburst=0;
	nl->hdlc_cfg.wburst=0;	
}

static int __devinit
mr16g_sysfs_init(struct device *dev)
{

	//hdlc-controller
	device_create_file(dev,&dev_attr_crc16);
	device_create_file(dev,&dev_attr_fill_7e);
	device_create_file(dev,&dev_attr_inv);
	device_create_file(dev,&dev_attr_rburst);
	device_create_file(dev,&dev_attr_wburst);

	//E1
	device_create_file(dev,&dev_attr_slotmap);	
	device_create_file(dev,&dev_attr_framed);
	device_create_file(dev,&dev_attr_hdb3);
	device_create_file(dev,&dev_attr_long_haul);	
	device_create_file(dev,&dev_attr_crc4);
	device_create_file(dev,&dev_attr_cas);
	device_create_file(dev,&dev_attr_map_ts16);
	device_create_file(dev,&dev_attr_clck);
	// debug
	device_create_file(dev,&dev_attr_getreg);
	device_create_file(dev,&dev_attr_setreg);
	device_create_file(dev,&dev_attr_chk_carrier);	
	device_create_file(dev,&dev_attr_hdlc_regs);	
    return 0;
}	    

static void __devexit
mr16g_sysfs_del(struct device *dev)
{
	//hdlc-controller
	device_remove_file(dev,&dev_attr_crc16);
	device_remove_file(dev,&dev_attr_fill_7e);
	device_remove_file(dev,&dev_attr_inv);
	device_remove_file(dev,&dev_attr_rburst);
	device_remove_file(dev,&dev_attr_wburst);

	// E1
	device_remove_file(dev,&dev_attr_slotmap);		
	device_remove_file(dev,&dev_attr_framed);
	device_remove_file(dev,&dev_attr_hdb3);
	device_remove_file(dev,&dev_attr_long_haul);	
	device_remove_file(dev,&dev_attr_crc4);
	device_remove_file(dev,&dev_attr_cas);
	device_remove_file(dev,&dev_attr_map_ts16);
	device_remove_file(dev,&dev_attr_clck);

	// debug
	device_remove_file(dev,&dev_attr_getreg);
	device_remove_file(dev,&dev_attr_setreg);
	device_remove_file(dev,&dev_attr_chk_carrier);		
	device_remove_file(dev,&dev_attr_hdlc_regs);	
}	    

//---------- hdlc -----------------//

// CRC count attribute 
static ssize_t
show_crc16( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

        if( cfg->crc16 )
		return snprintf(buf,PAGE_SIZE,"crc16");
        else
		return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t
store_crc16( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

        if( ndev->flags & IFF_UP )
		return size;

        if( !size )	return 0;
    
	if(buf[0] == '1' )
		cfg->crc16=1;
	else if( buf[0] == '0' )
		cfg->crc16=0;

	return size;	
}

static ssize_t
show_fill_7e( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

        if( cfg->fill_7e )
		return snprintf(buf,PAGE_SIZE,"fill_7e");
        else
		return snprintf(buf,PAGE_SIZE,"fill_ff");    
}

static ssize_t
store_fill_7e( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

	if( ndev->flags & IFF_UP )
		return size;
    
        if( !size )	return 0;
	
	if(buf[0] == '1' )
		cfg->fill_7e=1;
	else if( buf[0] == '0' )
		cfg->fill_7e=0;
	
	return size;	
}

static ssize_t 
show_inv( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);

        if( cfg->inv )
		return snprintf(buf,PAGE_SIZE,"inversion");
        else
		return snprintf(buf,PAGE_SIZE,"normal");    
}

static ssize_t
store_inv( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

	if( ndev->flags & IFF_UP )
		return size;

	if( !size )
		return 0;
    
	if(buf[0] == '1' )
		cfg->inv=1;
	else if( buf[0] == '0' )
		cfg->inv=0;

	return size;	
}

static ssize_t
show_rburst( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

        if( cfg->rburst )
		return snprintf(buf,PAGE_SIZE,"rbon");
        else
		return snprintf(buf,PAGE_SIZE,"rboff");    
}

static ssize_t
store_rburst( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

	if( ndev->flags & IFF_UP )
		return size;
    
	if( !size )	return 0;

	if(buf[0] == '1' )
		cfg->rburst=1;
	else if( buf[0] == '0' )
		cfg->rburst=0;
        return size;	
}

static ssize_t
show_wburst( struct device *dev, ADDIT_ATTR char *buf ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);	

        if( cfg->wburst )
		return snprintf(buf,PAGE_SIZE,"wbon");
        else
		return snprintf(buf,PAGE_SIZE,"wboff");    
}

static ssize_t
store_wburst( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
        struct hdlc_config *cfg=mr16g_hdlcfg(ndev);

        if( ndev->flags & IFF_UP )
		return size;
    
        if( !size )	return 0;

	if(buf[0] == '1' )
		cfg->wburst=1;
	else if( buf[0] == '0' )
		cfg->wburst=0;
        return size;	
}

//---------- ds2155-----------------//

// timeslots

static u32
str2slotmap(char *str,size_t size,int ts16,int *err)
{
        char *e,*s=str;
        u32 fbit,lbit,ts=0;
        int i;

	PDEBUG(4,"start");	
        for (;;) {
                fbit=lbit=simple_strtoul(s, &e, 10);
                if (s == e)
                        break;
                if (*e == '-') {
                        s = e+1;
                        lbit = simple_strtoul(s, &e, 10);
                }

                if (*e == ','){
                        e++;
                }
                if( !(fbit < MAX_TS_BIT && lbit < MAX_TS_BIT) )
                        break;
                for (i=fbit; i<=lbit;i++){
			if( i!=0 && (i!=16 || ts16) )
	                        ts |= 1L << i;
		}
                s=e;
        }
	PDEBUG(4,"str=%08x, s=%08x,size=%d",(u32)str,(u32)s,size);
	*err=0;	
	if( s != str+(size-1) )
		*err=1;
        return ts;
}


static int
slotmap2str(u32 smap, char *buf)
{
	int start = -1,end, i;
	char *p=buf;
	
	for(i=0;i<32;i++){
		if( start<0 ){
			start = ((smap >> i) & 0x1) ? i : -1;
		}else if( !((smap >> i) & 0x1) || i == 31){
			end = ((smap >> i) & 0x1) ? i : i-1;
			if( p>buf )
				p += sprintf(p,",");
			p += sprintf(p,"%d",start);
			if( start<end )
				p += sprintf(p,"-%d",end);
			start=-1;
		}
	}
	return strlen(buf);
}


static ssize_t
show_slotmap( struct device *dev, ADDIT_ATTR char *buff )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);
	return slotmap2str(cfg->slotmap,buff);
}


static ssize_t
store_slotmap( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);
	u32 ts=0;
	int err;
	char *str;

	PDEBUG(4,"start");	
        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( !size )
		return size;

	str=(char *)(buf+(size-1));
	*str=0;
	str=(char *)buf;	
	PDEBUG(4,"call str2slotmap");	
	ts=str2slotmap(str,size,cfg->ts16,&err);
	PDEBUG(4,"str2slotmap completed");		
	if( err ){
		printk("mr16g: error in timeslot string (%s)\n",buf);
		return size;
	}
	if( ts ){
		cfg->slotmap=ts;
	}
        return size;
}

static ssize_t
show_map_ts16( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->ts16 )
		return snprintf(buf,PAGE_SIZE,"mapped");
        else
		return snprintf(buf,PAGE_SIZE,"not mapped"); 
}

static ssize_t
store_map_ts16( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);	

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' ){
			cfg->ts16=0;
			cfg->slotmap &= ~(1<<16);
		}else if( buf[0]=='1' ){
		        cfg->ts16=1;
			if( cfg->cas ){
			    cfg->cas = 0;
			}
		}
        }    
        return size;
}


// framed/unframed mode 
static ssize_t
show_framed( struct device *dev, ADDIT_ATTR char *buf ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->framed )
		return snprintf(buf,PAGE_SIZE,"framed");
        else
		return snprintf(buf,PAGE_SIZE,"unframed"); 
}

static ssize_t
store_framed( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->framed=0;
		else
		if( buf[0]=='1' )
		        cfg->framed=1;
        }    
        return size;
}

// internal/external clock 
static ssize_t
show_clck( struct device *dev, ADDIT_ATTR char *buf ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->int_clck )
		return snprintf(buf,PAGE_SIZE,"internal");
        else
		return snprintf(buf,PAGE_SIZE,"external"); 
}

static ssize_t
store_clck( struct device *dev, ADDIT_ATTR const char *buf, size_t size ) 
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->int_clck=0;
		else
		if( buf[0]=='1' )
		        cfg->int_clck=1;
        }    
        return size;
}

// Long haul mode
static ssize_t
show_lhaul( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->long_haul )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off"); 

}

static ssize_t
store_lhaul( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->long_haul=0;
		else if( buf[0]=='1' )
		        cfg->long_haul=1;
        }    
        return size;
}


// HDB3 mode
static ssize_t
show_hdb3( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->hdb3 )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off"); 

}

static ssize_t
store_hdb3( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->hdb3=0;
		else
		if( buf[0]=='1' )
		        cfg->hdb3=1;
        }    
        return size;
}

// CRC4 mode

static ssize_t
show_crc4( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->crc4 )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off"); 

}
static ssize_t
store_crc4( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->crc4=0;
		else
		if( buf[0]=='1' )
		        cfg->crc4=1;
        }    
        return size;
}

// CAS mode

static ssize_t
show_cas( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        if( cfg->cas )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off"); 
}

static ssize_t
store_cas( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct ds2155_config *cfg=mr16g_e1cfg(ndev);

        /* if interface is up */
	if( ndev->flags & IFF_UP )
		return size;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->cas=0;
		else
		if( buf[0]=='1' ){
		        cfg->cas=1;
			if( cfg->ts16 ){
			    cfg->ts16 = 0;
			    cfg->slotmap &= ~(1<<16);
			}
		}
        }    
        return size;
}


// debug
static u8 getreg_val=0,getreg_reg=0;
static ssize_t show_getreg( struct device *dev, ADDIT_ATTR char *buf )
{
	return sprintf(buf,"reg(%02x)=%02x\n",getreg_reg,getreg_val);
}

static ssize_t store_getreg( struct device *dev, ADDIT_ATTR const char *buff, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct net_local *nl=mr16g_priv(ndev);
	u8 reg;
	u16 len;
	char *endp,*ptr,bf[PAGE_SIZE];
			
	if( !size ) return size;
	
	len= (PAGE_SIZE-1 > size) ? size : PAGE_SIZE-1;
	strncpy(bf,buff,len);
	bf[len]=0;
		
	ptr=bf;
	reg=simple_strtoul( ptr,&endp,16);
	PDEBUG(9,"reg=%02x",reg);
	getreg_reg=reg;
	getreg_val=ds2155_getreg(nl,reg);
	return size;
}


static ssize_t store_setreg( struct device *dev, ADDIT_ATTR const char *buff, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct net_local *nl=mr16g_priv(ndev);	

	u8 reg,val;
	u16 len;
	char *endp,*ptr,bf[PAGE_SIZE];
			
	if( !size ) return size;
	
	len= (PAGE_SIZE-1 > size) ? size : PAGE_SIZE-1;
	strncpy(bf,buff,len);
	bf[len]=0;
		
	ptr=bf;
	reg=simple_strtoul( ptr,&endp,16);
	PDEBUG(9,"reg=%02x",reg);
	PDEBUG(9,"endp=%08x",endp);
        ptr=endp;
        while( (ptr-bf <len) && !( *ptr>='0' && *ptr<='9') && 
		!( *ptr>='A' && *ptr<='F' ) &&	!( *ptr>='a' && *ptr<='f' ) )
                ptr++;
	val=(u8)simple_strtoul(ptr,&endp,16);
	PDEBUG(9,"val=%02x",val);
	ds2155_setreg(nl,reg,val);
	return size;
}


static ssize_t store_chk_carrier( struct device *dev, ADDIT_ATTR const char *buff, size_t size )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
//	mr16g_setup_carrier( ndev,&mask );
	
	return size;
}


static ssize_t show_hdlcregs( struct device *dev, ADDIT_ATTR char *buf )
{
	struct net_device *ndev=(struct net_device*)dev_get_drvdata(dev);
	struct net_local *nl=mr16g_priv(ndev);
	int len = 0;

	len += snprintf(buf+len,PAGE_SIZE-len,"MAP0=%02x MAP1=%02x MAP2=%02x MAP3=%02x\n",ioread8((iotype)&(nl->hdlc_regs->MAP0)),
		    ioread8((iotype)&(nl->hdlc_regs->MAP1)),ioread8((iotype)&(nl->hdlc_regs->MAP2)),
		    ioread8((iotype)&(nl->hdlc_regs->MAP3)) );

	len += snprintf(buf+len,PAGE_SIZE-len,"CRA(%02x) CRB(%02x) SR(%02x) IMR(%02x)\nCRDR(%02x) LRDR(%02x) CTDR(%02x) LTDR(%02x)\n",
			nl->hdlc_regs->CRA,nl->hdlc_regs->CRB,nl->hdlc_regs->SR,nl->hdlc_regs->IMR,
			nl->hdlc_regs->CRDR,nl->hdlc_regs->LRDR,nl->hdlc_regs->CTDR,nl->hdlc_regs->LTDR);

	return len;
}

