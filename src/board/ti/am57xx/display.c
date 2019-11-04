/*
 * Code to configure the LCD connected to DRA7xx EVM.
 *
 * Copyright (C) 2016 Texas Instruments Incorporated, <www.ti.com>
 * Author: Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * Based on the LCD and DSS drivers in the 3.14 kernel.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */
#include <common.h>
#include <asm/io.h>
#include <i2c.h>
#include "display.h"

/* Choose VID1 or VID3 for the DSS pipeline */
#define DISPC_MACRO(REGNAME)    DISPC_VID3_##REGNAME

enum omap_dss_signal_level {
	OMAPDSS_SIG_ACTIVE_LOW,
	OMAPDSS_SIG_ACTIVE_HIGH,
};

enum omap_dss_signal_edge {
	OMAPDSS_DRIVE_SIG_FALLING_EDGE,
	OMAPDSS_DRIVE_SIG_RISING_EDGE,
};

struct omap_video_timings {
	/* Unit: pixels */
	u16 x_res;
	/* Unit: pixels */
	u16 y_res;
	/* Unit: Hz */
	u32 pixelclock;
	/* Unit: pixel clocks */
	u16 hsw;	/* Horizontal synchronization pulse width */
	/* Unit: pixel clocks */
	u16 hfp;	/* Horizontal front porch */
	/* Unit: pixel clocks */
	u16 hbp;	/* Horizontal back porch */
	/* Unit: line clocks */
	u16 vsw;	/* Vertical synchronization pulse width */
	/* Unit: line clocks */
	u16 vfp;	/* Vertical front porch */
	/* Unit: line clocks */
	u16 vbp;	/* Vertical back porch */
	/* Vsync logic level */
	enum omap_dss_signal_level vsync_level;
	/* Hsync logic level */
	enum omap_dss_signal_level hsync_level;
	/* Interlaced or Progressive timings */
	bool interlace;
	/* Pixel clock edge to drive LCD data */
	enum omap_dss_signal_edge data_pclk_edge;
	/* Data enable logic level */
	enum omap_dss_signal_level de_level;
	/* Pixel clock edges to drive HSYNC and VSYNC signals */
	enum omap_dss_signal_edge sync_pclk_edge;
};

struct lcd_board_data {
	struct omap_video_timings timings;
};

/* Innolux_AT043TN24 */
static const struct lcd_board_data innolux_at043tn24_data = {
	.timings = {
		.x_res		= 480,
		.y_res		= 272,

		.pixelclock	= 9000000,

		.hfp		= 2,
		.hsw		= 41,
		.hbp		= 2,

		.vfp		= 2,
		.vsw		= 10,
		.vbp		= 2,

		.vsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.hsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.data_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
		.de_level	= OMAPDSS_SIG_ACTIVE_HIGH,
		.sync_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
	},
};

/* Lnnolux_AT056TN53 */
static const struct lcd_board_data lnnolux_at056tn53_data = {
	.timings = {
		.x_res		= 640,
		.y_res		= 480,

		.pixelclock	= 25200000,

		.hfp		= 16,
		.hsw		= 10,
		.hbp		= 134,

		.vfp		= 32,
		.vsw		= 2,
		.vbp		= 11,

		.vsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.hsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.data_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
		.de_level	= OMAPDSS_SIG_ACTIVE_HIGH,
		.sync_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
	},
};

/* Lnnolux_AT070TN83 */
static const struct lcd_board_data lnnolux_at070tn83_data = {
	.timings = {
		.x_res		= 800,
		.y_res		= 480,

		.pixelclock	= 33000000,

		.hfp		= 210,
		.hsw		= 30,
		.hbp		= 16,

		.vfp		= 22,
		.vsw		= 13,
		.vbp		= 10,

		.vsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.hsync_level	= OMAPDSS_SIG_ACTIVE_LOW,
		.data_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
		.de_level	= OMAPDSS_SIG_ACTIVE_HIGH,
		.sync_pclk_edge	= OMAPDSS_DRIVE_SIG_RISING_EDGE,
	},
};

unsigned int fir_coeff_h[8] = {
	0x00800000u,
	0x0e7df601u,
	0x2172f102u,
	0x3762f001u,
	0xf24e4ef2u,
	0xf06237f6u,
	0xf17221fau,
	0xf67d0efeu
};

unsigned int fir_coeff_hv[8] = {
	0x00800000u,
	0x0e7df6feu,
	0x2172f1fau,
	0x3762f0f6u,
	0xf24e4e00u,
	0xf0623701u,
	0xf1722102u,
	0xf67d0e01u,
};

unsigned int fir_coeff_v[8] = {
	0x00000000u,
	0x0000fe01u,
	0x0000fa02u,
	0x0000f601u,
	0x000000f2u,
	0x000001f6u,
	0x000002fau,
	0x000001feu,
};

unsigned int fir_coeff_h2[8] = {
	0x00800000u,
	0x0e7df601u,
	0x2172f102u,
	0x3762f001u,
	0xf24e4ef2u,
	0xf06237f6u,
	0xf17221fau,
	0xf67d0efeu,
};

unsigned int fir_coeff_hv2[8] = {
	0x00800000u,
	0x0e7df6feu,
	0x2172f1fau,
	0x3762f0f6u,
	0xf24e4e00u,
	0xf0623701u,
	0xf1722102u,
	0xf67d0e01u,
};

unsigned int fir_coeff_v2[8] = {
	0x00000000u,
	0x0000fe01u,
	0x0000fa02u,
	0x0000f601u,
	0x000000f2u,
	0x000001f6u,
	0x000002fau,
	0x000001feu,
};

unsigned int vid_conv_coeff[5] = {
	0x0199012au,
	0x012a0000u,
	0x079c0730u,
	0x0000012au,
	0x00000205u,
};

/*
 *
 * This functions uses the PRCM path for initializing the DSS
 * and the settings are done for the LCD display for a stream
 * of 800x480 resolution buffer on VID3 pipeline. These settings
 * are currently overwritten by the linux kernel which re-initializes
 * the DSS clocks and enables all the video and graphics pipelines.
 *
 */
void setup_display(u32 lcd_type, u32 address)
{
	unsigned int reg;
	unsigned int width  = 800;
	unsigned int height = 480;
	const struct lcd_board_data *bdata;
	const struct omap_video_timings *timings;
	unsigned int i = 0;
	unsigned int val = 0;

	unsigned int tmp_vid_reg_base;

	switch (lcd_type) {
	case LCD_TYPE_043A:
		bdata = &innolux_at043tn24_data;
		break;
	case LCD_TYPE_056A:
		bdata = &lnnolux_at056tn53_data;
		break;
	case LCD_TYPE_070A:
		bdata = &lnnolux_at070tn83_data;
		break;
	default:
		bdata = &lnnolux_at070tn83_data; // default 7 inch LCD
	}

	timings = &bdata->timings;
	width  = timings->x_res;
	height = timings->y_res;

	/* Set the LCD channel 0 clock polarity as rising edge */
	__raw_writel(0x00000, CTRL_CORE_SMA_SW_1);

	/* DIVHS divider for DISPC */
	reg = __raw_readl(CM_DIV_H12_DPLL_PER);
	reg = ((reg & 0xfffffbc0) | 0x204u);
	__raw_writel(reg, CM_DIV_H12_DPLL_PER);

	/* DISPC Divisor to enable LCD */
	__raw_writel(0x010001u, DISPC_DIVISOR);

	/* DISPC_DIVISOR1 */
	switch (lcd_type) {
	case LCD_TYPE_043A:
		__raw_writel(0x010014u, DISPC_DIVISOR1);
		break;
	case LCD_TYPE_056A:
		__raw_writel(0x010007u, DISPC_DIVISOR1);
		break;
	case LCD_TYPE_070A:
		__raw_writel(0x010006u, DISPC_DIVISOR1);
		break;
	default:
		__raw_writel(0x010006u, DISPC_DIVISOR1); // default 7 inch LCD
	};

	/* enable_lcd1 channel output. Use DSS Clock for LCD */
	reg = __raw_readl(DSS_CTRL);
	reg = ((reg & 0xfffcffff) | 0x10001u);
	__raw_writel(reg, DSS_CTRL);

	/* DSS initialization */
	/*
	 * enable the display controller for LCD
	 *
	 * 16: Reset GPOut1
	 * 15: Reset GPout0
	 * 12: No overlay optimization
	 * 9:8 24 bit otuput aligned in LSB of pixel data interface
	 * 3 : Active TFT operation enabled.
	 * 1 : LCD output enabled.
	 */
	__raw_writel(0x18309, DISPC_CONTROL1);

	/* Default color for display when video plane has no data.
	 * Format is YUV
	 */
	__raw_writel(0x0000F0u, DISPC_DEFAULT_COLOR0);

	/* 31:20 hbp, 19:8 hfp, 7:0 hsw */

	reg = ((timings->hbp<<20) | (timings->hfp<<8) | timings->hsw);
	__raw_writel(reg, DISPC_TIMING_H1);

	/* 31:20 vbp, 19:8 vfp, 7:0 vsw */
	reg = ((timings->vbp<<20) | (timings->vfp<<8) | timings->vsw);
	__raw_writel(reg, DISPC_TIMING_V1);

	/* DISPC_POLL_FREQ1 */
	__raw_writel(0x33000u, DISPC_POLL_FREQ1);

	reg = ((((height-1)<<16)&0xFFFF0000) | ((width-1)&0xFFFF));
	__raw_writel(reg, DISPC_SIZE_LCD1);

	__raw_writel(0x0u, DISPC_MACRO(POSITION)); /* DISPC_VID1_SIZE */

	reg = ((((height-1)<<16)&0xFFFF0000) | ((width-1)&0xFFFF));
	__raw_writel(reg, DISPC_MACRO(SIZE));

	/* DISPC_VID1_ATTRIBUTES
	 * 31:30 primary lcd output is selected
	 * 29 : incremental burst type
	 * 27:26 z order: 2
	 * 25: z order enabled
	 * 24: no self refresh
	 * 23: high priority pipeline
	 * 21: 5 taps used in vertical filtering logic
	 * 15:14 - 8x128 bit bursts
	 * 13:12 - rotation by 90 degrees
	 * 9: Enable color conversion from YUV to RGB
	 * 6:5 : Enable both horizontal and vertical resize processing
	 * 4:1 NV12 format
	 */
	__raw_writel(0x0aa08260, DISPC_MACRO(ATTRIBUTES));
	__raw_writel(0x1, DISPC_MACRO(ROW_INC)); /* ROW_INC */

	 reg = ((((height-1)<<16)&0xFFFF0000) | ((width-1)&0xFFFF));
	__raw_writel(reg, DISPC_MACRO(PICTURE_SIZE));

	/* setup the scalar coefficients */
	__raw_writel(0x02000200u, DISPC_MACRO(FIR2));
	__raw_writel(0x06000000u, DISPC_MACRO(AACU2_0));
	__raw_writel(0x0u, DISPC_MACRO(AACU2_1));

	__raw_writel(0x00000100u, DISPC_MACRO(ATTRIBUTES2));

	/* Y layer coefs */
	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_H_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_h[i], tmp_vid_reg_base+i*8);

	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_HV_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_hv[i], tmp_vid_reg_base+i*8);

	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_V_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_v[i], tmp_vid_reg_base+i*4);

	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_H2_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_h2[i], tmp_vid_reg_base+i*8);

	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_HV2_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_hv2[i], tmp_vid_reg_base+i*8);

	tmp_vid_reg_base = DISPC_MACRO(FIR_COEF_V2_0);
	for (i = 0; i < 8 ; i++)
		__raw_writel(fir_coeff_v2[i], tmp_vid_reg_base+i*4);


	tmp_vid_reg_base = DISPC_MACRO(CONV_COEF0);
	for (i = 0; i < 5 ; i++)
		__raw_writel(vid_conv_coeff[i], tmp_vid_reg_base+i*4);

	/*
	 * Input is in YUV format.
	 * Y in one buffer followed by UV subsampled by 2 in one
	 * dimension in another buffer.
	 */
	__raw_writel(address, DISPC_MACRO(BA_0));

	__raw_writel(address, DISPC_MACRO(BA_1));

	/* Offset to UV buffer */
	__raw_writel((address + (width*height)), DISPC_MACRO(BA_UV_0));
	__raw_writel((address + (width*height)), DISPC_MACRO(BA_UV_1));

	/* DISPC_VID1_ATTRIBUTES: enable the video */
	reg = __raw_readl(DISPC_MACRO(ATTRIBUTES));
	__raw_writel(reg | 0x1u, DISPC_MACRO(ATTRIBUTES));

	reg = __raw_readl(DISPC_CONTROL1);
	__raw_writel(reg | 0x20, DISPC_CONTROL1); /* DISPC_CONTROL1 */
}
