#ifndef SIGRAND_RING_H
#define SIGRAND_RING_H

// Errors
#define ERFULL 20 // Ring is full
#define ERINIT 21 // incorrect ring initialisation

#define HW_RING_MASK 0x7f
#define LAST_FRAG 0x00008000
#define ETHER_MAX_LEN   1536 - 4 // Find out clearly
#define IP_ALIGN 16

#define SW_RING_LEN 8
#define SW_RING_MASK 7




struct sg_hw_descr {
	u32  addr;
	u32  len;
}; 


struct sg_ring{
	// Hardware ring related variables
	// CxDR = { CRDR,CTDR } - Current processing descriptor
	// LxDR = { LRDR,LTDR } - Last processing descriptor
	// FxDR = { FRDR,FTDR } - First used descriptor
	struct sg_hw_descr *hw_ring;
	u8 hw_mask;
	u8 *CxDR,*LxDR,FxDR;
	// type: RX_RING || TX_RING
	u8 type;
	#define RX_RING 1
	#define TX_RING 2
	// Software control ring, wich has smaller size
	struct sk_buff *sw_ring[SW_RING_LEN];
	u8 sw_mask;
        u8 head, tail; 
	// OS specific
	spinlock_t lock;
	struct device *dev;
};

/*
inline u8 sg_ring_inc(u8 ind,int mask);
inline u8 sg_ring_dec(u8 ind,int mask);
int sg_ring_have_space(struct sg_ring *r);
int sg_ring_add_skb(struct sg_ring *r, struct sk_buff *skb);
struct sk_buff *sg_ring_del_skb(struct sg_ring *r,int *len);
*/

#endif

