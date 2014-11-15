/* sg17device.h:
 *
 * SDFE4 Library
 *
 *	Target device configuration
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */
       
#ifndef SG17_DEVICE_H
#define SG17_DEVICE_H

#include "sg17config.h"
#include "sg17hw.h"

int sdfe4_hdlc_xmit(u8 *msg,u16 len,struct sdfe4 *hwdev);
int sdfe4_hdlc_wait_intr(int to,struct sdfe4 *hwdev);
int sdfe4_hdlc_recv(u8 *buf,int *len,struct sdfe4 *hwdev);
void sdfe4_clear_channel(struct sdfe4 *hwdev);
// portability
void wait_ms(int x);
void sdfe4_memcpy(void *,const void *,int size);
// Link
int sdfe4_link_led_up(int i,struct sdfe4 *hwdev);
int sdfe4_link_led_down(int i,struct sdfe4 *hwdev);
int sdfe4_link_led_blink(int i, struct sdfe4 *hwdev);
int sdfe4_link_led_fast_blink(int i,struct sdfe4 *hwdev);
// locking
void sdfe4_lock_chip(struct sdfe4 *hwdev);
void sdfe4_unlock_chip(struct sdfe4 *hwdev);
// EOC
int sdfe4_eoc_wait_intr(int to,struct sdfe4 *hwdev);


#endif
