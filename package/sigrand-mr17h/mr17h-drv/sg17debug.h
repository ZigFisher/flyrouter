#ifndef SG_DEBUG_H
#define SG_DEBUG_H

#ifndef DEFAULT_LEV 
#	define DEFAULT_LEV 0
#endif

#define PDEBUG(lev,fmt,args...)
#define PDEBUGL(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...) \
		if( lev<=DEFAULT_LEV ) \
			printk(KERN_NOTICE "sg17lan: %s " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...) \
		if( lev<=DEFAULT_LEV ) \
			printk(fmt, ## args  )

#endif

extern int debug_xmit;
extern int debug_recv;
extern int debug_irq;
extern int debug_sci;
extern int debug_init;
extern int debug_sdfe4;
extern int debug_cur;
extern int debug_netcard;
extern int debug_link;
extern int debug_error;
extern int debug_eoc;

#endif		    
