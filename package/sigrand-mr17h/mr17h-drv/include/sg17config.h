/* sg17config.h:
 *
 * SDFE4 Library
 *
 *      Target device configuration
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */
        
#ifndef SG17_CONFIG_H
#define SG17_CONFIG_H

// Wat device you are using???

#define SG17_PCI_MODULE
//#define SG17_REPEATER 1

#if defined(SG17_PCI_MODULE) && defined(SG17_REPEATER)
#	error "You define SG17_PCI_MODULE & SG17_REPEATER at the same time!"
#elif !defined(SG17_PCI_MODULE) && !defined(SG17_REPEATER)
#	error "Please specify target device for library!"
#endif //SG17_PCI_MODULE && SG17_REPEATER


#endif
