#define PDEBUG(debug_lev,fmt,args...)

#ifdef ADM5120_DEBUG 
#	define DEBUG_1 1
#	define DEBUG_2 2
#	define DEBUG_3 3
#	define DEBUG_4 4
#	define DEBUG_5 5
#	ifndef DEBUG_DEF
#		define DEBUG_DEF 0
#	endif
#	undef PDEBUG
#	define PDEBUG(debug_lev,fmt,args...) \
    	    if( debug_lev < DEBUG_DEF ) \
		printk(KERN_NOTICE"%s: " fmt " \n", __FUNCTION__, ## args )
#endif
