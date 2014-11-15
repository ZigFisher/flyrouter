#include<linux/time.h>
#include<linux/delay.h>
#include<linux/interrupt.h>
#include<linux/netfilter.h>

/* functions */
void debug_count_delta(struct timeval *tv1,struct timeval tv2);
void debug_print_tv(struct timeval *tv,u32 cnt,struct timeval *tv1,char *str);
void debug_div_tv(struct timeval *tv1,u32 z);
void debug_sum_tv(struct timeval *tv1,struct timeval tv2);



/* debug macroses */
#define DEBUG_NET

#ifdef DEBUG_NET

#define DEBUG_NF_HOOK(pf, hook, skb, indev, outdev, okfn,counter_var)	\
({int __ret; 								\
struct timeval tv1,tv2; 						\
disable_irq(9); 							\
do_gettimeofday(&tv1); 							\
__ret=nf_hook_thresh(pf, hook, &(skb), indev, outdev, okfn,INT_MIN, 1); \
do_gettimeofday(&tv2); 							\
if( __ret == 1 ) 							\
	__ret = (okfn)(skb);                                            \
enable_irq(9); 								\
debug_count_delta(&tv1,tv2); 						\
debug_sum_tv(& debug_ip_##counter_var,tv1); 				\
debug_ip_##counter_var##_cnt+=1;  					\
__ret; })

#define DEBUG_NF_HOOK_COND(pf, hook, skb, indev, outdev, okfn, cond,counter_var) \
({int __ret; \
struct timeval tv1,tv2; \
disable_irq(9); \
do_gettimeofday(&tv1); \
__ret=NF_HOOK_COND(pf, hook, skb, indev, outdev, okfn, cond); \
do_gettimeofday(&tv2); \
enable_irq(9); \
debug_count_delta(&tv1,tv2); \
debug_sum_tv(& debug_ip_ ## counter_var,tv1); \
debug_ip_##counter_var##_cnt+=1 ;\
__ret; })


#define DEBUG_ADD_NEW_DELTA(tv1,tv2,counter_var) 	\
({ 								\
debug_count_delta(&tv1,tv2); 					\
debug_sum_tv(& debug_ip_ ## counter_var,tv1); 			\
debug_ip_##counter_var##_cnt+=1 ;				\
})

#else /* Not defined DEBUG_NET */

#define DEBUG_NF_HOOK(pf, hook, skb, indev, outdev, okfn,counter_var) \
({int __ret; \
__ret=NF_HOOK(pf, hook, skb, indev, outdev, okfn); \
__ret; })

#define DEBUG_NF_HOOK_COND(pf, hook, skb, indev, outdev, okfn, cond,counter_var) \
({ int __ret; \
__ret=NF_HOOK_COND(pf, hook, skb, indev, outdev, okfn, cond); \
__ret; })

#endif /* DEBUG_NET */


#define DEBUG_NET_OUTPUT(counter_var,str) \
({	struct timeval rez;\
	memset(&rez,0,sizeof(struct timeval)); \
	if ( debug_ip_##counter_var##_cnt ){ \
		rez.tv_sec=debug_ip_##counter_var.tv_sec; \
		rez.tv_usec=debug_ip_##counter_var.tv_usec; \
		debug_div_tv(&rez, debug_ip_##counter_var##_cnt ); \
	} \
	debug_print_tv(&rez,debug_ip_##counter_var##_cnt,&debug_ip_##counter_var,str);\
})



/* counters */
	

extern struct timeval debug_ip_arp;
extern struct timeval debug_ip_igmp;
extern struct timeval debug_ip_input;
extern struct timeval debug_ip_output;
extern struct timeval debug_ip_forward;
extern struct timeval debug_ip_hook;
extern struct timeval debug_ip_test;

extern struct timeval debug_ip_vs_xmit;
extern struct timeval debug_ip_ipt_REJECT;
extern struct timeval debug_ip_raw;
extern struct timeval debug_ip_xfrm4_input;
extern struct timeval debug_ip_xfrm4_output;
extern struct timeval debug_ip_ipmr;



extern u32 debug_ip_arp_cnt;
extern u32 debug_ip_igmp_cnt;
extern u32 debug_ip_input_cnt;
extern u32 debug_ip_output_cnt;
extern u32  debug_ip_ipmr_cnt;
extern u32  debug_ip_forward_cnt;
extern u32  debug_ip_hook_cnt;
extern u32  debug_ip_test_cnt;

