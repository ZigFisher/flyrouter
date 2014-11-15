#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/kobject.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "include/sdfe4_lib.h"
#include "include/sg17eoc.h"
#include "sg17lan.h"

// Debug parameters
//#define DEBUG_ON
#define DEFAULT_LEV 20
#include "sg17debug.h"


/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

// Mode control (master/slave)
static ssize_t show_mode(struct class_device *cdev, char *buf) 
{                                                                       
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	if( nl->shdsl_cfg->mode == STU_C )
		return snprintf(buf,PAGE_SIZE,"master");
	else if( nl->shdsl_cfg->mode == STU_R )
		return snprintf(buf,PAGE_SIZE,"slave");
	else
		return snprintf(buf,PAGE_SIZE,"NOT defined");	
}

static ssize_t
store_mode( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

        // if interface is up 
        if( (ndev->flags & IFF_UP) )
		return size;
        if( size > 0 ){
	if( buf[0] == '0' ){
	    cfg->mode = STU_R;
	    cfg->startup_initialization = STARTUP_FAREND;
	    cfg->transaction = GHS_TRNS_00;
	}else if( buf[0] == '1' ){
	    cfg->mode = STU_C;
	    cfg->startup_initialization = STARTUP_LOCAL;
	    cfg->transaction = GHS_TRNS_10;
	}	    
    }    
    return size;
}
static CLASS_DEVICE_ATTR(mode,0644,show_mode,store_mode);

// Annex control 
static ssize_t show_annex(struct class_device *cdev, char *buf) 
{                                                                       
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;	

	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		 return 0;
	switch( cfg->annex ){
	case ANNEX_A:
		return snprintf(buf,PAGE_SIZE,"A");
	case ANNEX_B:
		return snprintf(buf,PAGE_SIZE,"B");
	case ANNEX_A_B:
		return snprintf(buf,PAGE_SIZE,"AB");
/*	case ANNEX_G:
		return snprintf(buf,PAGE_SIZE,"annex G");
	case ANNEX_F:
		return snprintf(buf,PAGE_SIZE,"annex F");
*/	default:
		return snprintf(buf,PAGE_SIZE,"NOT defined");
	}	
}
static ssize_t
store_annex( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

        // if interface is up 
	if( ndev->flags & IFF_UP )
		return size;
        if( !size )	return size;
	
	PDEBUG(0,"tmp=%c",buf[0]);
	
        switch( buf[0] ){
        case '0':
		cfg->annex=ANNEX_A;
		break;
        case '1':
		cfg->annex=ANNEX_B;
		break;
        case '2':
		cfg->annex=ANNEX_A_B;
		break;
/*        case '3':
		cfg->annex=ANNEX_G;
		break;
        case '4':
		cfg->annex=ANNEX_F;
		break;
*/        default:
		break;
        }
        return size;
}
static CLASS_DEVICE_ATTR(annex, 0644 ,show_annex,store_annex);

// Rate control
static ssize_t show_rate(struct class_device *cdev, char *buf) 
{                                                                       
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	
	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		 return 0;
		 
	return snprintf(buf,PAGE_SIZE,"%d",nl->shdsl_cfg->rate);
}

static ssize_t
store_rate( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
        char *endp;
	u16 tmp;
	
        // check parameters
	if( (ndev->flags & IFF_UP) || !size)
		return size;
        if( !size ) return size;

        tmp=simple_strtoul( buf,&endp,0);
	if( !tmp )
		return size;

        cfg->rate=tmp;
        return size;
}
static CLASS_DEVICE_ATTR(rate, 0644 ,show_rate,store_rate);

// TCPAM control
static ssize_t show_tcpam(struct class_device *cdev, char *buf) 
{                                                                       
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;	

	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		 return 0;
	
	switch( cfg->tc_pam ){
	case TCPAM16:
		return snprintf(buf,PAGE_SIZE,"TCPAM16");
	case TCPAM32:
		return snprintf(buf,PAGE_SIZE,"TCPAM32");
	default:
		return snprintf(buf,PAGE_SIZE,"NOT defined");
	}	
}
static ssize_t
store_tcpam( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
        u8 tmp;

        // if interface is up 
	if( ndev->flags & IFF_UP )
		return size;
        if( !size )	return size;
	tmp=buf[0];
	PDEBUG(0,"tmp=%c",tmp);
        switch(tmp){
        case '0':
		cfg->tc_pam=TCPAM16;
		break;
        case '1':
		cfg->tc_pam=TCPAM32;
		break;
        default:
		break;
        }
        return size;
}
static CLASS_DEVICE_ATTR(tcpam, 0644 ,show_tcpam,store_tcpam);

// Apply changes
static ssize_t
store_apply_cfg( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
        // if interface is up 
	if( ndev->flags & IFF_UP )
		return size;
        if( !size )	return size;
        if( buf[0] == '1' )
		cfg->need_reconf=1;
        return size;
}
static CLASS_DEVICE_ATTR(apply_cfg, 0200 ,NULL,store_apply_cfg);


// ---------------------------- EOC ---------------------------------- //
static ssize_t show_eoc(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);    
        struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
        struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4 *hwdev = &card->hwdev;
	struct sdfe4_channel *ch = &hwdev->ch[sg17_sci_if2ch(s,nl->number)];
	char *ptr;
	int size;

	if( (size = sdfe4_eoc_rx(ch,&ptr)) < 0 )
		return 0;
	memcpy(buf,ptr,size);
	kfree(ptr);
	return size;
}

static ssize_t
store_eoc(struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
        struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
        struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4 *hwdev = &card->hwdev;
	sdfe4_eoc_tx(hwdev,sg17_sci_if2ch(s,nl->number),(char*)buf,size);
        return size;
}
static CLASS_DEVICE_ATTR(eoc, 0644 ,show_eoc,store_eoc);


// ------------------------- HDLC 0/1 ---------------------------------------- //

// CRC count attribute 
static ssize_t
show_crc16(struct class_device *cdev, char *buf) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
        struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->crc16 )
		return snprintf(buf,PAGE_SIZE,"crc16");
        else
		return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t
store_crc16( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
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
		cfg_bt=ioread8( &(nl->regs->CRA)) | CMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
        case '0':
		if( !(cfg->crc16) )
		        break;
		cfg->crc16=0;
		cfg_bt=ioread8( &(nl->regs->CRA)) & ~CMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	}	
        return size;	
}
static CLASS_DEVICE_ATTR(crc16, 0644 ,show_crc16,store_crc16);

// fill byte value
static ssize_t
show_fill_7e(struct class_device *cdev, char *buf) 
{
        struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
        struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->fill_7e )
		return snprintf(buf,PAGE_SIZE,"7E");
        else
		return snprintf(buf,PAGE_SIZE,"FF");    
}

static ssize_t
store_fill_7e( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
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
		cfg_bt=ioread8( &(nl->regs->CRA)) | FMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
        case '0':
		if( !(cfg->fill_7e) )
			break;
		cfg->fill_7e=0;
		cfg_bt=ioread8( &(nl->regs->CRA)) & ~FMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
        }	
	return size;	
}
static CLASS_DEVICE_ATTR(fill_7e, 0644 ,show_fill_7e,store_fill_7e);
// data inversion
static ssize_t
show_inv(struct class_device *cdev, char *buf) 
{
        struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
        struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->inv )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off");    
}

static ssize_t
store_inv( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
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
		cfg_bt=ioread8(&(nl->regs->CRA)) | PMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	case '0':
		if( !(cfg->inv) )
		        break;
		cfg->inv=0;
		cfg_bt=ioread8(&(nl->regs->CRA)) & ~PMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
        }	
	return size;	
}
static CLASS_DEVICE_ATTR(inv, 0644 ,show_inv,store_inv);

// PCI read burst on/off
static ssize_t
show_rburst(struct class_device *cdev, char *buf) 
{
        struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
        struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->rburst )
		return snprintf(buf,PAGE_SIZE,"on");
        else
		return snprintf(buf,PAGE_SIZE,"off");    

}

static ssize_t
store_rburst( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
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
		cfg_bt=ioread8(&(nl->regs->CRB)) | RDBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	case '0':
		if( !(cfg->rburst) )
			break;
		cfg->rburst=0;
		cfg_bt=ioread8(&(nl->regs->CRB)) & ~RDBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
        }	
        return size;	
}
static CLASS_DEVICE_ATTR(rburst, 0644 ,show_rburst,store_rburst);

// PCI write burst
static ssize_t
show_wburst(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
        struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	return snprintf(buf,PAGE_SIZE,"%s",cfg->wburst ? "on" : "off");
}

static ssize_t
store_wburst( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
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
		cfg_bt=ioread8(&(nl->regs->CRB)) | WTBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	case '0':
		if( !(cfg->wburst) )
			break;
		cfg->wburst=0;
		cfg_bt=ioread8(&(nl->regs->CRB)) & ~WTBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
        }	
        return size;	
}
static CLASS_DEVICE_ATTR(wburst, 0644 ,show_wburst,store_wburst);

// MAC address less significant value 
static ssize_t
store_maddr( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
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
static CLASS_DEVICE_ATTR(maddr, 0200 ,NULL,store_maddr);

// ------------------------- Statistics ------------------------------- //
// PCI write burst
static ssize_t
show_statistics(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
        struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
        struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4_stat statistic, *stat=&statistic;
	
	if( sdfe4_get_statistic(sg17_sci_if2ch(s,nl->number),s->hwdev,stat) )
    		return snprintf(buf,PAGE_SIZE,"Error Getting statistic");
	return snprintf(buf,PAGE_SIZE,"SNR_Marg(%d), LoopAtten(%d), ES_count(%u), SES_Count(%u)\n"
				"CRC_Anom_count(%u), LOSWS_count(%u), UAS_count(%u), SegAnomaly_Count(%u)\n"
				"SegDefect_count(%u), CounterOverfInd(%u), CounterResetInd(%u)\n",
			        stat->SNR_Margin_dB,stat->LoopAttenuation_dB,stat->ES_count,stat->SES_count,
				stat->CRC_Anomaly_count,stat->LOSWS_count,stat->UAS_Count,stat->SegmentAnomaly_Count,
			        stat->SegmentDefectS_Count,stat->CounterOverflowInd,stat->CounterResetInd );
												
}

static ssize_t
store_statistics( struct class_device *cdev,const char *buf, size_t size ) 
{
//        struct net_device *ndev = to_net_dev(cdev);
//	struct net_local *nl = netdev_priv(ndev);
//	struct hdlc_config *cfg=&(nl->hdlc_cfg);

        if( !size )	return 0;

	switch(buf[0] == '1'){
        }	
        return size;	
}
static CLASS_DEVICE_ATTR(statistics,0644,show_statistics,store_statistics);


// ------------------------- Compatibility ------------------------------- //
// NSGate compatibility
static ssize_t
show_nsg_comp(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
        struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	return snprintf(buf,PAGE_SIZE,"%s", nl->nsg_comp ? "on" : "off");
}

static ssize_t
store_nsg_comp( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
    
        if( !size )	return 0;

	switch(buf[0]){
	case '1':
		nl->nsg_comp = 1;
		break;
	case '0':
		nl->nsg_comp = 0;	
		break;
        }	
        return size;	
}
static CLASS_DEVICE_ATTR(nsg_comp, 0644 ,show_nsg_comp,store_nsg_comp);


// ------------------------- DEBUG ---------------------------------------- //

// debug_verbosity
static ssize_t
store_debug_on( struct class_device *cdev,const char *buf, size_t size ) 
{
//        struct net_device *ndev = to_net_dev(cdev);
//	struct net_local *nl = netdev_priv(ndev);
        // if interface is up 
        if( !size )	return size;
        if( buf[0] == '1' )
                debug_link=0;
	else
		debug_link=40;
        return size;
}
static CLASS_DEVICE_ATTR(debug_on, 0200 ,NULL,store_debug_on);


// hdlc registers
static ssize_t
show_sg17_regs(struct class_device *cdev, char *buf) 
{                                                                       
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,  "CRA(%02x),CRB(%02x),SR(%02x),IMR(%02x)\n"
					"CTDR(%02x),LTDR(%02x),CRDR(%02x),LRDR(%02x)\n"
					"RATE(%02x)\n",
					nl->regs->CRA,nl->regs->CRB,nl->regs->SR,nl->regs->IMR,
					*nl->tx.CxDR,*nl->tx.LxDR,*nl->rx.CxDR,*nl->rx.LxDR,
					nl->regs->RATE);
}

static ssize_t
store_sg17_regs( struct class_device *cdev,const char *buf, size_t size )
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
        u8 tmp;
	char *endp;
	PDEBUG(0,"buf[0]=%d",buf[0]);
        if( !size ) return 0;
	PDEBUG(0,"buf[0]=%d, %c",buf[0],buf[0]);
	if( buf[0] < '0' || buf[0] > '8' )
		return size;
	tmp=simple_strtoul( buf+2,&endp,16) & 0xff;
	PDEBUG(0,"buf[0]=%d, tmp=%02x, terget=%08x",buf[0],(u32)tmp,(u32)((u8*)nl->regs + buf[0]));	
	*((u8*)nl->regs + buf[0]) = tmp;
	return size;
}

static CLASS_DEVICE_ATTR(regs,0644,show_sg17_regs,store_sg17_regs);


// set|unset loopback
static ssize_t
store_loopback( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
        // if interface is up 
        if( !size )	return size;
        if( buf[0] == '1' )
                nl->regs->CRA |= DLBK;
	else
	        nl->regs->CRA &= (~DLBK);
        return size;
}
static CLASS_DEVICE_ATTR(loopback, 0200 ,NULL,store_loopback);


int sg17_start_xmit( struct sk_buff *skb, struct net_device *ndev );
static ssize_t
store_xmit_tst( struct class_device *cdev,const char *buf, size_t size ) 
{
        struct net_device *ndev = to_net_dev(cdev);
	struct sk_buff *skb;	
        // if interface is up 
        if( !size )	return size;
	
        PDEBUG(0,"TEST send");
        skb = dev_alloc_skb(ETHER_MAX_LEN);
        skb_put( skb, 200);
	sg17_start_xmit(skb,ndev);
        return size;
}
static CLASS_DEVICE_ATTR(xmit_tst, 0200 ,NULL,store_xmit_tst);

// ------------------------------------------------------------------------ //
static struct attribute *sg17_attr[] = {
	// shdsl
        &class_device_attr_mode.attr,
        &class_device_attr_annex.attr,
        &class_device_attr_rate.attr,
        &class_device_attr_tcpam.attr,
        &class_device_attr_apply_cfg.attr,
	// EOC
	&class_device_attr_eoc.attr,	
	// HDLC
	&class_device_attr_crc16.attr,
        &class_device_attr_fill_7e.attr,
        &class_device_attr_inv.attr,
	// PCI
        &class_device_attr_rburst.attr,
        &class_device_attr_wburst.attr,
	// net device
	&class_device_attr_maddr.attr,
	// statistics
	&class_device_attr_statistics.attr,
	// compatibility
	&class_device_attr_nsg_comp.attr,	
        // debug
	&class_device_attr_debug_on.attr,	
	&class_device_attr_regs.attr,
	&class_device_attr_loopback.attr,
	&class_device_attr_xmit_tst.attr,
	NULL
};

static struct attribute_group sg17_group = {
        .name  = "sg17_private",
        .attrs  = sg17_attr,
};

int
sg17_sysfs_register(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);	
	return sysfs_create_group(&class_dev->kobj, &sg17_group);
}

void
sg17_sysfs_remove(struct net_device *ndev){
	struct class_device *class_dev = &(ndev->class_dev);	
	sysfs_remove_group(&class_dev->kobj, &sg17_group);
}
