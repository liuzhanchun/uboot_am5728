/*
 * Copyright (C) 2016 Texas Instruments Incorporated, <www.ti.com>
 * Author: Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * Based on the LCD and DSS drivers in the 3.14 kernel.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */
#ifndef DRA7XX_DISPLAY_H
#define DRA7XX_DISPLAY_H
#include <common.h>

#define DISPC_IRQ_FRAMEDONE			(1 << 0)

#define CM_DIV_H12_DPLL_PER			0x4A00815C

#define CTRL_CORE_SMA_SW_1	     	0x4A002534

/* DSS Registers */
#define DISPC_DIVISOR				0x58001804
#define DSS_CTRL				    0x58000040
#define DISPC_SYSCONFIG             0x58001010
#define DISPC_IRQSTATUS             0x58001018
#define DISPC_IRQENABLE				0x5800101C
#define DISPC_CONTROL1				0x58001040
#define DISPC_CONFIG1				0x58001044
#define DISPC_DEFAULT_COLOR0	    0x5800104C
#define DISPC_TIMING_H1				0x58001064
#define DISPC_TIMING_V1				0x58001068
#define DISPC_POLL_FREQ1			0x5800106C
#define DISPC_DIVISOR1				0x58001070
#define DISPC_SIZE_LCD1				0x5800107C

#define DISPC_VID1_POSITION			0x580010C4
#define DISPC_VID1_SIZE				0x580010C8
#define DISPC_VID1_ATTRIBUTES		0x580010CC
#define DISPC_VID1_ROW_INC			0x580010D8
#define DISPC_VID1_PICTURE_SIZE		0x580010E4

#define DISPC_VID1_FIR_COEF_H_0		0x580010F0
#define DISPC_VID1_FIR_COEF_HV_0	0x580010F4
#define DISPC_VID1_FIR_COEF_V_0		0x580011E0
#define DISPC_VID1_FIR_COEF_H2_0	0x58001648
#define DISPC_VID1_FIR_COEF_HV2_0	0x5800164C
#define DISPC_VID1_FIR_COEF_V2_0	0x58001688

#define DISPC_VID1_ATTRIBUTES2		0x58001624
#define DISPC_VID1_FIR2				0x5800163C
#define DISPC_VID1_AACU2_0			0x58001640
#define DISPC_VID1_AACU2_1			0x58001644
#define DISPC_VID1_CONV_COEF0		0x58001130
#define DISPC_VID1_BA_0				0x580010BC
#define DISPC_VID1_BA_1				0x580010C0
#define DISPC_VID1_BA_UV_0			0x58001600
#define DISPC_VID1_BA_UV_1			0x58001604

#define DISPC_VID3_POSITION			0x5800139C
#define DISPC_VID3_SIZE				0x580013A8
#define DISPC_VID3_ATTRIBUTES		0x58001370
#define DISPC_VID3_ROW_INC			0x580013A4
#define DISPC_VID3_PICTURE_SIZE		0x58001394

#define DISPC_VID3_FIR_COEF_H_0		0x58001310
#define DISPC_VID3_FIR_COEF_HV_0	0x58001314
#define DISPC_VID3_FIR_COEF_V_0		0x58001350
#define DISPC_VID3_FIR_COEF_H2_0	0x58001730
#define DISPC_VID3_FIR_COEF_HV2_0	0x58001734
#define DISPC_VID3_FIR_COEF_V2_0	0x58001770

#define DISPC_VID3_ATTRIBUTES2		0x5800162C
#define DISPC_VID3_FIR2				0x58001724
#define DISPC_VID3_AACU2_0			0x58001728
#define DISPC_VID3_AACU2_1			0x5800172C
#define DISPC_VID3_CONV_COEF0		0x58001374

#define DISPC_VID3_BA_0				0x58001308
#define DISPC_VID3_BA_1				0x5800130C
#define DISPC_VID3_BA_UV_0			0x58001610
#define DISPC_VID3_BA_UV_1			0x58001614


enum display_lcd_type {
	LCD_TYPE_043A = 0, // 4.3 inch
	LCD_TYPE_056A = 1, // 5.6 inch
	LCD_TYPE_070A = 2, // 7.0 inch
	LCD_TYPE_INVALID = 0xff,
};

void setup_display(u32 lcd_type, u32 address);
void video_pll_conf(u32 lcd_type);
void print_video_pll_debug_info(void);
#endif