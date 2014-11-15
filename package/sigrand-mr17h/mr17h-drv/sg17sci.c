/* sg17_sci.c:  Sigrand SG-17PCI SCI HDLC for linux 2.6.x
 *
 *	Written 2006-2007 by Artem U. Polyakov <art@sigrand.ru>
 *
 *	Provide interface to SCI HDLC controller, wich control 
 *	Infineon SDFE4 chipset.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/version.h>

#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>


//---- Local includes ----//
#include "sg17sci.h"
#include "sg17lan.h"
#include "include/sg17hw.h"
#include "include/sdfe4_lib.h"
// Debug parameters
//#define DEBUG_ON
#define DEFAULT_LEV 20
#include "sg17debug.h"

void sg17_sci_monitor(void *data);


/*
 * SCI HDLC
 */ 
 
int
sg17_sci_init( struct sg17_sci *s,char *card_name,struct sdfe4 *hwdev)
{
	// init registers & buffers
	s->tx_buf = (u8*)s->mem_base + SCI_RXBUFF;
	s->rx_buf = (u8*)s->mem_base + SCI_TXBUFF;
	s->regs = (struct sg17_sci_regs *)((u8*)s->mem_base + SCI_REGS);
	s->tx_packets = 0;
	s->tx_bytes = 0;
	s->rx_packets = 0;
	s->rx_bytes = 0;
	
	s->ch_map[0] = 0;
	s->ch_map[3] = 1;
	
	spin_lock_init(&s->chip_lock);
	init_waitqueue_head( &s->wait_q );
	init_waitqueue_head( &s->eoc_wait_q );	
	s->hwdev = hwdev;
	INIT_WORK( &s->wqueue, sg17_sci_monitor,(void*)s);

	PDEBUG(debug_sci,"init: base=%08x\ntx_buf=%08x,rx_buf=%08x,regs=%08x",
		(u32)s->mem_base,(u32)s->tx_buf,(u32)s->rx_buf,(u32)s->regs);
	PDEBUG(debug_sdfe4,"s(0x%08x): hwdev=%08x",(u32)s,(u32)hwdev);	
	// register interrupt handler
	if( request_irq( s->irq, sg17_sci_intr, SA_SHIRQ, card_name, (void*)s) ){
		printk( KERN_ERR "%s: unable to get IRQ %d.\n", card_name, s->irq );
                goto err_exit;
        }
	
	PDEBUG(debug_sci,"success");
	
	return 0;
err_exit:
	return -1;								
}

inline int
sg17_sci_enable( struct sg17_sci *s )
{
	iowrite8( 0,&s->regs->CRA);
	mdelay(100);
	iowrite8( (XRST | RXEN),&s->regs->CRA);
	iowrite8( 0xff,&s->regs->SR);		
	iowrite8( (RXS | TXS | CRC | COL),&s->regs->IMR);	
	mdelay(10);
	PDEBUG(debug_sci,"SCI enabled");
	schedule_delayed_work(&s->wqueue,2*HZ);	
	return 0;
}

inline int
sg17_sci_disable( struct sg17_sci *s )
{
	iowrite8( 0,&s->regs->CRA);
	iowrite8( 0,&s->regs->IMR);	
	iowrite8( 0xff ,&s->regs->SR );
	cancel_delayed_work(&s->wqueue);
	PDEBUG(debug_sci,"disabled");
	
	return 0;
}

void
sg17_sci_remove( struct sg17_sci *s )
{
	free_irq( s->irq, s);
}


void
sg17_sci_monitor(void *data)
{
	struct sg17_sci *s = (struct sg17_sci *)data;
        struct sdfe4 *hwdev = s->hwdev;
	int ret;
	PDEBUG(debug_netcard,"");
        ret = sdfe4_state_mon( hwdev );
	sg17_link_support(s);
	schedule_delayed_work(&s->wqueue,2*HZ);
}

irqreturn_t
sg17_sci_intr(int  irq,  void  *dev_id,  struct pt_regs  *regs )
{
	struct sg17_sci *s = (struct sg17_sci *)dev_id;
	u8 mask = ioread8(&s->regs->IMR);
	u8 status = (ioread8(&s->regs->SR) & mask);	
	struct sdfe4_msg msg;
	int pamdsl_type;
	u16 in_len;
	int i;
	
	PDEBUG(debug_sci,"status=%02x",status);	
	if( !status )
		return IRQ_NONE;

	iowrite8(status,&s->regs->SR); 	// ack all interrupts
	iowrite8(0,&s->regs->IMR); 	// disable interrupts
	
	if( status & TXS ){
		PDEBUG(debug_sci,"TXS");
		s->tx_packets++;
		s->tx_bytes += ioread16(&s->regs->TXLEN);
	}

	if( status & RXS ){
		PDEBUG(debug_sci,"RXS");
		// Save incoming message
		in_len = ioread16(&s->regs->RXLEN);
/*
if( s->rx_msg[3] !=0 ){
	PDEBUG(debug_eoc,"Come NFC:");
	for(i=0; i < s->rx_len; i++)
	    printk("%02x ",s->rx_msg[i]);
	printk("\n",s->rx_msg[i]);	    
}
*/	
		// process message		
		
    		if( !sdfe4_msg_init( &msg, s->rx_buf, in_len ) ){

			iowrite8( (ioread8( &s->regs->CRA ) | RXEN), &s->regs->CRA );		

			pamdsl_type = sdfe4_pamdsl_parse(&msg,s->hwdev);
			switch( pamdsl_type ){
			case SDFE4_NOT_PAMDSL:
			case SDFE4_PAMDSL_ACK:
				s->rx_len = in_len;
				memcpy(s->rx_msg,msg.buf,s->rx_len);
				wake_up( &s->wait_q );
				break;
			case SDFE4_PAMDSL_NFC:
				PDEBUG(debug_eoc,"NFC wake queue");
				wake_up( &s->eoc_wait_q );
			default:
				break;
			}
		}

		s->rx_packets++;
		s->rx_bytes += i;
//--------------DEBUG --------------------------
		PDEBUGL(debug_sci,"Incoming data: ");		
		for(i=0; i < s->rx_len; i++)
			PDEBUGL(debug_sci,"%02x ",s->rx_msg[i]);
		PDEBUGL(debug_sci,"\n");
//--------------DEBUG --------------------------
	}

	if( status & CRC ){
		PDEBUG(0/*debug_irq*/,"CRC");
	}
	
	if( status & COL ){
		PDEBUG(0/*debug_irq*/,"COL");
	}
	
	iowrite8(mask, &s->regs->IMR); // enable interrupts
	return IRQ_HANDLED;
}

int
sg17_sci_recv( struct sg17_sci *s, char *msg, int *mlen)
{
	int clen = s->rx_len;
	int i;	

	if( clen < 0 )
		return clen;
	if( *mlen < clen )
		return -EINVAL;

	for( i=0; i<clen; i++)
		msg[i] = s->rx_msg[i];

	// enable message receiving
//	iowrite8( (ioread8( &s->regs->CRA ) | RXEN), &s->regs->CRA );
	PDEBUG(debug_cur,"");	
	return 0;
}

int sg17_sci_xmit( struct sg17_sci *s, char *msg, int len)
{
	int i;
	u8 tmp = ioread8(&s->regs->CRA);
	// message is too long
	if( len > SCI_BUFF_SIZE )
		return -EINVAL;
	
	// somebody already send something, need wait
	if( tmp & TXEN )
		return -EAGAIN;
	
	for( i=0; i<len; i++)
		iowrite8( msg[i],(u8*)s->tx_buf + i);
	iowrite16( len, &s->regs->TXLEN);
	iowrite8( (ioread8(&s->regs->CRA) | TXEN ), &s->regs->CRA);
	
	return 0;
}

inline int
sg17_sci_wait_intr( struct sg17_sci *s )
{
	int ret=0;
	ret = interruptible_sleep_on_timeout( &s->wait_q, HZ*2 );
	PDEBUG(debug_sci,"return = %d",ret);	
	if( !( ioread8( &s->regs->CRA ) & RXEN) )
		return 1;
	return ret;
}

inline int
sg17_sci_wait_eoc_intr( struct sg17_sci *s )
{
	int ret=0;
	PDEBUG(debug_eoc,"start");	
	ret = interruptible_sleep_on_timeout( &s->eoc_wait_q, HZ*2 );
	PDEBUG(debug_eoc,"return = %d",ret);
	return ret;
}

inline void
sg17_sci_dump_regs( struct sg17_sci *s )
{
	PDEBUG(0,"CRA: %02x; SR:  %02x; IMR: %02x",s->regs->CRA,s->regs->SR,s->regs->IMR);
	PDEBUG(0,"TXLEN: %d; RXLEN:  %d",s->regs->TXLEN,s->regs->RXLEN);
}

inline void
sg17_sci_link_up(struct sg17_sci *s,int i){
        sg17_link_up(s,s->ch_map[i]);
}
		
inline void
sg17_sci_link_down(struct sg17_sci *s,int i){
        sg17_link_down(s,s->ch_map[i]);
}
			
int
sg17_sci_if2ch(struct sg17_sci *s,int if_num)
{
	int i;

	if( (if_num < 0) || (if_num > SG17_IF_MAX) ){
		PDEBUG(debug_error,"error if parameter: %d",if_num);
		return -1;
	}
	for( i=0;i<SG17_IF_MAX;i++){
		if( s->ch_map[i] == if_num )
			return i;
	}
	PDEBUG(debug_error,"if not finded");
	return -1;
}

//-------------- Interface to sdfe4_lib -----------------------------//

int
sdfe4_hdlc_xmit(u8 *msg,u16 len,struct sdfe4 *hwdev){
	return sg17_sci_xmit((struct sg17_sci *)hwdev->data,msg,len);
}

int
sdfe4_hdlc_wait_intr(int to,struct sdfe4 *hwdev){
	if( sg17_sci_wait_intr((struct sg17_sci *)hwdev->data) )
		return 0;
	return -1;
}



/*todo add length*/
int
sdfe4_hdlc_recv(u8 *buf,int *len,struct sdfe4 *hwdev){
	*len = SDFE4_FIFO8;
	return sg17_sci_recv((struct sg17_sci *)hwdev->data,buf,len);
}

void
wait_ms(int x){
	mdelay(x);
}

void
sdfe4_hdlc_regs(struct sdfe4 *hwdev){
	sg17_sci_dump_regs((struct sg17_sci *)hwdev->data);
}

void
sdfe4_link_led_up(int i,struct sdfe4 *hwdev){
	PDEBUG(debug_link,"chan#%d",i);
	sg17_sci_link_up((struct sg17_sci *)hwdev->data,i);
}

void
sdfe4_link_led_down(int i,struct sdfe4 *hwdev){
	PDEBUG(debug_link,"chan#%d",i);
	sg17_sci_link_down((struct sg17_sci *)hwdev->data,i);	
}

void
sdfe4_link_led_blink(int i, struct sdfe4 *hwdev){
	PDEBUG(debug_link,"chan#%d",i);
}

void
sdfe4_link_led_fast_blink(int i,struct sdfe4 *hwdev){
	PDEBUG(debug_link,"chan#%d",i);
}

inline void sdfe4_clear_channel(struct sdfe4 *hwdev) {}
inline void
sdfe4_memcpy(void *dst,const void *src,int size){	
	memcpy(dst,src,size);
}


// locking
inline void
sdfe4_lock_chip(struct sdfe4 *hwdev){
	struct sg17_sci *s = (struct sg17_sci *)hwdev->data;
	spin_lock(&s->chip_lock);
}
inline void
sdfe4_unlock_chip(struct sdfe4 *hwdev){
	struct sg17_sci *s = (struct sg17_sci *)hwdev->data;
	spin_unlock(&s->chip_lock);
}


// EOC related
int
sdfe4_eoc_wait_intr(int to,struct sdfe4 *hwdev){
	if( sg17_sci_wait_eoc_intr((struct sg17_sci *)hwdev->data) )
		return 0;
	return -1;
}
