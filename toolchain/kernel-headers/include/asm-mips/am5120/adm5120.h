/*****************************************************************************
;
;   (C) Unpublished Work of ADMtek Incorporated.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF ADMTEK INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) ADMTEK EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN ADMTEK WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF ADMTEK.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : ADM5120
;    Creator : 
;    File    : include/asm/am5120/adm5120.h
;	 Date    : 2003.3.10
;    Abstract: 
;
;Modification History:
; 
;
;*****************************************************************************/


#ifndef  __ADM5120_H__
#define  __ADM5120_H__


#include <asm/addrspace.h>


/*=========================  Physical Memory Map  ============================*/
#define SDRAM_BASE				0
#define SMEM1_BASE				0x10000000

#define EXTIO0_BASE				0x10C00000
#define EXTIO1_BASE				0x10E00000
#define MPMC_BASE				0x11000000
#define USBHOST_BASE				0x11200000
#define PCIMEM_BASE				0x11400000
#define PCIIO_BASE				0x11500000
#define PCICFG_BASE				0x115FFFF0
#define MIPS_BASE				0x11A00000
#define SWCTRL_BASE				0x12000000

#define INTC_BASE				0x12200000
#define SYSC_BASE				0x12400000

#define UART0_BASE				0x12600000
#define UART1_BASE				0x12800000

#define SMEM0_BASE				0x1FC00000


/*=======================  MIPS interrupt  ===================*/
#define MIPSINT_SOFT0				0
#define MIPSINT_SOFT1				1
#define MIPSINT_IRQ				2
#define MIPSINT_FIQ				3
#define MIPSINT_REV0				4
#define MIPSINT_REV1				5
#define MIPSINT_REV2				6
#define MIPSINT_TIMER				7



/*====================  MultiPort Memory Controller (MPMC) ==================*/
/* registers offset */
#define MPMC_CONTROL_REG			0x0000
#define MPMC_STATUS_REG				0x0004
#define MPMC_CONFIG_REG				0x0008

#define MPMC_DM_CONTROL_REG			0x0020
#define MPMC_DM_REFRESH_REG			0x0024

#define MPMC_DM_TRP_REG				0x0030
#define MPMC_DM_TRAS_REG			0x0034
#define MPMC_DM_TSREX_REG			0x0038
#define MPMC_DM_TAPR_REG			0x003C
#define MPMC_DM_TDAL_REG			0x0040
#define MPMC_DM_TWR_REG				0x0044
#define MPMC_DM_TRC_REG				0x0048
#define MPMC_DM_TRFC_REG			0x004C
#define MPMC_DM_TXSR_REG			0x0050
#define MPMC_DM_TRRD_REG			0x0054
#define MPMC_DM_TMRD_REG			0x0058

#define MPMC_SM_EXTWAIT_REG			0x0080

#define MPMC_DM_CONFIG0_REG			0x0100
#define MPMC_DM_RASCAS0_REG			0x0104

#define MPMC_DM_CONFIG1_REG			0x0120
#define MPMC_DM_RASCAS1_REG			0x0124

#define MPMC_SM_CONFIG0_REG			0x0200
#define MPMC_SM_WAITWEN0_REG			0x0204
#define MPMC_SM_WAITOEN0_REG			0x0208
#define MPMC_SM_WAITRD0_REG			0x020C
#define MPMC_SM_WAITPAGE0_REG			0x0210
#define MPMC_SM_WAITWR0_REG			0x0214
#define MPMC_SM_WAITTURN0_REG			0x0218

#define MPMC_SM_CONFIG1_REG			0x0220
#define MPMC_SM_WAITWEN1_REG			0x0224
#define MPMC_SM_WAITOEN1_REG			0x0228
#define MPMC_SM_WAITRD1_REG			0x022C
#define MPMC_SM_WAITPAGE1_REG			0x0230
#define MPMC_SM_WAITWR1_REG			0x0234
#define MPMC_SM_WAITTURN1_REG			0x0238

#define MPMC_SM_CONFIG2_REG			0x0240
#define MPMC_SM_WAITWEN2_REG			0x0244
#define MPMC_SM_WAITOEN2_REG			0x0248
#define MPMC_SM_WAITRD2_REG			0x024C
#define MPMC_SM_WAITPAGE2_REG			0x0250
#define MPMC_SM_WAITWR2_REG			0x0254
#define MPMC_SM_WAITTURN2_REG			0x0258

#define MPMC_SM_CONFIG3_REG			0x0260
#define MPMC_SM_WAITWEN3_REG			0x0264
#define MPMC_SM_WAITOEN3_REG			0x0268
#define MPMC_SM_WAITRD3_REG			0x026C
#define MPMC_SM_WAITPAGE3_REG			0x0270
#define MPMC_SM_WAITWR3_REG			0x0274
#define MPMC_SM_WAITTURN3_REG			0x0278

/* Macro for access MPMC register */
#define MPMC_REG(_offset)				\
		(*((volatile unsigned long *)(KSEG1ADDR(MPMC_BASE + (_offset)))))


/* MPMC_CONTROL_REG (offset: 0x0000) */
#define MPMC_DRAIN_W_BUF			0x00000008
#define MPMC_LOW_POWER_MODE			0x00000004
#define MPMC_ADDR_MIRROR			0x00000002
#define MPMC_ENABLE				0x00000001
#define MPMC_CONTROL_MASK			0x0000000f

/* MPMC_STATUS_REG (offset: 0x0004) */
#define MPMC_SREFACK				0x00000004
#define MPMC_WBUF_DIRTY				0x00000002
#define MPMC_BUSY				0x00000001
#define MPMC_STATUS_MASK			0x00000007

/* MPMC_CONFIG_REG (offset: 0x0008) */
#define MPMC_CLK_RATIO_1_1			0x00000000
#define MPMC_CLK_RATIO_1_2			0x00000100
#define MPMC_LITTLE_ENDIAN			0x00000000
#define MPMC_BIG_ENDIAN				0x00000001
#define MPMC_CONFIG_MASK			0x00000101

/* MPMC_DM_CONTROL_REG (offset: 0x0020) */
#define DM_PVHHOUT_HI_VOLTAGE			0x00008000
#define DM_RPOUT_HI_VOLTAGE			0x00004000
#define DM_DEEP_SLEEP_MODE			0x00002000

#define DM_SDRAM_NOP				0x00000180
#define DM_SDRAM_PRECHARGE_ALL			0x00000100
#define DM_SDRAM_MODE_SETTING			0x00000080
#define DM_SDRAM_NORMAL_OP			0x00000000
#define DM_SDRAM_OPMODE_MASK			0x00000180

#define DM_SELF_REFRESH_MODE			0x00000004
#define DM_CLKOUT_ALWAYS			0x00000002
#define DM_CLKEN_ALWAYS				0x00000001

#define MPMC_DM_CONTROL_MASK			0x0000e187


/* MPMC_DM_REFRESH_REG (offset:0x0024) */
#define MPMC_DM_REFRESH_MASK			0x00000300

/* MPMC_DM_TRP_REG (offset: 0x0030) */
#define MPMC_DM_TRP_MASK			0x0000000f

/* MPMC_DM_TRAS_REG (offset: 0x0034) */
#define MPMC_DM_TRAS_MASK			0x0000000f

/* MPMC_DM_TSREX_REG (offset: 0x0038) */
#define MPMC_DM_TSREX_MASK			0x0000000f

/* MPMC_DM_TAPR_REG	(offset: 0x003C) */
#define MPMC_DM_TAPR_MASK			0x0000000f

/* MPMC_DM_TDAL_REG	(offset: 0x0040) */
#define MPMC_DM_TDAL_MASK			0x0000000f

/* MPMC_DM_TWR_REG (offset: 0x0044) */
#define MPMC_DM_TWR_MASK			0x0000000f

/* MPMC_DM_TRC_REG (offset: 0x0048) */
#define MPMC_DM_TRC_MASK 			0x0000001f

/* MPMC_DM_TRFC_REG (offset: 0x004C) */
#define MPMC_DM_TRFC_MASK			0x0000001f

/* MPMC_DM_TXSR_REG	(offset: 0x0050) */
#define MPMC_DM_TXSR_MASK			0x0000001f

/* MPMC_DM_TRRD_REG	(offset: 0x0054) */
#define MPMC_DM_TRRD_MASK			0x0000000f

/* MPMC_DM_TMRD_REG	(offset: 0x0058) */
#define MPMC_DM_TMRD_MASK			0x0000000f

/* MPMC_SM_EXTWAIT_REG (offset:	0x0080) */
#define MPMC_SM_EXTWAIT_MASK			0x0000003f


/* MPMC_DM_CONFIG0_REG (offset: 0x0100) */
/* MPMC_DM_CONFIG1_REG (offset: 0x0120) */
#define DM_CFG_ROW_WIDTH_13BIT			0x20000000
#define DM_CFG_ROW_WIDTH_12BIT			0x10000000
#define DM_CFG_ROW_WIDTH_11BIT			0x00000000
#define DM_CFG_ROW_WIDTH_MASK			0x30000000
#define DM_CFG_ROW_WIDTH_SHIFT			28

#define DM_CFG_2BANK_DEV			0x00000000
#define DM_CFG_4BANK_DEV			0x04000000
#define DM_CFG_BANK_SHIFT			26

#define DM_CFG_COL_WIDTH_11BIT			0x01400000
#define DM_CFG_COL_WIDTH_10BIT			0x01000000
#define DM_CFG_COL_WIDTH_9BIT			0x00c00000
#define DM_CFG_COL_WIDTH_8BIT			0x00800000
#define DM_CFG_COL_WIDTH_7BIT			0x00400000
#define DM_CFG_COL_WIDTH_6BIT			0x00000000
#define DM_CFG_COL_WIDTH_MASK			0x01c00000
#define DM_CFG_COL_WIDTH_SHIFT			22

#define DM_CFG_WRITE_PROTECT			0x00100000
#define DM_CFG_BUFFER_EN			0x00080000

#define DM_CFG_ADDR_MAPPING_MASK		0x00005F80

#define DM_CFG_DEV_SYNC_FLASH			0x00000010
#define DM_CFG_DEV_LOWPOWER_SDRAM		0x00000008
#define DM_CFG_DEV_SDRAM			0x00000000
#define DM_CFG_DEV_MASK				0x00000018


/* MPMC_DM_RASCAS0_REG (offset: 0x0104) */
/* MPMC_DM_RASCAS1_REG (offset: 0x0124) */

#define DM_CAS_LATENCY_3			0x00000300
#define DM_CAS_LATENCY_2			0x00000200
#define DM_CAS_LATENCY_1			0x00000100

#define DM_RAS_LATENCY_3			0x00000003
#define DM_RAS_LATENCY_2			0x00000002
#define DM_RAS_LATENCY_1			0x00000001


/* MPMC_SM_CONFIG0_REG (offset: 0x0200) */
/* MPMC_SM_CONFIG1_REG (offset: 0x0220) */
/* MPMC_SM_CONFIG2_REG (offset: 0x0240) */
/* MPMC_SM_CONFIG3_REG (offset: 0x0260) */

#define SM_WRITE_PROTECT			0x00100000
#define SM_WRITEBUF_ENABLE			0x00080000
#define SM_EXTENDED_WAIT			0x00000100
#define SM_PB					0x00000080
#define SM_CS_HIGH				0x00000040
#define SM_PAGE_MODE				0x00000008

#define SM_MEM_WIDTH_32BIT			0x00000002
#define SM_MEM_WIDTH_16BIT			0x00000001
#define SM_MEM_WIDTH_8BIT			0x00000000

#define MPMC_SM_CONFIG_MASK			0x001801cb


/* MPMC_SM_WAITWEN0_REG	(offset: 0x0204) */
/* MPMC_SM_WAITWEN1_REG	(offset: 0x0224) */
/* MPMC_SM_WAITWEN2_REG	(offset: 0x0244) */
/* MPMC_SM_WAITWEN3_REG	(offset: 0x0264) */
#define MPMC_SM_WAITWEN_MASK			0x0000000f


/* MPMC_SM_WAITOEN0_REG (offset: 0x0208) */
/* MPMC_SM_WAITOEN1_REG (offset: 0x0228) */
/* MPMC_SM_WAITOEN2_REG (offset: 0x0248) */
/* MPMC_SM_WAITOEN3_REG (offset: 0x0268) */
#define MPMC_SM_WAITOEN_MASK			0x0000000f

/* MPMC_SM_WAITRD0_REG (offset: 0x020C) */
/* MPMC_SM_WAITRD1_REG (offset: 0x022C) */
/* MPMC_SM_WAITRD2_REG (offset: 0x024C) */
/* MPMC_SM_WAITRD3_REG (offset: 0x026C) */
#define MPMC_SM_WAITRD_MASK			0x0000001f

/* MPMC_SM_WAITPAGE0_REG (offset: 0x0210) */
/* MPMC_SM_WAITPAGE1_REG (offset: 0x0230) */
/* MPMC_SM_WAITPAGE2_REG (offset: 0x0250) */
/* MPMC_SM_WAITPAGE3_REG (offset: 0x0270) */
#define MPMC_SM_WAITPAGE_MASK			0x0000001f


/* MPMC_SM_WAITWR0_REG (offset: 0x0214) */
/* MPMC_SM_WAITWR1_REG (offset: 0x0234) */
/* MPMC_SM_WAITWR2_REG (offset: 0x0254) */
/* MPMC_SM_WAITWR3_REG (offset: 0x0274) */
#define MPMC_SM_WAITWR_MASK			0x0000001f


/* MPMC_SM_WAITTURN0_REG (offset: 0x0218) */
/* MPMC_SM_WAITTURN1_REG (offset: 0x0238) */
/* MPMC_SM_WAITTURN2_REG (offset: 0x0258) */
/* MPMC_SM_WAITTURN3_REG (offset: 0x0278) */
#define MPMC_SM_WAITTURN_MASK			0x0000000f


/* SDRAM mode register */
/* ref: SDRAM data sheet. Ex: Micron MT48LC4M16A2 data sheet. */
#define SDRAM_BTLEN_1				0x0000
#define SDRAM_BTLEN_2				0x0001
#define SDRAM_BTLEN_4				0x0002
#define SDRAM_BTLEN_8				0x0003
#define SDRAM_BTLEN_FULLPAGE			0x0007
#define SDRAM_BTLEN_MASK			0x0007

#define SDRAM_BT_SEQUENCIAL			0x0000
#define SDRAM_BT_INTERLEVED			0x0008

#define SDRAM_CAS_LATENCY_2			0x0020
#define SDRAM_CAS_LATENCY_3			0x0030
#define SDRAM_CAS_LATENCY_MASK			0x0030

#define SDRAM_OPMODE_STANDARD			0x0000
#define SDRAM_OPMODE_MASK			0x0180

#define SDRAM_WBTMODE_ENABLE			0x0000
#define SDRAM_WBTMODE_DISABLE			0x0200

#define SDRAM_MODEREG_MASK			0x03FF



/*==========================  Interrupt Controller  ==========================*/
/* registers offset */
#define IRQ_STATUS_REG				0x00	/* Read */
#define IRQ_RAW_STATUS_REG			0x04	/* Read */
#define IRQ_ENABLE_REG				0x08	/* Read/Write */
#define IRQ_DISABLE_REG				0x0C	/* Write */
#define IRQ_SOFT_REG				0x10	/* Write */

#define IRQ_MODE_REG				0x14	/* Read/Write */
#define FIQ_STATUS_REG				0x18	/* Read */

/* test registers */
#define IRQ_TESTSRC_REG				0x1c	/* Read/Write */
#define IRQ_SRCSEL_REG				0x20	/* Read/Write */
#define IRQ_LEVEL_REG				0x24	/* Read/Write */

/*  Macro for accessing Interrupt controller register  */
#define ADM5120_INTC_REG(_reg)		\
	(*((volatile unsigned long *)(KSEG1ADDR(INTC_BASE + (_reg)))))

/* interrupt levels */
#define INT_LVL_TIMER				0	/* Timer */
#define INT_LVL_UART0				1	/* Uart 0 */
#define INT_LVL_UART1				2	/* Uart 1 */
#define INT_LVL_USBHOST				3	/* USB Host */
#define INT_LVL_EXTIO_0				4	/* External I/O 0 */
#define INT_LVL_EXTIO_1				5	/* External I/O 1 */
#define INT_LVL_PCI_0				6	/* PCI 0 */
#define INT_LVL_PCI_1				7	/* PCI 1 */
#define INT_LVL_PCI_2				8	/* PCI 2 */
#define INT_LVL_SWITCH				9	/* Switch */
#define INT_LVL_MAX				INT_LVL_SWITCH	

/* interrupts */
#define IRQ_TIMER				(0x1 << INT_LVL_TIMER)
#define IRQ_UART0				(0x1 << INT_LVL_UART0)
#define IRQ_UART1				(0x1 << INT_LVL_UART1)
#define IRQ_USBHOST				(0x1 << INT_LVL_USBHOST)
#define IRQ_EXTIO_0				(0x1 << INT_LVL_EXTIO_0)
#define IRQ_EXTIO_1				(0x1 << INT_LVL_EXTIO_1)
#define IRQ_PCI_INT0				(0x1 << INT_LVL_PCI_0)
#define IRQ_PCI_INT1				(0x1 << INT_LVL_PCI_1)
#define IRQ_PCI_INT2				(0x1 << INT_LVL_PCI_2)
#define IRQ_SWITCH				(0x1 << INT_LVL_SWITCH)

#define IRQ_MASK				0x3ff


/* IRQ LEVEL reg */
#define IRQ_EXTIO0_ACT_LOW			IRQ_EXTIO_0
#define IRQ_EXTIO1_ACT_LOW			IRQ_EXTIO_1
#define IRQ_PCIINT0_ACT_LOW			IRQ_PCI_INT0
#define IRQ_PCIINT1_ACT_LOW			IRQ_PCI_INT1
#define IRQ_PCIINT2_ACT_LOW			IRQ_PCI_INT2

#define IRQ_LEVEL_MASK				0x01F0

/*=========================  Switch Control Register  ========================*/
/* Control Register */
#define CODE_REG				0x0000
#define SftRest_REG				0x0004
#define Boot_done_REG				0x0008
#define SWReset_REG				0x000C
#define Global_St_REG				0x0010
#define PHY_st_REG				0x0014
#define Port_st_REG				0x0018
#define Mem_control_REG				0x001C
#define SW_conf_REG				0x0020
#define CPUp_conf_REG				0x0024
#define Port_conf0_REG				0x0028
#define Port_conf1_REG				0x002C
#define Port_conf2_REG				0x0030

#define VLAN_G1_REG				0x0040
#define VLAN_G2_REG				0x0044
#define Send_trig_REG				0x0048
#define Srch_cmd_REG				0x004C
#define ADDR_st0_REG				0x0050
#define ADDR_st1_REG				0x0054
#define MAC_wt0_REG				0x0058
#define MAC_wt1_REG				0x005C
#define BW_cntl0_REG				0x0060
#define BW_cntl1_REG				0x0064
#define PHY_cntl0_REG				0x0068
#define PHY_cntl1_REG				0x006C
#define FC_th_REG				0x0070
#define Adj_port_th_REG				0x0074
#define Port_th_REG				0x0078
#define PHY_cntl2_REG				0x007C
#define PHY_cntl3_REG				0x0080
#define Pri_cntl_REG				0x0084
#define VLAN_pri_REG				0x0088
#define TOS_en_REG				0x008C
#define TOS_map0_REG				0x0090
#define TOS_map1_REG				0x0094
#define Custom_pri1_REG				0x0098
#define Custom_pri2_REG				0x009C

#define Empty_cnt_REG				0x00A4
#define Port_cnt_sel_REG			0x00A8
#define Port_cnt_REG				0x00AC
#define SW_Int_st_REG				0x00B0
#define SW_Int_mask_REG				0x00B4

// GPIO config
#define GPIO_conf0_REG				0x00B8
#define GPIO_conf2_REG				0x00BC

// Watch dog
#define Watchdog0_REG				0x00C0
#define Watchdog1_REG				0x00C4

#define Swap_in_REG				0x00C8
#define Swap_out_REG				0x00CC

// Tx/Rx Descriptors
#define Send_HBaddr_REG				0x00D0
#define Send_LBaddr_REG				0x00D4
#define Recv_HBaddr_REG				0x00D8
#define Recv_LBaddr_REG				0x00DC
#define Send_HWaddr_REG				0x00E0
#define Send_LWaddr_REG				0x00E4
#define Recv_HWaddr_REG				0x00E8
#define Recv_LWaddr_REG				0x00EC

// Timer Control 
#define Timer_int_REG				0x00F0
#define Timer_REG				0x00F4

// LED control
#define Port0_LED_REG				0x0100
#define Port1_LED_REG				0x0104
#define Port2_LED_REG				0x0108
#define Port3_LED_REG				0x010c
#define Port4_LED_REG				0x0110


/* Macros for accessing Switch control register */
#define ADM5120_SW_REG(_reg)		\
	(*((volatile unsigned long *)(KSEG1ADDR(SWCTRL_BASE + (_reg)))))



/* CODE_REG */
#define CODE_ID_MASK				0x00FFFF
#define CODE_ADM5120_ID				0x5120

#define CODE_REV_MASK				0x0F0000
#define CODE_REV_SHIFT				16
#define CODE_REV_ADM5120_0			0x8

#define CODE_CLK_MASK				0x300000
#define CODE_CLK_SHIFT				20

#define CPU_CLK_175MHZ				0x0
#define CPU_CLK_200MHZ				0x1
#define CPU_CLK_225MHZ				0x2
#define CPU_CLK_250MHZ				0x3

#define CPU_SPEED_175M				(175000000/2)
#define CPU_SPEED_200M				(200000000/2)
#define CPU_SPEED_225M				(225000000/2)
#define CPU_SPEED_250M				(250000000/2)

#define CPU_NAND_BOOT				0x01000000
#define CPU_DCACHE_2K_WAY			(0x1 << 25)
#define CPU_DCACHE_2WAY				(0x1 << 26)
#define CPU_ICACHE_2K_WAY			(0x1 << 27)
#define CPU_ICACHE_2WAY				(0x1 << 28)

#define CPU_GMII_SUPPORT			0x20000000

#define CPU_PQFP_MODE				(0x1 << 29)

#define CPU_CACHE_LINE_SIZE			16

/* SftRest_REG	*/
#define SOFTWARE_RESET				0x1

/* Boot_done_REG */
#define BOOT_DONE				0x1

/* SWReset_REG */
#define SWITCH_RESET				0x1

/* Global_St_REG */
#define DATA_BUF_BIST_FAILED			(0x1 << 0)
#define LINK_TAB_BIST_FAILED			(0x1 << 1)
#define MC_TAB_BIST_FAILED			(0x1 << 2)
#define ADDR_TAB_BIST_FAILED			(0x1 << 3)
#define DCACHE_D_FAILED				(0x3 << 4)
#define DCACHE_T_FAILED				(0x1 << 6)
#define ICACHE_D_FAILED				(0x3 << 7)
#define ICACHE_T_FAILED				(0x1 << 9)
#define BIST_FAILED_MASK			0x03FF

#define ALLMEM_TEST_DONE			(0x1 << 10)

#define SKIP_BLK_CNT_MASK			0x1FF000
#define SKIP_BLK_CNT_SHIFT			12


/* PHY_st_REG */
#define PORT_LINK_MASK				0x0000001F
#define PORT_MII_LINKFAIL			0x00000020
#define PORT_SPEED_MASK				0x00001F00

#define PORT_GMII_SPD_MASK			0x00006000
#define PORT_GMII_SPD_10M			0
#define PORT_GMII_SPD_100M			0x00002000
#define PORT_GMII_SPD_1000M			0x00004000

#define PORT_DUPLEX_MASK			0x003F0000
#define PORT_FLOWCTRL_MASK			0x1F000000

#define PORT_GMII_FLOWCTRL_MASK			0x60000000
#define PORT_GMII_FC_ON				0x20000000
#define PORT_GMII_RXFC_ON			0x20000000
#define PORT_GMII_TXFC_ON			0x40000000

/* Port_st_REG */
#define PORT_SECURE_ST_MASK			0x001F
#define MII_PORT_TXC_ERR			0x0080

/* Mem_control_REG */
#define SDRAM_SIZE_4MBYTES			0x0001
#define SDRAM_SIZE_8MBYTES			0x0002
#define SDRAM_SIZE_16MBYTES			0x0003
#define SDRAM_SIZE_64MBYTES			0x0004
#define SDRAM_SIZE_128MBYTES			0x0005
#define SDRAM_SIZE_MASK				0x0007

#define MEMCNTL_SDRAM1_EN			(0x1 << 5)

#define ROM_SIZE_DISABLE			0x0000
#define ROM_SIZE_512KBYTES			0x0001
#define ROM_SIZE_1MBYTES			0x0002
#define	ROM_SIZE_2MBYTES			0x0003
#define ROM_SIZE_4MBYTES			0x0004
#define ROM_SIZE_8MBYTES			0x0005
#define ROM_SIZE_MASK				0x0007

#define ROM0_SIZE_SHIFT				8
#define ROM1_SIZE_SHIFT				16


/* SW_conf_REG */
#define SW_AGE_TIMER_MASK			0x000000F0
#define SW_AGE_TIMER_DISABLE			0x0
#define SW_AGE_TIMER_FAST			0x00000080
#define SW_AGE_TIMER_300SEC			0x00000010
#define SW_AGE_TIMER_600SEC			0x00000020
#define SW_AGE_TIMER_1200SEC			0x00000030
#define SW_AGE_TIMER_2400SEC			0x00000040
#define SW_AGE_TIMER_4800SEC			0x00000050
#define SW_AGE_TIMER_9600SEC			0x00000060
#define SW_AGE_TIMER_19200SEC			0x00000070
//#define SW_AGE_TIMER_38400SEC			0x00000070

#define SW_BC_PREV_MASK				0x00000300
#define SW_BC_PREV_DISABLE			0
#define SW_BC_PREV_64BC				0x00000100
#define SW_BC_PREV_48BC				0x00000200
#define SW_BC_PREV_32BC				0x00000300

#define SW_MAX_LEN_MASK				0x00000C00
#define SW_MAX_LEN_1536				0
#define SW_MAX_LEN_1522				0x00000800
#define SW_MAX_LEN_1518				0x00000400

#define SW_DIS_COLABT				0x00001000

#define SW_HASH_ALG_MASK			0x00006000
#define SW_HASH_ALG_DIRECT			0
#define SW_HASH_ALG_XOR48			0x00002000
#define SW_HASH_ALG_XOR32			0x00004000

#define SW_DISABLE_BACKOFF_TIMER		0x00008000

#define SW_BP_NUM_MASK				0x000F0000
#define SW_BP_NUM_SHIFT				16
#define SW_BP_MODE_MASK				0x00300000
#define SW_BP_MODE_DISABLE			0
#define SW_BP_MODE_JAM				0x00100000
#define SW_BP_MODE_JAMALL			0x00200000
#define SW_BP_MODE_CARRIER			0x00300000
#define SW_RESRV_MC_FILTER			0x00400000
#define SW_BISR_DISABLE				0x00800000

#define SW_DIS_MII_WAS_TX			0x01000000
#define SW_BISS_EN				0x02000000
#define SW_BISS_TH_MASK				0x0C000000
#define SW_BISS_TH_SHIFT			26
#define SW_REQ_LATENCY_MASK			0xF0000000
#define SW_REQ_LATENCY_SHIFT			28


/* CPUp_conf_REG */
#define SW_CPU_PORT_DISABLE			0x00000001
#define SW_PADING_CRC				0x00000002
#define SW_BRIDGE_MODE				0x00000004

#define SW_DIS_UN_SHIFT				9
#define SW_DIS_UN_MASK				(0x3F << SW_DIS_UN_SHIFT)
#define SW_DIS_MC_SHIFT				16
#define SW_DIS_MC_MASK				(0x3F << SW_DIS_MC_SHIFT)
#define SW_DIS_BC_SHIFT				24
#define SW_DIS_BC_MASK				(0x3F << SW_DIS_BC_SHIFT)


/* Port_conf0_REG */
#define SW_DISABLE_PORT_MASK			0x0000003F
#define SW_EN_MC_MASK				0x00003F00
#define SW_EN_MC_SHIFT				8
#define SW_EN_BP_MASK				0x003F0000
#define SW_EN_BP_SHIFT				16
#define SW_EN_FC_MASK				0x3F000000
#define SW_EN_FC_SHIFT				24


/* Port_conf1_REG */
#define SW_DIS_SA_LEARN_MASK			0x0000003F
#define SW_PORT_BLOCKING_MASK			0x00000FC0
#define SW_PORT_BLOCKING_SHIFT			6
#define SW_PORT_BLOCKING_ON			0x1

#define SW_PORT_BLOCKING_MODE_MASK		0x0003F000
#define SW_PORT_BLOCKING_MODE_SHIFT		12
#define SW_PORT_BLOCKING_CTRLONLY		0x1

#define SW_EN_PORT_AGE_MASK			0x03F00000
#define SW_EN_PORT_AGE_SHIFT			20
#define SW_EN_SA_SECURED_MASK			0xFC000000
#define SW_EN_SA_SECURED_SHIFT			26


/* Port_conf2_REG */
#define SW_GMII_AN_EN				0x00000001
#define SW_GMII_FORCE_SPD_MASK			0x00000006
#define SW_GMII_FORCE_SPD_10M			0
#define SW_GMII_FORCE_SPD_100M			0x2
#define SW_GMII_FORCE_SPD_1000M			0x4

#define SW_GMII_FORCE_FULL_DUPLEX		0x00000008

#define SW_GMII_FORCE_RXFC			0x00000010
#define SW_GMII_FORCE_TXFC			0x00000020

#define SW_GMII_EN				0x00000040
#define SW_GMII_REVERSE				0x00000080

#define SW_GMII_TXC_CHECK_EN			0x00000100

#define SW_LED_FLASH_TIME_MASK			0x00030000
#define SW_LED_FLASH_TIME_30MS			0
#define SW_LED_FLASH_TIME_60MS			0x00010000
#define SW_LED_FLASH_TIME_240MS			0x00020000
#define SW_LED_FLASH_TIME_480MS			0x00030000


/* Send_trig_REG */
#define SEND_TRIG_LOW				0x0001
#define SEND_TRIG_HIGH				0x0002


/* Srch_cmd_REG */
#define SW_MAC_SEARCH_START			0x000001
#define SW_MAX_SEARCH_AGAIN			0x000002

/* MAC_wt0_REG */
#define SW_MAC_WRITE				0x00000001
#define SW_MAC_WRITE_DONE			0x00000002
#define SW_MAC_FILTER_EN			0x00000004
#define SW_MAC_VLANID_SHIFT			3
#define SW_MAC_VLANID_MASK			0x00000038
#define SW_MAC_VLANID_EN			0x00000040
#define SW_MAC_PORTMAP_MASK			0x00001F80
#define SW_MAC_PORTMAP_SHIFT			7
#define SW_MAC_AGE_MASK				(0x7 << 13)
#define SW_MAC_AGE_STATIC			(0x7 << 13)
#define SW_MAC_AGE_VALID			(0x1 << 13)
#define SW_MAC_AGE_EMPTY			0

/* BW_cntl0_REG */
#define SW_PORT_TX_NOLIMIT			0
#define SW_PORT_TX_64K				1
#define SW_PORT_TX_128K				2
#define SW_PORT_TX_256K				3
#define SW_PORT_TX_512K				4
#define SW_PORT_TX_1M				5
#define SW_PORT_TX_4M				6
#define SW_PORT_TX_10MK				7

/* BW_cntl1_REG */
#define SW_TRAFFIC_SHAPE_IPG			(0x1 << 31)

/* PHY_cntl0_REG */
#define SW_PHY_ADDR_MASK			0x0000001F
#define PHY_ADDR_MAX				0x1f
#define SW_PHY_REG_ADDR_MASK			0x00001F00
#define SW_PHY_REG_ADDR_SHIFT			8
#define PHY_REG_ADDR_MAX			0x1f
#define SW_PHY_WRITE				0x00002000
#define SW_PHY_READ				0x00004000
#define SW_PHY_WDATA_MASK			0xFFFF0000
#define SW_PHY_WDATA_SHIFT			16


/* PHY_cntl1_REG */
#define SW_PHY_WRITE_DONE			0x00000001
#define SW_PHY_READ_DONE			0x00000002
#define SW_PHY_RDATA_MASK			0xFFFF0000
#define SW_PHY_RDATA_SHIFT			16

/* FC_th_REG */
/* Adj_port_th_REG */
/* Port_th_REG */

/* PHY_cntl2_REG */
#define SW_PHY_AN_MASK				0x0000001F
#define SW_PHY_SPD_MASK				0x000003E0
#define SW_PHY_SPD_SHIFT			5
#define SW_PHY_DPX_MASK				0x00007C00
#define SW_PHY_DPX_SHIFT			10
#define SW_FORCE_FC_MASK			0x000F8000
#define SW_FORCE_FC_SHIFT			15
#define SW_PHY_NORMAL_MASK			0x01F00000
#define SW_PHY_NORMAL_SHIFT			20
#define SW_PHY_AUTOMDIX_MASK			0x3E000000
#define SW_PHY_AUTOMDIX_SHIFT			25
#define SW_PHY_REC_MCCAVERAGE			0x40000000


/* PHY_cntl3_REG */
/* Pri_cntl_REG */
/* VLAN_pri_REG */
/* TOS_en_REG */
/* TOS_map0_REG */
/* TOS_map1_REG */
/* Custom_pri1_REG */
/* Custom_pri2_REG */
/* Empty_cnt_REG */
/* Port_cnt_sel_REG */
/* Port_cnt_REG */


/* SW_Int_st_REG & SW_Int_mask_REG */
#define SEND_H_DONE_INT				0x0000001
#define SEND_L_DONE_INT				0x0000002
#define RX_H_DONE_INT				0x0000004
#define RX_L_DONE_INT				0x0000008
#define RX_H_DESC_FULL_INT			0x0000010
#define RX_L_DESC_FULL_INT			0x0000020
#define PORT0_QUE_FULL_INT			0x0000040
#define PORT1_QUE_FULL_INT			0x0000080
#define PORT2_QUE_FULL_INT			0x0000100
#define PORT3_QUE_FULL_INT			0x0000200
#define PORT4_QUE_FULL_INT			0x0000400
#define PORT5_QUE_FULL_INT			0x0000800

#define CPU_QUE_FULL_INT			0x0002000
#define GLOBAL_QUE_FULL_INT			0x0004000
#define MUST_DROP_INT				0x0008000
#define BC_STORM_INT				0x0010000

#define PORT_STATUS_CHANGE_INT			0x0040000
#define INTRUDER_INT				0x0080000
#define	WATCHDOG0_EXPR_INT			0x0100000
#define WATCHDOG1_EXPR_INT			0x0200000
#define RX_DESC_ERR_INT				0x0400000
#define SEND_DESC_ERR_INT			0x0800000
#define CPU_HOLD_INT				0x1000000
#define SWITCH_INT_MASK				0x1FDEFFF


/* GPIO_conf0_REG */
#define GPIO0_INPUT_MODE			0x00000001
#define GPIO1_INPUT_MODE			0x00000002
#define GPIO2_INPUT_MODE			0x00000004
#define GPIO3_INPUT_MODE			0x00000008
#define GPIO4_INPUT_MODE			0x00000010
#define GPIO5_INPUT_MODE			0x00000020
#define GPIO6_INPUT_MODE			0x00000040
#define GPIO7_INPUT_MODE			0x00000080

#define GPIO0_OUTPUT_MODE			0
#define GPIO1_OUTPUT_MODE			0
#define GPIO2_OUTPUT_MODE			0
#define GPIO3_OUTPUT_MODE			0
#define GPIO4_OUTPUT_MODE			0
#define GPIO5_OUTPUT_MODE			0
#define GPIO6_OUTPUT_MODE			0
#define GPIO7_OUTPUT_MODE			0

#define GPIO0_INPUT_MASK			0x00000100
#define GPIO1_INPUT_MASK			0x00000200
#define GPIO2_INPUT_MASK			0x00000400
#define GPIO3_INPUT_MASK			0x00000800
#define GPIO4_INPUT_MASK			0x00001000
#define GPIO5_INPUT_MASK			0x00002000
#define GPIO6_INPUT_MASK			0x00004000
#define GPIO7_INPUT_MASK			0x00008000

#define GPIO0_OUTPUT_EN				0x00010000
#define GPIO1_OUTPUT_EN				0x00020000
#define GPIO2_OUTPUT_EN				0x00040000
#define GPIO3_OUTPUT_EN				0x00080000
#define GPIO4_OUTPUT_EN				0x00100000
#define GPIO5_OUTPUT_EN				0x00200000
#define GPIO6_OUTPUT_EN				0x00400000
#define GPIO7_OUTPUT_EN				0x00800000

#define GPIO_CONF0_OUTEN_MASK			0x00ff0000

#define GPIO0_OUTPUT_HI				0x01000000
#define GPIO1_OUTPUT_HI				0x02000000
#define GPIO2_OUTPUT_HI				0x04000000
#define GPIO3_OUTPUT_HI				0x08000000
#define GPIO4_OUTPUT_HI				0x10000000
#define GPIO5_OUTPUT_HI				0x20000000
#define GPIO6_OUTPUT_HI				0x40000000
#define GPIO7_OUTPUT_HI				0x80000000

#define GPIO0_OUTPUT_LOW			0
#define GPIO1_OUTPUT_LOW			0
#define GPIO2_OUTPUT_LOW			0
#define GPIO3_OUTPUT_LOW			0
#define GPIO4_OUTPUT_LOW			0
#define GPIO5_OUTPUT_LOW			0
#define GPIO6_OUTPUT_LOW			0
#define GPIO7_OUTPUT_LOW			0


/* GPIO_conf2_REG */
#define EXTIO_WAIT_EN				(0x1 << 6)
#define EXTIO_CS1_INT1_EN			(0x1 << 5)
#define EXTIO_CS0_INT0_EN			(0x1 << 4)

/* Watchdog0_REG, Watchdog1_REG */
#define WATCHDOG0_RESET_EN			0x80000000
#define WATCHDOG1_DROP_EN			0x80000000

#define WATCHDOG_TIMER_SET_MASK			0x7FFF0000
#define WATCHDOG_TIMER_SET_SHIFT		16
#define WATCHDOG_TIMER_MASK			0x00007FFF


/* Timer_int_REG */
#define SW_TIMER_INT_DISABLE			0x10000
#define SW_TIMER_INT				0x1

/* Timer_REG */
#define SW_TIMER_EN				0x10000
#define SW_TIMER_MASK				0xffff
#define SW_TIMER_10MS_TICKS			0x3D09
#define SW_TIMER_1MS_TICKS			0x61A
#define SW_TIMER_100US_TICKS			0x9D


/* Port0_LED_REG, Port1_LED_REG, Port2_LED_REG, Port3_LED_REG, Port4_LED_REG*/
#define GPIOL_INPUT_MODE			0x00
#define GPIOL_OUTPUT_FLASH			0x01
#define GPIOL_OUTPUT_LOW			0x02
#define GPIOL_OUTPUT_HIGH			0x03
#define GPIOL_LINK_LED				0x04
#define GPIOL_SPEED_LED				0x05
#define GPIOL_DUPLEX_LED			0x06
#define GPIOL_ACT_LED				0x07
#define GPIOL_COL_LED				0x08
#define GPIOL_LINK_ACT_LED			0x09
#define GPIOL_DUPLEX_COL_LED			0x0A
#define GPIOL_10MLINK_ACT_LED			0x0B
#define GPIOL_100MLINK_ACT_LED			0x0C
#define GPIOL_CTRL_MASK				0x0F

#define GPIOL_INPUT_MASK			0x7000
#define GPIOL_INPUT_0_MASK			0x1000
#define GPIOL_INPUT_1_MASK			0x2000
#define GPIOL_INPUT_2_MASK			0x4000

#define PORT_LED0_SHIFT				0
#define PORT_LED1_SHIFT				4
#define PORT_LED2_SHIFT				8


/*===========================  UART Control Register  ========================*/
#define UART_DR_REG				0x00
#define UART_RSR_REG				0x04
#define UART_ECR_REG				0x04
#define UART_LCR_H_REG				0x08
#define UART_LCR_M_REG				0x0c
#define UART_LCR_L_REG				0x10
#define UART_CR_REG				0x14
#define UART_FR_REG				0x18
#define UART_IIR_REG				0x1c
#define UART_ICR_REG				0x1C
#define UART_ILPR_REG				0x20

/*  rsr/ecr reg  */
#define UART_OVERRUN_ERR			0x08
#define UART_BREAK_ERR				0x04
#define UART_PARITY_ERR				0x02
#define UART_FRAMING_ERR			0x01
#define UART_RX_STATUS_MASK			0x0f
#define UART_RX_ERROR				( UART_BREAK_ERR | UART_PARITY_ERR | UART_FRAMING_ERR)

/*  lcr_h reg  */
#define UART_SEND_BREAK				0x01
#define UART_PARITY_EN				0x02
#define UART_EVEN_PARITY			0x04
#define UART_TWO_STOP_BITS			0x08
#define UART_ENABLE_FIFO			0x10

#define UART_WLEN_5BITS				0x00
#define UART_WLEN_6BITS				0x20
#define UART_WLEN_7BITS				0x40
#define UART_WLEN_8BITS				0x60
#define UART_WLEN_MASK				0x60

/*  cr reg  */
#define UART_PORT_EN				0x01
#define UART_SIREN				0x02
#define UART_SIRLP				0x04
#define UART_MODEM_STATUS_INT_EN		0x08
#define UART_RX_INT_EN				0x10
#define UART_TX_INT_EN				0x20
#define UART_RX_TIMEOUT_INT_EN			0x40
#define UART_LOOPBACK_EN			0x80

/*  fr reg  */
#define UART_CTS				0x01
#define UART_DSR				0x02
#define UART_DCD				0x04
#define UART_BUSY				0x08
#define UART_RX_FIFO_EMPTY			0x10
#define UART_TX_FIFO_FULL			0x20
#define UART_RX_FIFO_FULL			0x40
#define UART_TX_FIFO_EMPTY			0x80

/*  iir/icr reg  */
#define UART_MODEM_STATUS_INT			0x01
#define UART_RX_INT				0x02
#define UART_TX_INT				0x04
#define UART_RX_TIMEOUT_INT			0x08

#define UART_INT_MASK				0x0f

#define ADM5120_UARTCLK_FREQ			62500000

#define UART_BAUDDIV(_rate)			((unsigned long)(ADM5120_UARTCLK_FREQ/(16*(_rate)) - 1))

/*  uart_baudrate  */
#define UART_230400bps_DIVISOR			UART_BAUDDIV(230400)
#define UART_115200bps_DIVISOR			UART_BAUDDIV(115200)
#define UART_76800bps_DIVISOR			UART_BAUDDIV(76800)
#define UART_57600bps_DIVISOR			UART_BAUDDIV(57600)
#define UART_38400bps_DIVISOR			UART_BAUDDIV(38400)
#define UART_19200bps_DIVISOR			UART_BAUDDIV(19200)
#define UART_14400bps_DIVISOR			UART_BAUDDIV(14400)
#define UART_9600bps_DIVISOR			UART_BAUDDIV(9600)
#define UART_2400bps_DIVISOR			UART_BAUDDIV(2400)
#define UART_1200bps_DIVISOR			UART_BAUDDIV(1200)


/* Cache Controller */
//#define ADM5120_CACHE_CTRL_BASE		0x70000000
#define ADM5120_CACHE_LINE_SIZE			16
//#define ADM5120_CACHE_CTRL_REGSIZE		4


/********** GPIO macro *************/
#define GPIO_MEASURE	0x000f00f0 //enable output status of pin 0, 1, 2, 3 

#define GPIO_MEASURE_INIT() \
do { \
	ADM5120_SW_REG(GPIO_conf0_REG) = GPIO_MEASURE; \
} while (0)


#define GPIO_SET_HI(num) \
do { \
	ADM5120_SW_REG(GPIO_conf0_REG) |= 1 << (24 + num); \
} while (0)


#define GPIO_SET_LOW(num) \
do { \
	ADM5120_SW_REG(GPIO_conf0_REG) &= ~(1 << (24 + num)); \
} while (0)


#define GPIO_TOGGLE(num) \
do { \
	ADM5120_SW_REG(GPIO_conf0_REG) ^= (1 << (24 + num)); \
} while (0)


#define BOOT_LINE_SIZE		256
#define BSP_STR_LEN		64

/*
 * System configuration
 */
/*typedef struct BOARD_CFG_S
{
	unsigned long blmagic;
	unsigned char bootline[BOOT_LINE_SIZE+1];
	
	unsigned long macmagic;
	unsigned char mac[4][8];

	unsigned long idmagic;    
	unsigned char serial[BSP_STR_LEN+1];

	unsigned long vermagic;
	unsigned char ver[BSP_STR_LEN+1];
	
} BOARD_CFG_T, *PBOARD_CFG_T;


#define BL_MAGIC			0x6c62676d
#define MAC_MAGIC			0x636d676d
#define VER_MAGIC			0x7276676d
#define ID_MAGIC			0x6469676d
*/
typedef struct BOARD_CFG_S
{
	unsigned long macmagic;
	unsigned char sr[3];
	unsigned char mac[3][6];
	
} BOARD_CFG_T, *PBOARD_CFG_T;


#define MAC_MAGIC			0x31305348


#endif /* __ADM5120_H__ */
