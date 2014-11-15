/* $Id: at76_ieee802_11.h,v 1.1 2004/08/18 22:01:45 jal2 Exp $ */

/* Copyright (c) 2003 Oliver Kurth
 * Copyright (c) 2004 Joerg Albert <joerg.albert@gmx.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 *
 * This file is part of the Berlios driver for WLAN USB devices based on the
 * Atmel AT76C503A/505/505A. See at76c503.h for details.
 */

#ifndef _IEEE802_11_H
#define _IEEE802_11_H

struct ieee802_11_hdr {
	u16 frame_ctl;
	u16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	u16 seq_ctl;
} __attribute__ ((packed));

/* max. length of frame body, incl. IV and ICV fields)
   see 802.11(1999), section 7.1.2 */
#define IEEE802_11_MAX_DATA_LEN		(4+2304+4)

/* + 4 at the end for the FCS (Do we get it from the device ???) */
#define IEEE802_11_MAX_FRAME_LEN  \
      (sizeof(struct ieee802_11_hdr) + IEEE802_11_MAX_DATA_LEN + 4)

//#define IEEE802_11_HLEN			30
//#define IEEE802_11_FRAME_LEN		(IEEE802_11_DATA_LEN + IEEE802_11_HLEN)

/* defines for information element coding:
   1 byte ID, 1 byte length of information field, n bytes information field
   (see 7.3.2 in [1]) */
#define IE_ID_SSID 0              /* length 0 - 32 */
#define IE_ID_SUPPORTED_RATES 1
#define IE_ID_DS_PARAM_SET 3
#define IE_ID_CF_PARAM_SET 4
#define IE_ID_TIM 5
#define IE_ID_IBSS_PARAM_SET 6
#define IE_ID_CHALLENGE_TEXT 16

/* we must convert frame_control to cpu endianess before reading it. */

/* Frame control field constants, see 802.11 std, chapter 7.1.3, pg. 36 */
#define IEEE802_11_FCTL_VERS		0x0002
#define IEEE802_11_FCTL_FTYPE		0x000c
#define IEEE802_11_FCTL_STYPE		0x00f0
#define IEEE802_11_FCTL_TODS		0x0100
#define IEEE802_11_FCTL_FROMDS		0x0200
#define IEEE802_11_FCTL_MOREFRAGS	0x0400
#define IEEE802_11_FCTL_RETRY		0x0800
#define IEEE802_11_FCTL_PM		0x1000
#define IEEE802_11_FCTL_MOREDATA	0x2000
#define IEEE802_11_FCTL_WEP		0x4000
#define IEEE802_11_FCTL_ORDER		0x8000

/* frame type values */
#define IEEE802_11_FTYPE_MGMT		0x0000
#define IEEE802_11_FTYPE_CTL		0x0004
#define IEEE802_11_FTYPE_DATA		0x0008

/* management subtypes */
#define IEEE802_11_STYPE_ASSOC_REQ	0x0000
#define IEEE802_11_STYPE_ASSOC_RESP 	0x0010
#define IEEE802_11_STYPE_REASSOC_REQ	0x0020
#define IEEE802_11_STYPE_REASSOC_RESP	0x0030
#define IEEE802_11_STYPE_PROBE_REQ	0x0040
#define IEEE802_11_STYPE_PROBE_RESP	0x0050
#define IEEE802_11_STYPE_BEACON		0x0080
#define IEEE802_11_STYPE_ATIM		0x0090
#define IEEE802_11_STYPE_DISASSOC	0x00A0
#define IEEE802_11_STYPE_AUTH		0x00B0
#define IEEE802_11_STYPE_DEAUTH		0x00C0

/* control subtypes */
#define IEEE802_11_STYPE_PSPOLL		0x00A0
#define IEEE802_11_STYPE_RTS		0x00B0
#define IEEE802_11_STYPE_CTS		0x00C0
#define IEEE802_11_STYPE_ACK		0x00D0
#define IEEE802_11_STYPE_CFEND		0x00E0
#define IEEE802_11_STYPE_CFENDACK	0x00F0

/* data subtypes */
#define IEEE802_11_STYPE_DATA		0x0000
#define IEEE802_11_STYPE_DATA_CFACK	0x0010
#define IEEE802_11_STYPE_DATA_CFPOLL	0x0020
#define IEEE802_11_STYPE_DATA_CFACKPOLL	0x0030
#define IEEE802_11_STYPE_NULLFUNC	0x0040
#define IEEE802_11_STYPE_CFACK		0x0050
#define IEEE802_11_STYPE_CFPOLL		0x0060
#define IEEE802_11_STYPE_CFACKPOLL	0x0070

/* sequence control fragment / seq nr fields (802.12 std., ch. 7.1.3.4, pg. 40) */
#define IEEE802_11_SCTL_FRAG		0x000F
#define IEEE802_11_SCTL_SEQ		0xFFF0

/* capability field in beacon, (re)assocReq */
#define IEEE802_11_CAPA_ESS             0x0001
#define IEEE802_11_CAPA_IBSS            0x0002
#define IEEE802_11_CAPA_CF_POLLABLE     0x0004
#define IEEE802_11_CAPA_POLL_REQ        0x0008
#define IEEE802_11_CAPA_PRIVACY         0x0010
#define IEEE802_11_CAPA_SHORT_PREAMBLE  0x0020

/* auth frame: algorithm type */
#define IEEE802_11_AUTH_ALG_OPEN_SYSTEM 0x0000
#define IEEE802_11_AUTH_ALG_SHARED_SECRET 0x0001

/* disassoc/deauth frame: reason codes (see 802.11, ch. 7.3.1.7, table 18) */
#define IEEE802_11_REASON_UNSPECIFIED         0x0001
#define IEEE802_11_REASON_PREV_AUTH_INVALID   0x0002
#define IEEE802_11_REASON_DEAUTH_LEAVING      0x0003
#define IEEE802_11_REASON_DISASS_INACTIVITY   0x0004
#define IEEE802_11_REASON_DISASS_TOO_MANY_STA 0x0005
#define IEEE802_11_REASON_CL2_FROM_NONAUTH    0x0006
#define IEEE802_11_REASON_CL3_FROM_NONASSOC   0x0007
#define IEEE802_11_REASON_DISASS_LEAVING      0x0008
#define IEEE802_11_REASON_NOT_AUTH            0x0009

/* status in some response frames (802.11, ch. 7.3.1.9, table 19) */
#define IEEE802_11_STATUS_SUCCESS             0x0000
#define IEEE802_11_STATUS_UNSPECIFIED         0x0001
#define IEEE802_11_STATUS_UNSUPP_CAPABILITIES 0x000a
#define IEEE802_11_STATUS_NO_PREV_ASSOC       0x000b
#define IEEE802_11_STATUS_ASSOC_FAILED        0x000c
#define IEEE802_11_STATUS_UNSUPP_AUTH_ALG     0x000d
#define IEEE802_11_STATUS_AUTH_INV_TRANS_SEQ  0x000e
#define IEEE802_11_STATUS_AUTH_CHALLENGE_FAIL 0x000f
#define IEEE802_11_STATUS_AUTH_TIMEOUT        0x0010
#define IEEE802_11_STATUS_ASSOC_TOO_MANY_STA  0x0011
#define IEEE802_11_STATUS_BASIC_RATE_SET      0x0012

#endif /* _IEEE802_11_H */











