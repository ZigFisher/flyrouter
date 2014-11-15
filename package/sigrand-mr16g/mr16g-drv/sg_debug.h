#ifndef SG_DEBUG_H
#define SG_DEBUG_H


#define PDEBUG(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...) \
		if( lev<DEFAULT_LEV ) \
			printk(KERN_ERR "mr16g: %s " fmt " \n",__FUNCTION__, ## args  )
#endif

#endif		    
