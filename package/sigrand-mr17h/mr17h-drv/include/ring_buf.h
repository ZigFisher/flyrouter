#ifndef SG17_RING_BUF_H
#define SG17_RING_BUF_H

#define RING_SIZE 64
#define MAX_SBUFS 16

#ifdef __cplusplus
extern "C" {
#endif

struct rbuf_buf{
    char *ptr[MAX_SBUFS];
    unsigned short size[MAX_SBUFS];
    unsigned short used;
};

struct ring_buf{
    struct rbuf_buf rb[RING_SIZE];
    unsigned short head,tail;
};

struct ring_buf *rbuf_init(void);
void rbuf_free(struct ring_buf *r);
int rbuf_append(struct ring_buf *r,char *in_buf,int size,int end_flag);
void rbuf_abort_new(struct ring_buf *r);
int rbuf_get_cur(struct ring_buf *r,char **out_buf);

#ifdef __cplusplus
}
#endif

#endif
