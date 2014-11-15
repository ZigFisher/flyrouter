#ifndef SG17_EOC_H
#define SG17_EOC_H

#include "ring_buf.h"

struct sg17_eoc
{
        u8 eoc_tx:2;
	u8 eoc_rx_new:1;
        u8 eoc_rx_drop:1;
	u8:4;
        struct ring_buf *buf;
};

static inline struct sg17_eoc*
eoc_init(void){
	struct sg17_eoc *e = kmalloc(sizeof(struct sg17_eoc),GFP_KERNEL);
	e->eoc_tx = 0;
	e->eoc_rx_new = 0;
	e->eoc_rx_drop = 0;
	e->buf = rbuf_init();
	return e;
}

static inline void 
eoc_free(struct sg17_eoc *e){
	rbuf_free(e->buf);
}


static inline int
eoc_append(struct sg17_eoc *s,char *ptr,int size,int end_flag){
	return rbuf_append(s->buf,ptr,size,end_flag);
}

static inline void
eoc_abort_new(struct sg17_eoc *s){
	rbuf_abort_new(s->buf);
}


static inline int
eoc_get_cur(struct sg17_eoc *s,char **ptr){
	return rbuf_get_cur(s->buf,ptr);
}

#endif
