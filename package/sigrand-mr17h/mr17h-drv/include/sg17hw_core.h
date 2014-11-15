/* sg17hw_core.h:
 *
 * SDFE4 Library
 *
 *      Universal target device definitions
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */

#ifndef SG17HW_CORE_H
#define SG17HW_CORE_H

#include "sg17eoc.h"

#define EOC_MSG_MAX_LEN 112

struct sdfe4_channel{
        u16 state               :3;
        u16 state_change        :1;
        u16 conn_state          :5;
	u16 conn_state_change   :1;
	u16 sdi_dpll_sync       :1;
        u16 enabled             :1;
	struct sg17_eoc *eoc;
	u8 perf_prims;
        u8 eoc_rx_msg[EOC_MSG_MAX_LEN];
};
											
#define TCPAM16 0
#define TCPAM32 1

struct sdfe4_if_cfg{
	u32 mode : 2;
        u32 repeater : 1;
        u32 startup_initialization:8;
        u32 transaction: 2;
        u32 annex:2;
	u32 input_mode:8;
        u32 loop :2;
        u32 tc_pam :1;
        u32 need_reconf :1;
	u32 :5;
        u16 frequency;
        u16 payload_bits;
        u16 rate;
	
};

struct sdfe4_stat {
        u8 SNR_Margin_dB;
        u8 LoopAttenuation_dB;
        u8 ES_count;
        u8 SES_count;
        u16 CRC_Anomaly_count;
        u8 LOSWS_count;
        u8 UAS_Count;
        u16 SegmentAnomaly_Count;
        u8 SegmentDefectS_Count;
        u8 CounterOverflowInd;
        u8 CounterResetInd;
/*
        u16 tx_sync_word;
        u16 rx_sync_word;
        u8 tx_stuff_bits;
        u8 rx_stuff_bits;
        s8 pow_backoff;
        s8 pow_backoff_farend;
        u8 ghs_pwr_lev_carr;
        u8 bits_p_symbol;
*/	
};
																							
#define SG17_IF_MAX     4

#endif

