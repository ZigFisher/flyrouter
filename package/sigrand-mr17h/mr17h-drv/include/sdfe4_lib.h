/* sdfe4_lib.h:
 *
 * SDFE4 Library
 *
 *      SDFE4 chipset description
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */
#ifndef SDFE4_LIB_H
#define SDFE4_LIB_H

#include "sg17hw.h"

#define SDFE4_EMB_NUM		4
// Protocol stack definitions
#define HDLC_HDR_SZ		0x02
#define TRANSP_HDR_SZ		0x02
#define MSG_RAM_CMDHDR_SZ	0x02
#define MSG_RAM_ACKHDR_SZ	0x00
#define RAM_CMDHDR_SZ		(HDLC_HDR_SZ+MSG_RAM_CMDHDR_SZ)
#define RAM_ACKHDR_SZ		(HDLC_HDR_SZ+MSG_RAM_ACKHDR_SZ)

#define MSG_AUX_CMDHDR_SZ	0x01
#define MSG_AUX_ACKHDR_SZ	0x00
#define AUX_CMDHDR_SZ		(HDLC_HDR_SZ+MSG_AUX_CMDHDR_SZ)
#define AUX_ACKHDR_SZ		(HDLC_HDR_SZ+MSG_AUX_ACKHDR_SZ)

#define MSG_EMB_HDR_SZ	0x04
/*TODO: check MSG_EMB_ACKHDR_SZ */
#define MSG_EMB_ACKHDR_SZ	0x04
#define EMB_CMDHDR_SZ		(HDLC_HDR_SZ+TRANSP_HDR_SZ+MSG_EMB_HDR_SZ)
#define EMB_ACKHDR_SZ		EMB_CMDHDR_SZ
#define EMB_NFCHDR_SZ		EMB_CMDHDR_SZ


#define RAM_HDLC_PKT_SZ8	HDLC_HDR_SZ+MSG_RAM_CMDHDR_SZ+RAM_FW_PKT_SZ8
#define RAM_HDLC_PKT_SZ32	RAM_HDLC_PKT_SZ8/4 + RAM_HDLC_PKT_SZ8%4

#define AUX_PKT_SZ8		0x1
#define AUX_HDLC_PKT_SZ8	HDLC_HDR_SZ+MSG_AUX_CMDHDR_SZ+AUX_PKT_SZ8

#define SDFE4_FIFO8	128
#define SDFE4_FIFO32	SDFE4_FIFO8/4

//
#define MSG_ONLY	0
#define TRANSP_USE  	1

#ifdef SG17_PCI_MODULE
#	define FW_PKT_SIZE             0x40
#else
#	define FW_PKT_SIZE             0x10
#endif
#define FW_PKT_SIZE32           FW_PKT_SIZE/4
#define FW_PKT_HDR_SIZE         0x08
#define FW_CODE_SIZE		0x1C000
#define FW_DATA_SIZE		0x8000
#define FW_CODE_OFFS		0x0
#define FW_DATA_OFFS		(FW_CODE_OFFS+FW_CODE_SIZE)
#define FW_CODE_CRC_OFFS	(FLASH_FW_STRTADDR+FW_DATA_OFFS+FW_DATA_SIZE)
#define FW_DATA_CRC_OFFS	(FW_CODE_CRC_OFFS+4)
#define FW_DTPNT_OFFS		(FW_DATA_CRC_OFFS+4)
#define FWdtpnt			0x120C0608 // 0x120C0608 ///0x08060C12;
#define FLASH_FW_STRTADDR	0x00010000
#define FLASH_FW_ENDADDR	0x0003400C
//*****************************************************************************

#define PEF24624_ADR_HOST	0xF1
/* example for a device address different from the default (0xF) */

#define PEF24624_ADR_DEV	0xF0
#define PEF24624_ADR_RAMSHELL	0x05
#define PEF24624_ADR_AUX	0x09

#define PEF24624_ADR_PAMDSL(Ch)     ((((Ch)<<2) | 0x02) & 0x0F)
#define PEF24624_PAMDSL_ADR(Ch)     ( ((Ch & 0xF)>>2) & 0x0F)

#define PEF24624_ADR_PAMDSL_0       0x02
#define PEF24624_ADR_PAMDSL_1       0x06
#define PEF24624_ADR_PAMDSL_2       0x0A
#define PEF24624_ADR_PAMDSL_3       0x0E
#define PEF24624_ADR_PAMDSL_GRP     0x0D
#define FWSTART_START_ch0           0x00000001
#define FWSTART_START_ch3           0x00000008
#define FWSTART_START_ch1           0x00000002
#define FWSTART_START_ch2           0x00000004
#define FWSTART_STARTALL            0x0000000F


#define  CMD_WR_REG_AUX_SCI_IF_MODE  0x00A9
#define  CMD_WR_RAM_RS               0x0003
#define  CMD_WR_REG_RS_FWSTART       0x0061
#define  CMD_RD_REG_RS_FWSTART       0x0060
#define  CMD_WR_REG_RS_FWCTRL        0x0001
#define  CMD_RD_REG_RS_FWCRC         0x0020
#define  CMD_RD_RAM_RS               0x0002
#define  CMD_WR_REG_RS_FWDTPNT       0x0041
#define  CMD_RD_REG_RS_FWDTPNT       0x0040
#define  CMD_WR_REG_AUX_AUX_IF_MODE  0x0081
#define  CMD_WR_REG_AUX_SDI_IF_SEL_0 0x0089
#define  CMD_WR_REG_AUX_SDI_IF_SEL_1 0x0091
#define  CMD_WR_REG_AUX_SDI_IF_SEL_2 0x0099
#define  CMD_WR_REG_AUX_SDI_IF_SEL_3 0x00A1
#define  CMD_CONNECT_CTRL            0x0C04
#define  ACK_CONNECT_CTRL            0x0E04
#define  CMD_CHANNEL_DISABLE         0x0C0F
#define  ACK_CHANNEL_DISABLE	     0x0E0F
#define  CMD_CFG_SYM_DSL_MODE        0x0404
#define  ACK_CFG_SYM_DSL_MODE        0x2622
#define  CMD_CFG_GHS_MODE            0x2422
#define  ACK_CFG_GHS_MODE            0x0604
#define  CMD_CFG_CAPLIST_SHORT       0x2432
#define  CMD_CFG_CAPLIST_SHORT_VER_2 0x2452
#define  ACK_CFG_CAPLIST_SHORT_VER_2 0x2652
#define  CMD_CFG_SDI_SETTINGS        0x840F
#define  ACK_CFG_SDI_SETTINGS        0x860F
#define  CMD_CFG_EOC_RX              0xA422
#define  ACK_CFG_EOC_RX              0xA622
#define  CMD_EOC_TX		     0xAC02
#define  ACK_EOC_TX                  0xAE02
#	define EOC_TX_ACK_POS	0x05
#	define EOC_TX_ACK_NEG	0x06
#define  CMD_GHS_REG_INITIATION      0x2C62
#define  ACK_GHS_REG_INITIATION      0x2E62
#define  CMD_CAPLIST_GET             0x2802
#define  ACK_CAPLIST_GET             0x2A02
#define  CMD_CFG_CAPLIST             0x2412
#define  ACK_CFG_CAPLIST             0x2612
#define  CMD_CONNECT_STAT_GET        0x0804
#define  CMD_CFG_SDI_RX              0x842F
#define  ACK_CFG_SDI_RX              0x862F
#define  CMD_CFG_SDI_TX              0x841F
#define  ACK_CFG_SDI_TX              0x861F
#define  CMD_DSL_PARAM_GET           0x4802
#define  ACK_DSL_PARAM_GET	     0x4A02
#define  CMD_CFG_MULTIWIRE_MASTER    0xB40F
#define  CMD_CFG_MULTIWIRE_SLAVE     0xB41F
#define  CMD_CFG_DSL_PARAM           0x4402
#define  CMD_PERF_STATUS_GET         0x9432
#define  ACK_PERF_STATUS_GET         0x9632
#define  ACK_CFG_MULTIWIRE_SDATA     0xB63F

#define  NFC_CONNECT_CTRL		0x0D04
#	define  MAIN_INIT		0x00
#	define  MAIN_PRE_ACT		0x01
#	define  MAIN_CORE_ACT		0x02
#	define  MAIN_DATA_MODE		0x03
#	define  MAIN_EXCEPTION		0x05
#	define  MAIN_TEST		0x06
#define  NFC_CONNECT_CONDITION		0x0D14
#	define  GHS_STARTUP		0x06
#	define  GHS_TRANSFER		0x01
#	define  EXCEPTION		0x04
#	define  GHS_30SEC_TIMEOUT	0x0e
#define  NFC_SDI_DPLL_SYNC		0x855F
#define  NFC_MPAIR_DELAY_MEASURE_SDFE4	0x8D0F
#define  NFC_PERF_PRIM			0x9912
#define  NFC_FBIT_RX			0x9952
#define  NFC_EOC_TX			0xAD02
#	define EOC_TX_IDLE		0x01
#	define EOC_TX_READY		0x02
#	define EOC_TX_FRAME		0x03
#	define EOC_TX_ABORT		0x04
#define  NFC_EOC_RX			0xA912
#define  NFC_UNDEF_MSG_ID		0x010F
#define  NFC_MULTIWIRE_MASTER		0xB94F
#define  NFC_MULTIWIRE_PAIR_NR		0xB96F

#define  SLIP_FAST                   0x00
#define  SDI_FALLING                 0x00
#define  SDI_RISING                  0x01
#define  SDI_HIGH                    0x01
#define  MERGED_CL_SRU               0x4
#define  LINK_INITIATION             0x00
#define  MODE_SELECT                 0x3
#define  GHS_INITIATION              0x01
#define  TIM_REF_CLK_OUT_FREE_FSC    0x01
#define  TIM_REF_CLK_OUT_SYMBOL_REF  0x03
#define  TIM_REF_CLK_OUT_SYM_8KHZ    0x00
#define  TIM_REF_CLK_IN_8KHZ         0x00
#define  TIM_DATA_CLK_8KHZ           0x10
#define  TIM_REF_CLK_IN_1536KHZ      0x01
#define  TIM_DATA_CLK_1536KHZ        0x11
#define  TIM_REF_CLK_IN_1544KHZ      0x02
#define  TIM_DATA_CLK_1544KHZ        0x12
#define  TIM_REF_CLK_IN_2048KHZ      0x03
#define  TIM_DATA_CLK_2048KHZ        0x13
#define  TIM_REF_CLK_IN_4096KHZ      0x04
#define  TIM_DATA_CLK_4096KHZ        0x14
#define  TIM_REF_CLK_IN_8192KHZ      0x05
#define  TIM_DATA_CLK_8192KHZ        0x15
#define  TIM_REF_CLK_IN_20480KHZ     0x07
#define  TIM_DATA_CLK_20480KHZ       0x18
#define  SDI_NODPLL                  0x00
#define  SDI_DPLL4IN                 0x01
#define  SDI_DPLL4INOUT              0x03
#define  SDI_NO                      0x00
#define  SDI_YES                     0x01
#define  SDI_NO_LOOP                 0x00
#define  SDI_REMOTE_LOOP             0x01
#define  SDI_REMOTE_CLK_ONLY         0x02
#define  SDI_INCLK                   0x0A

#define  SDI_INCLK_INSP_TDMMSP       0x05
#define  SDI_TDMSP_TDMMSP            0x08
#define  SDI_TDMCLK_TDMMSP           0x02
#define  SDI_TDMCLK_TDMSP_TDMMSP     0x03
#define  SDI_TDMCLK                  0x00
#define  SDI_DSL3                    0x09
#define  SDI_DSL3_NS                 0x0D
#define  PMMS_OFF                    0x00
#define  SYM_PSD                     0x00
#define  ANNEX_A_B                   0x03
#define  ANNEX_A                     0x01
#define  ANNEX_B                     0x02
#define  SHDSL_CLK_MODE_3a           0x4
#define  SHDSL_CLK_MODE_1            0x1
#define  SHDSL_CLK_MODE_2            0x2
#define  EPL_DISABLED                0x00
#define  EPL_ENABLED                 0x08
#define  PMMS_NORMAL                 0x00
#define  PBO_NORMAL                  0x00
#define  STARTUP_LOCAL               0x00
#define  STARTUP_FAREND              0x80
#define  GHS_TRNS_00                 0x00
#define  GHS_TRNS_01                 0x01
#define  GHS_TRNS_10                 0x02
#define  GHS_TRNS_11                 0x03
#define  SHDSL                       0x01
#define  REPEATER                    0x1
#define  TERMINATOR                  0x0
#define  STU_C                       0x1
#define  STU_R                       0x2
#define  FWCTRL_CHK                  0x00000100
#define  FWCTRL_SWITCH               0x00010000
#define  FWCTRL_PROTECT              0x01000000
#define  FWCTRL_VALID                0x00000001
#define  SINGLE                      0x00
#define  CLK_INTERNAL                0x00
#define  FRAME_PLESIO                0x0

// ERROR codes
#define EXMIT	1
#define	ERESET	2

struct sdfe4_ret{
	u8 stamp;
	u32 val;
};

struct sdfe4_msg{
	u16 ack_id;
	int len;
	u8 buf[SDFE4_FIFO8];
};

struct cmd_cfg_eoc_rx{
       u16 max_num_bytes;
};

#define TCPAM16_INT1_MIN 192
#define TCPAM16_INT1_MAX 2304
#define TCPAM16_INT2_MIN 2368
#define TCPAM16_INT2_MAX 3840
#define TCPAM32_INT1_MIN 768
#define TCPAM32_INT1_MAX 5696

struct cmd_cfg_caplist_short_ver_2{
	u8 clock_mode;
	u8 annex;
	u8 psd_mask;
	u8 pow_backoff;
	u16 base_rate_min;
	u16 base_rate_max;
	u16 base_rate_min16;
	u16 base_rate_max16;
	u16 base_rate_min32;
	u16 base_rate_max32;
	u8 sub_rate_min;
	u8 sub_rate_max;
	u8 enable_pmms;
	u8 pmms_margin;
	u8 rsvd0;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 octet_no_0;
	u8 octet_val_0;
	u8 octet_no_1;
	u8 octet_val_1;
	u8 octet_no_2;
	u8 octet_val_2;
	u8 octet_no_3;
	u8 octet_val_3;
	u8 octet_no_4;
	u8 octet_val_4;
	u8 octet_no_5;
	u8 octet_val_5;
	u8 octet_no_6;
	u8 octet_val_6;
	u8 octet_no_7;
	u8 octet_val_7;
	u8 octet_no_8;
	u8 octet_val_8;
	u8 octet_no_9;
	u8 octet_val_9;
	u8 octet_no_10;
	u8 octet_val_10;
	u8 octet_no_11;
	u8 octet_val_11;
	u8 octet_no_12;
	u8 octet_val_12;
	u8 octet_no_13;
	u8 octet_val_13;
	u8 octet_no_14;
	u8 octet_val_14;
	u8 octet_no_15;
	u8 octet_val_15;
};

struct cmd_cfg_sym_dsl_mode{
	u8 mode;
	u8 repeater;
	u8 standard;
	u8 rsvd0;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 rsvd4;
	u8 rsvd5;
	u8 rsvd6;
	u8 rsvd7;
};

struct cmd_cfg_ghs_mode{
	u8 transaction;
	u8 startup_initialization;
	u8 pbo_mode;
	u8 pmms_margin_mode;
	u8 epl_mode;
	u8 rsvd1;
	u8 rsvd2;
	u8 rsvd3;
	u8 rsvd4;
	u8 rsvd5;
	u8 rsvd6;
	u8 rsvd7;
};

struct cmd_cfg_sdi_settings{
	u8 input_mode;
	u8 output_mode;
	u16 frequency;
	u16 payload_bits;
	u8 frames;
	u8 loop;
	u8 ext_clk8k;
	u8 dpll4bclk;
	u8 refclkin_freq;
	u8 refclkout_freq;
};


struct cmd_cfg_sdi_tx{
	s32 data_shift;
	s8 frame_shift;
	u8 sp_level;
	u8 sp_sample_edg;
	u8 data_sample_edg;
	s32 lstwr_1strd_dly;
	u8 slip_mode;
	u8 rsvd1;
	u8 align;
	u8 rsvd3;
};

struct  cmd_cfg_sdi_rx{
	s32 data_shift;
	s8 frame_shift;
	u8 sp_level;
	u8 driving_edg;
	u8 data_shift_edg;
	s32 lstwr_1strd_dly;
	u8 slip_mode;
	u8 rsvd1;
	u8 align;
	u8 rsvd3;
};

struct ack_dsl_param_get {
	u8 stu_mode;
	u8 repeater;
	u8 annex;
	u8 clk_ref;
	u16 base_rate;
	u8 sub_rate;
	u8 psd_mask;
	u8 frame_mode;
	u8 rsvd2;
	u16 tx_sync_word;
	u16 rx_sync_word;
	u8 tx_stuff_bits;
	u8 rx_stuff_bits;
	s8 pow_backoff;
	s8 pow_backoff_farend;
	u8 ghs_pwr_lev_carr;
	u8 bits_p_symbol;
};

struct ack_perf_status_get {
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
};


u32 u8_to_u32(u8 *src);
int sdfe4_msg_init(struct sdfe4_msg *msg, char *cmsg, int len);
int sdfe4_chk_transplayer(u8 *msg);
int sdfe4_chk_msglayer(u8 *msg);
int sdfe4_reset_hwdev_chan(struct sdfe4_channel *ch);
int sdfe4_rs_cmd(u8 opcd, u32 *params, u16 plen,struct sdfe4_ret *ret,struct sdfe4 *hwdev);
int sdfe4_aux_cmd(u8 opcode, u8 param_1,struct sdfe4_ret *ret,struct sdfe4 *hwdev);
int sdfe4_pamdsl_cmd(u8 ch, u16 opcd, u8 *params, u16 plen,struct sdfe4_msg *rmsg,struct sdfe4 *hwdev);
int sdfe4_pamdsl_ack(u8 ch, u8 hdr0,struct sdfe4 *hwdev);
int sdfe4_pamdsl_nfc(struct sdfe4_msg *msg,struct sdfe4 *hwdev);
#define SDFE4_NOT_PAMDSL 0x1
#define SDFE4_PAMDSL_ACK 0x2
#define SDFE4_PAMDSL_NFC 0x3
#define SDFE4_PAMDSL_SYNC 0x4
#define SDFE4_PAMDSL_ERROR 0x5
int sdfe4_pamdsl_parse(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev);
int sdfe4_drv_poll(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev);
int sdfe4_download_fw(struct sdfe4 *hwdev
#ifdef SG17_PCI_MODULE
, u8 *fw, int fw_size
#endif
);
int sdfe4_setup_chan(u8 ch, struct sdfe4 *hwdev);
int sdfe4_setup_channel(int ch, struct sdfe4 *hwdev);
int sdfe4_start_channel(int ch, struct sdfe4 *hwdev);
int sdfe4_start_chip(struct sdfe4 *hwdev);
int sdfe4_state_mon(struct sdfe4 *hwdev);
int sdfe4_get_statistic(u8 ch, struct sdfe4 *hwdev,struct sdfe4_stat *stat);
int sdfe4_start_as_modem(struct sdfe4 *hwdev);

#ifdef SG17_REPEATER
int sdfe4_repeater_start( struct sdfe4 *hwdev );
#endif // SG17_REPEATER

// EOC related functions
void sdfe4_eoc_init(struct sdfe4_channel *ch,char *ptr,int size);
int sdfe4_eoc_rx(struct sdfe4_channel *ch,char **ptr);
int sdfe4_eoc_tx(struct sdfe4 *hwdev,int ch,char *ptr,int size);
#define SDFE4_EOC_MAXMSG 112


#endif
