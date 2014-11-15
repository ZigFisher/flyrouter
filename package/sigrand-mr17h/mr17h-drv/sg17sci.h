#ifndef SG17_SCI_HDLC_H
#define SG17_SCI_HDLC_H

#include <asm/io.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include "include/sg17hw.h"

struct sg17_sci_regs{
	u8 __iomem CRA,CRB,SR,IMR;
	u16 __iomem TXLEN,RXLEN;
};

struct sg17_sci{
        void *mem_base;
        volatile struct sg17_sci_regs *regs;
        volatile u8 *rx_buf,*tx_buf;
	u8 rx_msg[SCI_BUFF_SIZE];
	u8 rx_len;
        // OS rlated objects
	int irq;
	wait_queue_head_t  wait_q,eoc_wait_q;
	spinlock_t chip_lock;
	struct work_struct wqueue;
	// SDFE4 description
	struct sdfe4 *hwdev;
	// channel to if mapping
	u8 ch_map[SG17_IF_MAX];
        // statistics
        unsigned long tx_bytes, rx_bytes;
        unsigned long tx_packets, rx_packets;
};
								
int sg17_sci_init( struct sg17_sci *, char *, struct sdfe4 *);
void sg17_sci_remove( struct sg17_sci *s );
int sg17_sci_enable( struct sg17_sci *s );
int sg17_sci_disable( struct sg17_sci *s );
irqreturn_t sg17_sci_intr(int  irq,  void  *dev_id,  struct pt_regs  *regs );
int sg17_sci_xmit( struct sg17_sci *, char *msg, int len);
int sg17_sci_recv_intr( struct sg17_sci *s, char **msg);
int sg17_sci_recv( struct sg17_sci *, char *msg, int *mlen);
int sg17_sci_wait_intr( struct sg17_sci * );
void sg17_sci_dump_regs( struct sg17_sci *s );
void sg17_sci_link_up(struct sg17_sci *s,int i);
void sg17_sci_link_down(struct sg17_sci *s,int i);
int sg17_sci_if2ch(struct sg17_sci *s,int if_num);



#endif
