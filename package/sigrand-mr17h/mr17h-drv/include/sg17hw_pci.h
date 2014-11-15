/* sg17hw_pci.h:
 *
 * SDFE4 Library
 *
 *      Definitions for SG-17PCI target device
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */

#ifndef SG17PCI_HW_H
#define SG17PCI_HW_H

#include <asm/types.h>
#include <linux/netdevice.h>
#include "sg17hw_core.h"

// CRA bits
#define TXEN    0x01            // transmitter enable
#define RXEN    0x02            // receiver  enable
#define NCRC    0x04            // ignore received CRC
#define DLBK    0x08            // digital loopback
#define CMOD    0x10            // 0 - use CRC-32, 1 - CRC-16
#define FMOD    0x20            // interframe fill: 0 - all ones, 1 - 0xfe
#define PMOD    0x40            // data polarity: 0 - normal, 1 - invert
#define XRST    0x80            // reset the transceiver

// CRB bits
#define RDBE    0x01            // read burst enable
#define WTBE    0x02            // write burst enable
#define RODD    0x04            // receive 2-byte alignment
#define RXDE    0x08            // receive data enable

// SR and IMR bits
#define TXS     0x01            // transmit success
#define RXS     0x02            // receive success
// SR and IMR bits
#define TXS     0x01            // transmit success
#define RXS     0x02            // receive success
#define CRC     0x04            // CRC error
#define OFL     0x08            // fifo overflow error
#define UFL     0x10            // fifo underflow error
#define EXT     0x20            // interrupt from sk70725
#define COL     0x40            // interrupt from sk70725
// IMR only
#define TSI     0x80            // generate test interrupt


//---- SG17-PCI IO memory ----//
#define SG17_OIMEM_SIZE 0x4000
#define SG17_HDLC_MEMSIZE 0x1000
#define SG17_SCI_MEMSIZE 0x2000
// mem offsets
#define SG17_HDLC_CH0_MEMOFFS 0x0000
#define SG17_HDLC_CH1_MEMOFFS (SG17_HDLC_CH0_MEMOFFS + SG17_HDLC_MEMSIZE)
#define SG17_SCI_MEMOFFS (SG17_HDLC_CH1_MEMOFFS + SG17_HDLC_MEMSIZE)
// HDLC channels mapping
#define HDLC_TXBUFF	0x0
#define HDLC_RXBUFF	0x400
#define HDLC_REGS	0x800
// SCI HDLC mapping
#define SCI_BUFF_SIZE	0x800
#define SCI_RXBUFF	0x0
#define SCI_TXBUFF	SCI_RXBUFF + SCI_BUFF_SIZE
#define SCI_REGS	SCI_TXBUFF + SCI_BUFF_SIZE

struct sdfe4{
        u8 msg_cntr;
        struct sdfe4_channel ch[SG17_IF_MAX];
        struct sdfe4_if_cfg cfg[SG17_IF_MAX];
        void *data;
};
				

#endif

