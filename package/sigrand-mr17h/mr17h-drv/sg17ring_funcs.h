#ifndef SIGRAND_RING_FUNCS_H
#define SIGRAND_RING_FUNCS_H

#include <linux/ioport.h>
#include <linux/dma-mapping.h>
#include <linux/skbuff.h>

static inline u8
sg_ring_inc(u8 ind,int mask){
	return ((ind + 1) & mask) ;
}

static inline u8
sg_ring_dec(u8 ind,int mask){
	return ((ind - 1) & mask) ;
}

static int
sg_ring_have_space(struct sg_ring *r)
{
	int ret=1;

	// check that ring is not full
	if( sg_ring_inc(r->tail,r->sw_mask) == r->head ) {
		ret=0;
	}	
	return ret;
}


static inline int
sg_ring_add_skb(struct sg_ring *r, struct sk_buff *skb)
{
	u8 ind;
	int desc_len,dma_len;
	enum dma_data_direction dma_type;
	dma_addr_t bus_addr;
asm("#2");
	// check that ring is not full
	if( sg_ring_inc(r->tail,r->sw_mask) == r->head ) {
//		PDEBUG(0," %s ring: ring is full", (r->type==TX_RING) ? "TX" : "RX");
		return -ERFULL;
	}	
	// set some ring type specific parameters
	if( r->type == TX_RING ){
		desc_len = skb->len | LAST_FRAG;
		dma_len = skb->len;		
		dma_type = DMA_TO_DEVICE;
	}else if( r->type == RX_RING ){
		desc_len = 0;
		dma_len = ETHER_MAX_LEN;
		dma_type = DMA_FROM_DEVICE;
	}else{
		// Error case. Bad ring type initialisation
//		PDEBUG(0,"bad ring type initialisation");
		return -ERINIT;
	}
	// Map the buffer for DMA 
asm("#3");
	bus_addr = dma_map_single(r->dev,skb->data, dma_len, dma_type);
	// set hardware descriptors
	ind = ioread8( (u8*)(r->LxDR)) & (r->hw_mask);
	iowrite32(bus_addr,&(r->hw_ring[ ind ].addr)) ;
	iowrite32(desc_len,(u8*)&(r->hw_ring[ ind ].len)) ;
	ind = sg_ring_inc(ind,r->hw_mask);
	iowrite8(ind,(u8*)(r->LxDR));
	r->sw_ring[ r->tail ] = skb;
	r->tail=sg_ring_inc(r->tail,r->sw_mask);
asm("#3.1");	
	return 0;
}


static inline struct sk_buff *
sg_ring_del_skb(struct sg_ring *r,int *len)
{
        u8 ind;
	dma_addr_t bus_addr;
	int dma_len;
	enum dma_data_direction dma_type;
	struct sk_buff *skb;

	ind = ioread8((u8*)(r->CxDR));
	if( r->FxDR == ind ){
//		PDEBUG(0,"%s ring: nothing to del", (r->type==TX_RING) ? "TX" : "RX");	
		return NULL;
	}
	// set some ring type specific parameters
	if( r->type == TX_RING ){
		dma_len=r->sw_ring[r->head]->len;		
		dma_type=DMA_TO_DEVICE;
	}else if( r->type == RX_RING ){
		dma_len=ETHER_MAX_LEN;	
		dma_type=DMA_FROM_DEVICE;
	}else{
		// Error case. Bad ring type initialisation
//		PDEBUG(0,"bad ring type initialisation");
		return NULL;
	}

	// unmap DMA memory 
	bus_addr=ioread32( (u32*)&(r->hw_ring[ r->FxDR ].addr));
	*len = ioread32( (u32*)&(r->hw_ring[ r->FxDR ].len)) & 0x7ff;
	dma_unmap_single(r->dev,bus_addr, dma_len, dma_type );
	skb = r->sw_ring[ r->head ];
	r->sw_ring[ r->head ] = NULL;
	r->FxDR=sg_ring_inc(r->FxDR,r->hw_mask);
	r->head=sg_ring_inc(r->head,r->sw_mask);
	return skb;
}


#endif

