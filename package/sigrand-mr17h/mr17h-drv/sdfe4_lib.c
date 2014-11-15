/* sdfe4_lib.c:
 *
 * SDFE4 Library
 *
 *      Infineon SDFE4 SHDSL chipset control library
 *	Provide architecture independent interface for
 *	Sigrand SG-17 devices
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */
	 

#include "include/sg17device.h"
#include "include/sdfe4_lib.h"
//#define DEBUG_ON
#define DEFAULT_LEV 20
#include "sg17debug.h"

inline u32
u8_to_u32(u8 *src)
{
        u32 ret32=0;
        u8 i,*ret8=(u8*)&ret32;
        for( i=0;i<4;i++)
                ret8[i]=src[i];
        return ret32;
}

/*
 * sdfe4_msg_init
 */
int
sdfe4_msg_init(struct sdfe4_msg *msg, char *cmsg, int len)
{
	int i;
	if( len > SDFE4_FIFO8 )
		return -1;
	for( i=0;i<len;i++)
		msg->buf[i] = cmsg[i];
	msg->len = len;
	return 0;
}

/*
 * sdfe4_chk_transplayer:
 * Checks transport layer status provided transport header
 * @msg		- message to check
*/
inline int
sdfe4_chk_transplayer(u8 *msg){
/* TODO: apply transport layer checking */
	return 0;
}

/*
 * sdfe4_chk_msglayer:
 * Checks message layer  status provided by RSTA register
 * @msg		- message to check
*/
inline int
sdfe4_chk_msglayer(u8 *msg){
	if( msg[0] != 0xA9 )
		return -1;
	return 0;
}

/*
 * sdfe4_reset_hwdev_chan:
 * Clear channel status when resetting channel
 * @ch		- channel handler
*/
int
sdfe4_reset_hwdev_chan(struct sdfe4_channel *ch)
{
	int ch_en_bkp=0;
	struct sg17_eoc *e = ch->eoc;	
	PDEBUG(debug_eoc,"EOC before reset: %p",ch->eoc);	
	if( ch->enabled )
		ch_en_bkp=1;
	memset(ch,0,sizeof(*ch));
	ch->enabled=ch_en_bkp;
	ch->eoc = e;
	PDEBUG(debug_eoc,"EOC after reset: %p",ch->eoc);
	return 0;
}

/*
 * sgfe4_xmit_rs_msg:
 * Sends message to RAM block of SDFE-4
 * @opcd	- operation code of RAM command
 * @params	- command parameters
 * @plen	- number of command parameters
 * @ret	- return value (if not NULL)
 * @data	- additional data for portability
*/
int
sdfe4_rs_cmd(u8 opcd, u32 *params, u16 plen,struct sdfe4_ret *ret,struct sdfe4 *hwdev)
{
	int i;
	u32 buf[SDFE4_FIFO32];
	u32 *msg32=(u32*)buf;
	u8  *msg8=(u8*)buf;
        int len=0;
	int r = 0;

	if ( plen > FW_PKT_SIZE32+1 ){
		return -1;
	}

	// prepare parameters of message
	// (send without transport protocol)
	msg8[0] = PEF24624_ADR_HOST ;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_RAMSHELL;
	msg8[2] = 0;
	msg8[3] = opcd;


 	for (i=0; i<plen; i++)
		msg32[i+1]=params[i];

	sdfe4_lock_chip(hwdev);
	//Send message to RAM
	if( (r = sdfe4_hdlc_xmit(msg8,RAM_CMDHDR_SZ+plen*4,hwdev)) ){
		PDEBUG(debug_sdfe4,"sdfe4_hdlc_xmit error = %d", r);	
		goto exit;
	}

	if( (r = sdfe4_hdlc_wait_intr(15000,hwdev)) ){
		PDEBUG(debug_sdfe4,"sdfe4_hdlc_wait_intr error = %d", r);
		goto exit;
	}
	if( (r = sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) ){
		PDEBUG(debug_sdfe4,"sdfe4_hdlc_recv error = %d", r);	
		goto exit;
	}
	msg8=(u8*)&buf;
        msg8+=RAM_ACKHDR_SZ;
	if( sdfe4_chk_msglayer(msg8) ){
		r = -1;
		goto exit;
	}
	ret->val=u8_to_u32(&msg8[1]);	
	
	if( ret->stamp ){
		
		if( (r=sdfe4_hdlc_wait_intr(15000,hwdev)) ){
			goto exit;
		}

	        if( (r=sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) ){
			goto exit;
        	}
		msg8=(u8*)&buf;
		msg8+=RAM_ACKHDR_SZ;
		if( (r=sdfe4_chk_msglayer(msg8)) ){
			goto exit;
		}
		ret->val=u8_to_u32(&msg8[1]);
	}
exit:
	sdfe4_unlock_chip(hwdev);	
	return r;
}


/*
 * sdfe4_aux_cmd:
 * Sends message to AUX block of SDFE-4
 * @opcd	- operation code of AUX command
 * @params	- command parameter
 * @data	- additional data for portability
*/
int
sdfe4_aux_cmd(u8 opcode, u8 param_1,struct sdfe4_ret *ret,struct sdfe4 *hwdev)
{
	u8 buf[SDFE4_FIFO8];
	u8 *msg8=(u8*)buf;
	int len=0;
	int e = 0;
	
	// prepare parameters of message
	// (send without transport protocol)
	msg8[0] = PEF24624_ADR_HOST;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_AUX;
	msg8[2] = opcode;
	msg8[3] = param_1;
	
	sdfe4_lock_chip(hwdev);

	if( (e=sdfe4_hdlc_xmit(msg8,AUX_CMDHDR_SZ+1,hwdev)) ){
		goto exit;
	}

	if( (e = sdfe4_hdlc_wait_intr(15000,hwdev)) )
		goto exit;

	if( (e=sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) )
		goto exit;

	msg8=(u8*)&buf[RAM_ACKHDR_SZ];
	if( (e=sdfe4_chk_msglayer(msg8)) )
		goto exit;
	
	if( ret )
		ret->val=u8_to_u32(&msg8[1]);

exit:
	sdfe4_unlock_chip(hwdev);
	return e;
}

/*
 * sdfe4_pamdsl_cmd:
 * Sends command to Embedded controller block of SDFE-4
 * @opcd	- operation code of RAM command
 * @ch		- controller number	
 * @params	- command parameters
 * @plen	- number of command parameters
 * @data	- additional data for portability
*/
int
sdfe4_pamdsl_cmd(u8 ch, u16 opcd, u8 *params, u16 plen,struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
	u8 buf[SDFE4_FIFO8];
	u8 *msg8=(u8*)buf;
	int i;
	int error = 0;

	rmsg->len = 0;

	// prepare parameters of message
	msg8[0] = PEF24624_ADR_HOST;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_PAMDSL(ch);
	// include transport protocol
	msg8[3] = msg8[6] = msg8[7] = 0;
	// add message id
	msg8[4] = opcd & 0xFF;
	msg8[5] = (opcd>>8) & 0xFF;

	// message params must be already in little endian, no conversion !!!
	for(i=0;i<plen;i++)
		msg8[i+8]=params[i];

	// clean channel
//	sdfe4_clear_channel(rmsg,hwdev);

	sdfe4_lock_chip(hwdev);
#ifdef SG17_REPEATER
	i=0;
	do{
#endif	
		msg8[2] = 0x08 | ( hwdev->msg_cntr & 0x1);
		if( (error=sdfe4_hdlc_xmit(msg8,EMB_CMDHDR_SZ+plen,hwdev)) ){
		// TODO: error handling
			PDEBUG(debug_error,"err(%d) in sdfe4_hdlc_xmit",error);
			error = -EXMIT;
			goto exit;
		}
		
#ifdef SG17_REPEATER
		if( i == 3 ){
			PDEBUG(debug_error,"error no answer");		
			error = -ERESET;
			goto exit;
		}
		i++;
	}
	while( sdfe4_drv_poll(rmsg,hwdev) );
#else	

	if( (error=sdfe4_hdlc_wait_intr(15000,hwdev)) ){
		PDEBUG(debug_error,"err(%d) no intr",error);	
		error = -1;
		goto exit;
	}
	
	i = sdfe4_hdlc_recv(rmsg->buf,&rmsg->len,hwdev);
#endif	
		
	if( rmsg->ack_id != *(u16*)(&rmsg->buf[4])){
		error = -1;
		goto exit;
	}

exit:		
	sdfe4_unlock_chip(hwdev);
	return error;
}

/*
 * sdfe4_pamdsl_ack:
 * Sends acknoledge to Embedded controller block of SDFE-4
 * @opcd	- operation code of RAM command
 * @ch	- controller number	
 * @hdr0	- first byte of transport layer header
 * @data	- additional data for portability
*/
int
sdfe4_pamdsl_ack(u8 ch, u8 hdr0,struct sdfe4 *hwdev)
{
	u8 msg[8];

	// prepare parameters of message
	msg[0] = PEF24624_ADR_HOST;
	msg[1] = PEF24624_ADR_DEV | (ch & 0xF);
	// include transport protocol
	msg[2] = hdr0;
	msg[3] = msg[6] = msg[7] = 0;
	// add message id
	msg[4] = 0;
	msg[5] = 0;

	// message params must be already in little endian, no conversion !!!
	if( sdfe4_hdlc_xmit(msg,EMB_ACKHDR_SZ,hwdev) ){
	// TODO: error handling
		return -1;
	}
	
	return 0;
}

/*
 * sdfe4_pamdsl_nfc:
 * Proceed notification from Embedded controller block of SDFE-4
 * @msg	- structure, that holds message
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 */
int
sdfe4_pamdsl_nfc(struct sdfe4_msg *msg,struct sdfe4 *hwdev)
{
	u8 hdr0,hdr1,ch;
	u16 ackID;	
	hdr0=msg->buf[2];
	hdr1=msg->buf[3];
	ch=PEF24624_PAMDSL_ADR(msg->buf[0]);
PDEBUG(debug_eoc,"ch=%d",ch);
	ackID=*(u16*)(&msg->buf[4]);
asm("#1");	
	switch( ackID ){
	case NFC_CONNECT_CTRL:
asm("#2");
		PDEBUG(debug_sdfe4,"NFC_CONNECT_CTRL, status = %02x",msg->buf[EMB_NFCHDR_SZ]);
		hwdev->ch[ch].state=msg->buf[EMB_NFCHDR_SZ];
		hwdev->ch[ch].state_change=1;
asm("#3");
		return 0;
	case NFC_CONNECT_CONDITION:
asm("#4");
		hwdev->ch[ch].conn_state=msg->buf[EMB_NFCHDR_SZ];
		hwdev->ch[ch].conn_state_change=1;
asm("#5");
		return 0;
	case NFC_SDI_DPLL_SYNC:
asm("#6");
		hwdev->ch[ch].sdi_dpll_sync=1;
		PDEBUG(0,"sdi_dpll_sync=1");
		return 0;
	case NFC_PERF_PRIM:
		hwdev->ch[ch].perf_prims=msg->buf[EMB_NFCHDR_SZ];
		return 0;
	case NFC_EOC_TX:
asm("#7!!");
		PDEBUG(debug_eoc,"EOC(%d): new state: (%p) msg->len=%d",ch,hwdev->ch[ch].eoc,msg->len);
		hwdev->ch[ch].eoc->eoc_tx=msg->buf[EMB_NFCHDR_SZ];

asm("#8!!");		
		return 0;
	case NFC_EOC_RX:
asm("#9!!");	
		PDEBUG(debug_eoc,"EOC(%d): Get new message",ch);
asm("#9.1!!");		
		sdfe4_eoc_init(&hwdev->ch[ch],&msg->buf[EMB_NFCHDR_SZ],msg->len-EMB_NFCHDR_SZ);
asm("#10!!");		
		return 0;
	case NFC_UNDEF_MSG_ID:
	case NFC_MULTIWIRE_MASTER:

	case NFC_MULTIWIRE_PAIR_NR:
	case NFC_MPAIR_DELAY_MEASURE_SDFE4:	
	case  NFC_FBIT_RX:
		break;
	}
	return -1;
}

/*
 * sdfe4_pamdsl_parse:
 * Parse message from SDFE-4 chipset
 * @rmsg 	- structure, that holds message
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return message type
 */
int
sdfe4_pamdsl_parse(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
	u8 hdr0,hdr1,chan;

	hdr0=rmsg->buf[2];
	hdr1=rmsg->buf[3];
	chan=rmsg->buf[0];
	
	if( (chan & 0x1) ){
		PDEBUG(debug_sdfe4,"Error channel - %02x",chan);
		return  SDFE4_NOT_PAMDSL;
	}

	switch ( hdr0 & 0xfe ){
	case 0x88:
	// Transport counter sync
		if( ( (hwdev->msg_cntr) & 0x1) != (hdr0 & 0x1) ){
    			hwdev->msg_cntr = (hdr0 & 0x1);
		}
		(hwdev->msg_cntr)++;
		return SDFE4_PAMDSL_SYNC;
	case 0x06:
	// Reset requested from the counter part.
		(hwdev->msg_cntr)=0;		
		return SDFE4_PAMDSL_SYNC;
	case 0x08:
	// This CMD, ACK or NFC
		sdfe4_pamdsl_ack(chan, (0x88 | ( hdr0 & 0x1)),hwdev);
		if( hdr1 ){
		 	hwdev->msg_cntr++;	
			sdfe4_pamdsl_nfc(rmsg,hwdev);
			return SDFE4_PAMDSL_NFC;
		}
		return SDFE4_PAMDSL_ACK;
	case 0x8E:
	//   Counter part received corrupted message.
		hwdev->msg_cntr++;
		return SDFE4_PAMDSL_SYNC;
	case 0x00:
	default:
	// TODO:  Error handling
		return SDFE4_PAMDSL_ERROR;
	}
}


/*
 * sdfe4_drv_poll:
 * Poll and proceed the messages from Embedded controller block of SDFE-4
 * @rmsg		- structure, that will hold received message
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 */
int
sdfe4_drv_poll(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
       //,ackID
	while(1){
		if( sdfe4_hdlc_wait_intr(150000,hwdev) )
			return -1;
		if( sdfe4_hdlc_recv(rmsg->buf,&rmsg->len,hwdev) ){
		// TODO: error handling
			return -1;
		}
		if( sdfe4_pamdsl_parse(rmsg,hwdev) > 0 )
			break;
	}
	return 0;	
}


/*
 * sdfe4_download_fw:
 * Downloads firmware to SDFE-4 chipset
 * @hwdew	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * @fw		- (only for PCI adapter) - pointer to firmware 
 * @fw_size	- (only for PCI adapter) - firmware size
 */
int
sdfe4_download_fw(struct sdfe4 *hwdev
#ifdef SG17_PCI_MODULE
 , u8 *fw, int fw_size
#endif
)
{
    	int i,k,iter;
  	u32 Data_U32[256];
  	struct sdfe4_ret ret;
#ifdef SG17_REPEATER	
	struct sdfe4_msg rmsg;
#endif	
	u8 CODE_CRC[4]={0xE0,0xF3,0x4E,0x7D};	
	u8 DATA_CRC[4]={0x8F,0xED,0xEF,0xFC};
	u8 *ret8;

	Data_U32[0]=0;
	ret.stamp=0;
	PDEBUG(debug_sdfe4,"hwdev = %08x",(u32)hwdev);
	PDEBUG(debug_sdfe4,"hwdev->data = %08x",(u32)hwdev->data);	
  	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWSTART,Data_U32,1,&ret,hwdev)){
		return -1;
    	}

	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		return -1;
    	}


 	iter=FW_CODE_SIZE/FW_PKT_SIZE;
	for(k=0;k<iter;k++){
		Data_U32[0]=(k*FW_PKT_SIZE)/4;
		for(i=0;i<FW_PKT_SIZE/4;i++){
#ifdef SG17_PCI_MODULE
			Data_U32[i+1] = cpu_to_be32(*((u32*)&fw[k*FW_PKT_SIZE + i*4]));
#else
			Data_U32[i+1]=cpu_to_be(FLASH_WordRead (FLASH_FW_STRTADDR +FW_CODE_OFFS +
								FW_PKT_SIZE*k + i*0x4));
#endif								
		}
		if( sdfe4_rs_cmd(CMD_WR_RAM_RS,(u32*)Data_U32,FW_PKT_SIZE32+1,&ret,hwdev)){
			PDEBUG(debug_sdfe4,"CMD_WR_RAM_RS for code error, iter = %d",k);
			return -1;
     		}
	}

	wait_ms(2);

	ret.stamp = 1;
	ret8 = (u8*)&ret.val;
	Data_U32[0]=FWCTRL_CHK;
        if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		wait_ms(2);
                Data_U32[0]=0;
		ret.stamp=0;
		if( sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,hwdev)){
			return -1;
		}
        }


	//  Count firmware code CRC 
	PDEBUGL(debug_sdfe4,"Code CRC: ");
	for(i=0;i<4;i++){
		PDEBUGL(debug_sdfe4,"%02x ",ret8[i]);
	}
	PDEBUGL(debug_sdfe4,"\n");
	

	for(i=0;i<4;i++){
		if(CODE_CRC[i]!=ret8[i]){
			return -1;
		}
	}

	// Load firmware data 
	wait_ms(100);
	ret.stamp=0;
	Data_U32[0]=FWCTRL_SWITCH;
	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev) )
		return -1;

	wait_ms(100);
	iter=FW_DATA_SIZE/FW_PKT_SIZE;
  	for(k=0;k<iter;k++){
		Data_U32[0]=(k*FW_PKT_SIZE)/4;
		for(i=0;i<FW_PKT_SIZE/4;i++){
#ifdef SG17_PCI_MODULE
			Data_U32[i+1] = cpu_to_be32(*((u32*)&fw[FW_DATA_OFFS+k*FW_PKT_SIZE + i*4]));
#else
	        	Data_U32[i+1] = cpu_to_be(FLASH_WordRead (FLASH_FW_STRTADDR + FW_DATA_OFFS +
                                			          FW_PKT_SIZE*k + i*0x4));
#endif								  
		}
		if( sdfe4_rs_cmd(CMD_WR_RAM_RS,(u32*)Data_U32,FW_PKT_SIZE32+1,&ret,hwdev)){
			PDEBUG(debug_sdfe4,"CMD_WR_RAM_RS for data error, iter = %d",k);
			return -1;
		}
	}

 	wait_ms(2);
	ret.stamp=1;
	Data_U32[0]=FWCTRL_CHK | FWCTRL_SWITCH;
	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
      		wait_ms(2);
                Data_U32[0]=0;
		ret.stamp=0;
		if( sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,hwdev)){
			return -1;
		}
	}



	// Count firmware data CRC DATA
	PDEBUGL(debug_sdfe4,"Data CRC: ");
	for(i=0;i<4;i++){
		PDEBUGL(debug_sdfe4,"%02x ",ret8[i]);
	}
	PDEBUGL(debug_sdfe4,"\n");
	

	for(i=0;i<4;i++){
		if(DATA_CRC[i]!=ret8[i]){
			return -1;
		}
	}

	wait_ms(100);
	ret.stamp=0;
	Data_U32[0]=FWdtpnt;
  	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWDTPNT,Data_U32,1,&ret,hwdev)){
		while(1);
    	}

	wait_ms(200);
	Data_U32[0]=FWCTRL_VALID;
  	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		return -1;
    	}

  	wait_ms(100);
	Data_U32[0]=0;
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled )
			Data_U32[0] |= (1<<i);
	}
  	if(sdfe4_rs_cmd(CMD_WR_REG_RS_FWSTART,Data_U32,1,&ret,hwdev)){
		return -1;
	}

	// INIT
#ifdef SG17_REPEATER	
	while( !sdfe4_drv_poll(&rmsg,hwdev) );
#endif	
	
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled &&
		   		hwdev->ch[i].state != MAIN_INIT ){
			return -1;
		}
		hwdev->ch[i].state_change=0;
	}
	return 0;
}

/*
 * sdfe4_setup_chan:
 * Setup SHDSL Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_setup_chan(u8 ch, struct sdfe4 *hwdev)
{
	struct sdfe4_if_cfg *cfg=&(hwdev->cfg[ch]);
   	u32 buf[SDFE4_FIFO32];
	struct cmd_cfg_sym_dsl_mode *sym_dsl;
	struct cmd_cfg_ghs_mode *ghs_mode;
	struct cmd_cfg_caplist_short_ver_2 *caplist;
	struct cmd_cfg_sdi_settings *sdi_settings;
	struct cmd_cfg_sdi_tx *sdi_tx;
	struct cmd_cfg_sdi_rx *sdi_rx;
	struct cmd_cfg_eoc_rx *eoc_rx;
	struct sdfe4_msg rmsg;
	struct sdfe4_ret ret;
	// 1. Setup if role
	PDEBUG(debug_sdfe4,"Setup if role");
	sym_dsl=(struct cmd_cfg_sym_dsl_mode *)buf;
	memset(sym_dsl,0,sizeof(*sym_dsl));
	sym_dsl->repeater= cfg->repeater ;
	sym_dsl->mode=cfg->mode;
	sym_dsl->standard=SHDSL;
	rmsg.ack_id=ACK_CFG_SYM_DSL_MODE;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SYM_DSL_MODE,(u8*)buf,sizeof(*sym_dsl),&rmsg,hwdev))
		return -1;
	
	
	//2. Setup transaction
	PDEBUG(debug_sdfe4,"Setup transaction");
	ghs_mode=(struct cmd_cfg_ghs_mode *)buf;
	memset(ghs_mode,0,sizeof(*ghs_mode));
	ghs_mode->transaction = cfg->transaction;
	ghs_mode->startup_initialization=cfg->startup_initialization;
	ghs_mode->pbo_mode=PBO_NORMAL;
	ghs_mode->pmms_margin_mode=PMMS_NORMAL;
	ghs_mode->epl_mode=EPL_ENABLED;
	rmsg.ack_id=ACK_CFG_GHS_MODE;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_GHS_MODE,(u8*)buf,sizeof(*ghs_mode),&rmsg,hwdev))
		return -1;
	
	// 3 Caplist_V2
	PDEBUG(debug_sdfe4,"Caplist_V2");
	caplist=(struct cmd_cfg_caplist_short_ver_2 *)buf;
	memset(caplist,0,sizeof(*caplist));
	caplist->clock_mode=SHDSL_CLK_MODE_1;
	caplist->annex = cfg->annex;
	caplist->psd_mask=0x00;
	caplist->pow_backoff=0x00;
	
	if(cfg->mode==STU_R){
	                caplist->base_rate_min=192;
			caplist->base_rate_max=2304;
			caplist->base_rate_min16=2368;
			caplist->base_rate_max16=3840;
			caplist->base_rate_min32=768;
			caplist->base_rate_max32=5696;
	
	}
	else {
	
		if( cfg->tc_pam == TCPAM32 ){
			caplist->base_rate_min=0;
			caplist->base_rate_max=0;
			caplist->base_rate_min16=0;
			caplist->base_rate_max16=0;
			caplist->base_rate_min32=TCPAM32_INT1_MIN;
			caplist->base_rate_max32=cfg->rate;
		}else if( cfg->tc_pam == TCPAM16 ){
			caplist->base_rate_min32=0;
			caplist->base_rate_max32=0;
			if( cfg->rate > TCPAM16_INT1_MAX ){
				caplist->base_rate_min=TCPAM16_INT1_MIN;
				caplist->base_rate_max=TCPAM16_INT1_MAX;
				caplist->base_rate_min16=TCPAM16_INT2_MIN;
				caplist->base_rate_max16=cfg->rate;
			}else{
				caplist->base_rate_min16=0;
				caplist->base_rate_max16=0;
				caplist->base_rate_min=TCPAM16_INT1_MIN;
				caplist->base_rate_max=cfg->rate;
			}			
		}	
	}
	
	caplist->sub_rate_min=0x00;
	caplist->sub_rate_max=0x00;
	caplist->enable_pmms=PMMS_OFF;
	caplist->pmms_margin=0x00;
	rmsg.ack_id=ACK_CFG_CAPLIST_SHORT_VER_2;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_CAPLIST_SHORT_VER_2,(u8*)buf,sizeof(*caplist),&rmsg,hwdev))
		return -1;
		

	// 5 confi AUX
	PDEBUG(debug_sdfe4,"config AUX_SDI_IF_SEL_3");
	wait_ms(20);
	if(sdfe4_aux_cmd(CMD_WR_REG_AUX_SDI_IF_SEL_3,0x03, &ret,hwdev))
          	return -1;
	PDEBUG(debug_sdfe4,"config AUX_SDI_IF_SEL_0");		
	wait_ms(20);
	if(sdfe4_aux_cmd(CMD_WR_REG_AUX_SDI_IF_SEL_0,0x00, &ret,hwdev))
          	return -1;
	
	PDEBUG(debug_sdfe4,"config AUX_AUX_IF_MODE");
	wait_ms(20);
	if(sdfe4_aux_cmd(CMD_WR_REG_AUX_AUX_IF_MODE,0x82, &ret,hwdev))
		return -1;

	
	
	//6 SDI settings
	PDEBUG(debug_sdfe4,"SDI settings");
	wait_ms(20);
	sdi_settings=(struct cmd_cfg_sdi_settings*)buf;
	memset(sdi_settings,0,sizeof(*sdi_settings));
        sdi_settings->input_mode=cfg->input_mode ;
	sdi_settings->output_mode=SDI_TDMSP_TDMMSP;
	sdi_settings->frequency=cfg->frequency;
	sdi_settings->payload_bits=cfg->payload_bits;
	sdi_settings->frames=0x30;
	sdi_settings->loop=cfg->loop;
	sdi_settings->ext_clk8k=SDI_NO;
	sdi_settings->dpll4bclk=SDI_NODPLL;
	sdi_settings->refclkin_freq=TIM_DATA_CLK_8KHZ;
	sdi_settings->refclkout_freq=TIM_REF_CLK_OUT_SYM_8KHZ;
	rmsg.ack_id=ACK_CFG_SDI_SETTINGS;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_SETTINGS,(u8*)buf,sizeof(*sdi_settings),&rmsg,hwdev))
		return -1;
		

	//7 Config SDI RX
	PDEBUG(debug_sdfe4,"Config SDI RX");
	sdi_rx=(struct cmd_cfg_sdi_rx*)buf;
	memset(sdi_rx,0,sizeof(*sdi_rx));
	sdi_rx->frame_shift=0x00;
	sdi_rx->sp_level=SDI_HIGH;
	sdi_rx->driving_edg=SDI_RISING;
	sdi_rx->data_shift_edg=SDI_NO;
	sdi_rx->lstwr_1strd_dly=0x93;
	sdi_rx->slip_mode=SLIP_FAST;
        sdi_rx->align=SDI_NO;
	rmsg.ack_id=ACK_CFG_SDI_RX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_RX,(u8*)buf,sizeof(*sdi_rx),&rmsg,hwdev))
		return -1;
	
	//8 Config SDI TX
	PDEBUG(debug_sdfe4,"Config SDI TX");
	sdi_tx=(struct cmd_cfg_sdi_tx*)buf;
	memset(sdi_tx,0,sizeof(*sdi_tx));
	sdi_tx->frame_shift=0x00;
	sdi_tx->sp_level=SDI_HIGH;
        sdi_tx->sp_sample_edg=SDI_FALLING;
	sdi_tx->data_sample_edg=SDI_FALLING;
	sdi_tx->lstwr_1strd_dly=0x93;
	sdi_tx->slip_mode=SLIP_FAST;
	sdi_tx->align=SDI_NO;
	rmsg.ack_id=ACK_CFG_SDI_TX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_TX,(u8*)buf,sizeof(*sdi_tx),&rmsg,hwdev))
		return -1;


	//9. config EOC
	PDEBUG(debug_sdfe4,"Config EOC");
	eoc_rx=(struct cmd_cfg_eoc_rx*)buf;
	memset(eoc_rx,0,sizeof(*eoc_rx));
	eoc_rx->max_num_bytes=800;
	rmsg.ack_id=ACK_CFG_EOC_RX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_EOC_RX,(u8*)buf,sizeof(*eoc_rx),&rmsg,hwdev))
		return -1;
	
	return 0;
}

/*
 * sdfe4_setup_channel:
 * Wrapper to sdfe4_setup_chan
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_setup_channel(int ch, struct sdfe4 *hwdev)
{
	sdfe4_clear_channel(hwdev);
	return sdfe4_setup_chan(ch,hwdev);
}

/*
 * sdfe4_start_channel:
 * Starup Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_start_channel(int ch,struct sdfe4 *hwdev)
{
     	struct sdfe4_msg rmsg;
	u8 main_pre_act[3]={MAIN_PRE_ACT,0x0,0x0};
	wait_ms(10);
	rmsg.ack_id=ACK_CONNECT_CTRL;
	if(sdfe4_pamdsl_cmd(ch,CMD_CONNECT_CTRL,main_pre_act,3,&rmsg,hwdev))
		return -1;
		
	if( rmsg.buf[EMB_ACKHDR_SZ] != MAIN_PRE_ACT )
		return -1;
	return 0;
}


/*
 * sdfe4_disable_channel:
 * Disables Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_disable_channel(int ch,struct sdfe4 *hwdev)
{
     	struct sdfe4_msg rmsg;
	u8 main_init[3]={MAIN_INIT,0x0,0x0};	
	wait_ms(50);
	rmsg.ack_id=ACK_CONNECT_CTRL;
	if(sdfe4_pamdsl_cmd(ch,CMD_CONNECT_CTRL,main_init,3,&rmsg,hwdev))
		return -1;

	return 0;
}

/*
 * sdfe4_start_as_modem:
 * Starts enabled Embedded controller blocks of SDFE-4 in modem mode
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_start_as_modem(struct sdfe4 *hwdev)
{
	int i;
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled ){
			if( sdfe4_start_channel(i,hwdev) )
				return -1;
#ifdef SG17_PCI_MODULE
			wait_ms(10);
#endif
		}
	}
	return 0;
}

/*
 * sdfe4_load_config:
 * Sync parameters of Embedded controller block and device handler (hwdev)
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_load_config(u8 ch, struct sdfe4 *hwdev)
{
  	struct sdfe4_msg rmsg;
	struct ack_dsl_param_get *dsl_par;
	struct sdfe4_if_cfg *ch_cfg=&(hwdev->cfg[ch]);
	int TC_PAM;
	int r;
	
	wait_ms(10);
        rmsg.ack_id=ACK_DSL_PARAM_GET;
	if( (r = sdfe4_pamdsl_cmd(ch,CMD_DSL_PARAM_GET,NULL,0,&rmsg,hwdev)) ){
		PDEBUG(debug_error,"error(%d) in CMD_DSL_PARAM_GET",r);
		return -1;
	}
	dsl_par=(struct ack_dsl_param_get *)&(rmsg.buf[8]);
	PDEBUG(debug_sdfe4,"Get return");	
	if(dsl_par->bits_p_symbol >= 0x04){
		TC_PAM =TCPAM32;
	}else{
		TC_PAM =TCPAM16;
	}
	ch_cfg->annex = dsl_par->annex;
	ch_cfg->tc_pam = TC_PAM;
	ch_cfg->rate = dsl_par->base_rate;
	PDEBUG(debug_sdfe4,"rate = %d",ch_cfg->rate);	
	return 0;
}

int
sdfe4_get_statistic(u8 ch, struct sdfe4 *hwdev,struct sdfe4_stat *stat)
{
  	struct sdfe4_msg rmsg;
	struct ack_perf_status_get *perf;
//	struct ack_dsl_param_get *dsl_par;		
//	struct sdfe4_if_cfg *ch_cfg=&(hwdev->cfg[ch]);
	int r;
	
	wait_ms(10);

        rmsg.ack_id=ACK_PERF_STATUS_GET;
	if( (r = sdfe4_pamdsl_cmd(ch,CMD_PERF_STATUS_GET,NULL,0,&rmsg,hwdev)) ){
		PDEBUG(debug_error,"error(%d) in CMD_DSL_PARAM_GET",r);
		return -1;
	}
	perf = (struct ack_perf_status_get*)&(rmsg.buf[8]);
	sdfe4_memcpy(stat,perf,sizeof(*perf));
	return 0;
}



/*
 * sdfe4_load_config:
 * Process notifications from Embedded controller block of SDFE4
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_state_mon(struct sdfe4 *hwdev)
{
	int i;
	struct sdfe4_channel *chan;
	struct sdfe4_if_cfg *cfg;
	
	PDEBUG(debug_sdfe4,"");
	for(i=0;i<4;i++){
		if( !hwdev->ch[i].enabled )
			continue;
		chan=&hwdev->ch[i];
		cfg=&hwdev->cfg[i];
		if( chan->state_change ){
			switch( chan->state ){
			case MAIN_CORE_ACT:
				sdfe4_link_led_blink(i,hwdev);
				break;
			case MAIN_DATA_MODE:
				sdfe4_link_led_fast_blink(i,hwdev);
				break;
			case MAIN_INIT:
				sdfe4_link_led_down(i,hwdev);
				sdfe4_reset_hwdev_chan(&(hwdev->ch[i]));
    				if( sdfe4_setup_channel(i,hwdev) )
					PDEBUG(debug_sdfe4,"error in sdfe4_setup_channel");
				if( sdfe4_start_channel(i,hwdev) )
					PDEBUG(debug_sdfe4,"error in sdfe4_start_channel");
				wait_ms(10);
				return -1;
			case MAIN_EXCEPTION:
			  	break;
			}
			chan->state_change=0;
		}
			
		if( chan->conn_state_change ){
			switch( chan->conn_state ){
			case GHS_STARTUP:
				break;
			case GHS_TRANSFER:
			        break;
			case EXCEPTION:
				break;
			case GHS_30SEC_TIMEOUT:
				// SRU specific
			  	break;
			}
			chan->conn_state_change=0;
		}

		if(chan->sdi_dpll_sync){
			sdfe4_load_config(i,hwdev);
			sdfe4_link_led_up(i,hwdev);
			chan->sdi_dpll_sync = 0;
		}

		if(cfg->need_reconf){
			sdfe4_disable_channel(i,hwdev);
			sdfe4_link_led_down(i,hwdev);
			sdfe4_reset_hwdev_chan(&(hwdev->ch[i]));			
			wait_ms(200);
			if( sdfe4_setup_channel(i,hwdev) )
				PDEBUG(debug_sdfe4,"error in sdfe4_setup_channel");
			if( sdfe4_start_channel(i,hwdev) )
				PDEBUG(debug_sdfe4,"error in sdfe4_start_channel");
			cfg->need_reconf = 0;
		}
	}
	return 0;
}

#ifdef SG17_REPEATER

/*
 * sdfe4_repeater_start:
 * Starts Embedded controller block of SDFE-4 in repeater mode
 * @hwdev - structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status 
 */

int
sdfe4_repeater_start( struct sdfe4 *hwdev )
{
  	struct sdfe4_msg rmsg;
	struct ack_dsl_param_get *dsl_par;
	struct sdfe4_if_cfg *cfg_ch0=&(hwdev->cfg[0]);
	
  	int TC_PAM;

	if( sdfe4_setup_channel(3,hwdev) )
		return -1;

	if( sdfe4_start_channel(3,hwdev) )
		return -1;
	
	while(1){
		while(sdfe4_drv_poll(&rmsg,hwdev));
		if( state_monitoring(hwdev) )
			return -1;
		if(hwdev->ch[3].sdi_dpll_sync){
		    	break;
		}
	}
	
        rmsg.ack_id=ACK_DSL_PARAM_GET;
	
	if( sdfe4_pamdsl_cmd(3,CMD_DSL_PARAM_GET,NULL,NULL,&rmsg,hwdev) ){
		return -1;
	}
	dsl_par=(struct ack_dsl_param_get *)&(rmsg.buf[8]);
	
	/*детект TCPAM*/
	
	if(dsl_par->bits_p_symbol >= 0x04){
		TC_PAM =TCPAM32;
	}else{
		TC_PAM =TCPAM16;
	}
	
	
	  /// Обработка принятого сообщения !
	// config SDFE chenal 0.
	// STU_C
	cfg_ch0->mode=                    STU_C;
	//REPEATER
	cfg_ch0->repeater=                TERMINATOR;
	// STARTUP_FAREND  STARTUP_LOCAL
	cfg_ch0->startup_initialization= STARTUP_LOCAL;
	//GHS_TRNS_00 :GHS_TRNS_01:GHS_TRNS_11:GHS_TRNS_10
	cfg_ch0->transaction=            GHS_TRNS_10;
	/// ANNEX_A_B   ANNEX_A   ANNEX_B ANNEX_G ANNEX_F
	cfg_ch0->annex=                  dsl_par->annex;
	///  TC-PAM: TCPAM16  TCPAM32
	cfg_ch0->tc_pam=                TC_PAM;
	// rate (speed)
	cfg_ch0->rate=                  dsl_par->base_rate;

	
	// SDI_TDMCLK_TDMMSP  SDI_DSL3
	cfg_ch0->input_mode=             SDI_TDMCLK_TDMMSP;
	// Terminal =8192 , Repeater= 12288;
	cfg_ch0->frequency=              8192;
	// Terminal =5696 , Repeater= 2048;
	cfg_ch0->payload_bits=           5696;
	// петля вход-> выход =   SDI_NO_LOOP SDI_REMOTE_LOOP
	cfg_ch0->loop=                   SDI_NO_LOOP;
	
	if( sdfe4_setup_channel(0,hwdev) )
		return -1;
	
	if( sdfe4_start_channel(0,hwdev) )
		return -1;
	
	while(1){
		while(sdfe4_drv_poll(&rmsg,hwdev));
		if( state_monitoring(hwdev) )
			return -1;
		if(hwdev->ch[0].sdi_dpll_sync){
		    	break;
		}
	}
	return 0;
}
#endif

/*----------------------------------------------------------
    EOC Functions
 -----------------------------------------------------------*/

/*
 * sdfe4_eoc_init:
 * 	initialise new arrived EOC message and save it to 
 *	channels (ch) internal buffer for future process
 *	by OS applications
 *	Parameters:
 *	@ch - channel reseived message
 *	@ptr - pointer to message
 *	@size - message size
 */
void sdfe4_eoc_init(struct sdfe4_channel *ch,char *ptr,int size)
{
        int eflag;
	int msize;
PDEBUG(debug_eoc,"ptr=%p, size=%d,msg_size=%d",ptr,size,ptr[2]);

	// check if size is valid    
        if( size < 4 ){
		PDEBUG(debug_eoc,"Abort: small size");
	        return;
	}
/*	
//----------DEBUG-----------------//
int i;
for(i=0;i<ptr[2];i++){
    printk("%02x ",ptr[i+3]);
}
printk(debug_eoc,"\n");
//----------DEBUG-----------------//
*/

	if( ptr[1] ){
	// receiption failed
		eoc_abort_new(ch->eoc);
		PDEBUG(debug_eoc,"Abort: failed receiption");		
		return;
	}
	    
	eflag = ptr[0]; // message completion flag
	msize = ptr[2]; // message length
	PDEBUG(debug_eoc,"eflag=%d,msize=%d",eflag,msize);	
/*	if( msize != (size-3) ){
	// bad block size
		PDEBUG(debug_eoc,"Abort: sizes mismatch");	
		eoc_abort_cur(ch->eoc);
		return;
	}
*/	PDEBUG(debug_eoc,"process appending");	
	eoc_append(ch->eoc,&ptr[3],msize,eflag);
}		

/*
 * sdfe4_eoc_init:
 * 	initialise new arrived EOC message and save it to 
 *	channels (ch) internal buffer for future process
 *	by OS applications
 *	Parameters:
 *	@ch - channel reseived message
 *	@ptr - pointer to message
 *	@size - message size
 */
inline int sdfe4_eoc_rx(struct sdfe4_channel *ch,char **ptr){
	return eoc_get_cur(ch->eoc,ptr);
}

int
sdfe4_eoc_tx(struct sdfe4 *hwdev,int ch,char *ptr,int size)
{
	int count = size/SDFE4_EOC_MAXMSG + 1;
	char msg[SDFE4_EOC_MAXMSG+2];
	struct sdfe4_msg rmsg;	
	int i,offset = 0,j;
	u8 cp;
	PDEBUG(debug_eoc,"Start.size=%d, %d interations",size,count);
for(i=0;i<size;i++){
	printk("%02x ",ptr[i]);
}
printk("\n");
	for(i=0;i<count;i++){
		cp = (size>SDFE4_EOC_MAXMSG) ? SDFE4_EOC_MAXMSG : size;
		memcpy(msg+2,ptr+offset,cp);
		offset += cp;
		size -= cp;
		rmsg.ack_id=ACK_EOC_TX;
		PDEBUG(debug_eoc,"Call CMD_EOC_TX, send %d bytes",cp);
		hwdev->ch[ch].eoc->eoc_tx = 0;
		if( i+1 == count ){
			msg[0] = 1;
		}else{
			msg[0] = 0;
		}
		msg[1] = cp;
/*
printk(KERN_NOTICE"MSG#%d: ");
for(j=0;j<cp+2;j++){
	if( (j+1) % 25 == 0)
		printk("\n");
	printk("%02x ",msg[j]);
}
printk("\n");
*/		
		hwdev->ch[ch].eoc->eoc_tx = 0;
		if(sdfe4_pamdsl_cmd(ch,CMD_EOC_TX,(u8*)msg,cp+2,&rmsg,hwdev)){
			PDEBUG(debug_eoc,"CMD_EOC_TX failed");		
			return -1;
		}
		PDEBUG(debug_eoc,"CMD_EOC_TX: success");
		PDEBUG(debug_eoc,"wait for NFC");
		sdfe4_eoc_wait_intr(1500,hwdev);
		if( i+1==count ){
			if( hwdev->ch[ch].eoc->eoc_tx != EOC_TX_READY ){
				PDEBUG(debug_eoc,"Fail to trasnmit, no EOC_TX_READY");
			        return -1;
			}
			PDEBUG(debug_eoc,"TX Successful");			
		}else{
			if( hwdev->ch[ch].eoc->eoc_tx != EOC_TX_FRAME ){
				PDEBUG(debug_eoc,"Fail to trasnmit, no EOC_TX_READY");
			        return -1;
			}
		}

		PDEBUG(debug_eoc,"Interation %d success",i);
	}
	PDEBUG(debug_eoc,"TX success");	
	return 0;
}

