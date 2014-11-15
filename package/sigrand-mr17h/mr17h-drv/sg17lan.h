/* sg17lan.h: Sigrand SG-17PCI SHDSL modem driver for linux (kernel 2.6.x)
 *
 *	Written 2006-2007 by Artem U. Polyakov <art@sigrand.ru>
 *
 *	This driver presents SG-17PCI modem 
 *	to system as common ethernet-like netcard.
 *
 */

#ifndef SG17LAN_H
#define SG17LAN_H

#include "sg17ring.h"
#include "sg17sci.h"
#include "include/sg17hw.h"
#include "include/sdfe4_lib.h"
#include <linux/netdevice.h>

// Portability 
#define iotype void*
//#define IO_READ_WRITE
#ifndef IO_READ_WRITE
#       define iowrite8(val,addr)  writeb(val,addr)
#       define iowrite16(val,addr)  writeb(val,addr)
#       define iowrite32(val,addr)  writel(val,addr)
#       define ioread8(addr) readb(addr)
#       define ioread16(addr) readb(addr)
#       define ioread32(addr) readl(addr)
#endif


//--- SG-17PCI Card control ----//
struct sg17_card{
	int number;
	int if_num;	
	char name[40];
	atomic_t locked; // if some of cards interfaces are up
	
	// PCI card fields
	struct pci_dev *pdev;
	void *mem_base;	

	struct sg17_sci sci;
	struct sdfe4 hwdev;
	// netdev fields
	struct net_device *ndevs[4];

};

#define TX_TIMEOUT      400
struct sg17_hw_regs {
        u8  CRA, CRB, SR, IMR, CTDR, LTDR, CRDR, LRDR, RATE;
};

struct hdlc_config{
	u8  crc16: 1;
	u8  fill_7e: 1;
	u8  inv: 1;
	u8  rburst: 1;
	u8  wburst: 1;
};
		    
			
struct net_local{
	int number;
	struct net_device_stats	stats;
        struct device *dev;
	// mapped memory address
	volatile struct sg17_hw_regs *regs;
        // transmit and reception queues 
	struct sg_ring rx,tx;
	struct sdfe4_if_cfg *shdsl_cfg;
	struct hdlc_config hdlc_cfg;
	// compatibilitys
	u8 nsg_comp :1;
	u8 :7;
	
};

void sg17_link_up(struct sg17_sci *s, int if_num);
void sg17_link_down(struct sg17_sci *s, int if_num);
void sg17_link_support(struct sg17_sci *s);


#endif
