/*
 * sg_hdlc.h,v 1.00 2006/09/04
 *
 * Definitions for Sigrand HDLC controller 
 * Copyright (C) 2006, Artem U. Polyakov (art@sigrand.ru)
 */

#ifndef SG_HDLC_H
#define SG_HDLC_H


/* CR bits */
#define TXEN    0x01            /* transmitter enable */
#define RXEN    0x02            /* receiver  enable */
#define NCRC    0x04            /* ignore received CRC */
#define DLBK    0x08            /* digital loopback */
#define CMOD    0x10            /* 0 - use CRC-32, 1 - CRC-16 */
#define FMOD    0x20            /* interframe fill: 0 - all ones, 1 - 0xfe */
#define PMOD    0x40            /* data polarity: 0 - normal, 1 - invert */
#define XRST    0x80            /* reset the transceiver */

/* CRB bits */
#define RDBE    0x01            /* read burst enable */
#define WTBE    0x02            /* write burst enable */
#define RODD    0x04            /* receive 2-byte alignment */
#define RXDE    0x08            /* receive data enable */
#define FRM	0x10		/* framed mode */
#define EXTC	0x20		/* sync from external generator */

/* SR and IMR bits */
#define TXS     0x01            /* transmit success */
#define RXS     0x02            /* receive success */
/* SR only */
#define CRC     0x04            /* CRC error */
#define OFL     0x08            /* fifo overflow error */
#define UFL     0x10            /* fifo underflow error */
#define EXT     0x20            /* interrupt from sk70725 */
// IMR only 
#define TSI     0x80            /* generate test interrupt */

#define LAST_FRAG 0x00008000

#define ETHER_MIN_LEN   64
#define SG16_MAX_FRAME  (1536 + 16)

#define XQLEN   8
#define RQLEN   8

// We don't have official vendor id yet...
#define MR16G_PCI_VENDOR 0x55
#define MR16G_PCI_DEVICE 0x9b

// Portability 
#define iotype void*
//#define IO_READ_WRITE
#ifndef IO_READ_WRITE
#       define iowrite8(val,addr)  writeb(val,addr)
#       define iowrite32(val,addr)  writel(val,addr)
#       define ioread8(addr) readb(addr)
#       define ioread32(addr) readl(addr)
#endif

// E1
#define MAX_TS_BIT 32

// macroses
#define mr16g_priv(ndev) ((struct net_local*)(dev_to_hdlc(ndev)->priv))
#define mr16g_e1cfg(ndev) ((struct ds2155_config*)&(mr16g_priv(ndev)->e1_cfg))
#define mr16g_hdlcfg(ndev) ((struct hdlc_config*)&(mr16g_priv(ndev)->hdlc_cfg))
	


struct ds2155_config{
    u32 slotmap;
    u8 framed	:1;
    u8 int_clck :1;
    u8 cas 	:1;
    u8 crc4 	:1;
    u8 ts16     :1;
    u8 hdb3 	:1;
    u8 long_haul:1;
    
};

struct hdlc_config
{
        u8  crc16: 1;
        u8  fill_7e: 1;
	u8  inv: 1;
	u8  rburst: 1;
	u8  wburst: 1;
};
		    

struct mr16g_hw_regs {
        u8  CRA, CRB, SR, IMR, CTDR, LTDR, CRDR, LRDR,MAP0,MAP1,MAP2,MAP3;
};

struct mr16g_hw_descr{
        u32  address;
	u32  length;
};
																	 

struct net_local{

	// standard net device statictics
	struct net_device_stats     stats;
	
	// device entity
	struct device *dev;
	// configuration	
	struct ds2155_config e1_cfg;
	struct hdlc_config hdlc_cfg;
	   
	// IO memory map
	void *mem_base;
	volatile struct mr16g_hw_regs *hdlc_regs;
	volatile struct mr16g_hw_descr    *tbd;
	volatile struct mr16g_hw_descr    *rbd;
	volatile u8 *ds2155_regs;
	
	//concurent racing 
	spinlock_t rlock,xlock;
	
        // transmit and reception queues 
        struct sk_buff *xq[ XQLEN ], *rq[ RQLEN ];
	unsigned head_xq, tail_xq, head_rq, tail_rq;
	    
	// the descriptors mapped onto the first buffers in xq and rq 
	unsigned head_tdesc, head_rdesc;
		    	
};

#endif
