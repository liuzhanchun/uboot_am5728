/*
 * (C) Copyright 2016
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Chaitanya Ghone <csghone@ti.com>
 * Piyali Goswami <piyali_g@ti.com>
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include "display.h"

#define HWREG(addr)		                  (*(unsigned int *)(addr))
#define FLD_MASK(start, end)	          (((1 << ((start) - (end) + 1)) - 1) << (end))
#define FLD_VAL(val, start, end)          (((val) << (end)) & FLD_MASK(start, end))
#define FLD_GET(val, start, end)          (((val) & FLD_MASK(start, end)) >> (end))
#define FLD_MOD(orig, val, start, end)    (((orig) & ~FLD_MASK(start, end)) | FLD_VAL(val, start, end))

#define uint32_t		unsigned int
#define int32_t			int

#define PLLCTRL1				0x58004300
#define PLLCTRL2				0x58005300
#define PLLCTRL3				0x58009300
#define DSI_PROTOCOL_ENGINE1	0x58004000
#define DSI_PROTOCOL_ENGINE2	0x58005000
#define DSI_PROTOCOL_ENGINE3	0x58009000

/*DSS PLL Registers*/
#define PLL_CONTROL		  	   (0x00000000u)
#define PLL_STATUS		  	   (0x00000004u)
#define PLL_GO			  	   (0x00000008u)
#define PLL_CONFIGURATION1	   (0x0000000Cu)
#define PLL_CONFIGURATION2	   (0x00000010u)
#define PLL_CONFIGURATION3	   (0x00000014u)

uint32_t base_addr		 = PLLCTRL1;
#define DSI_CLK_CTRL		 (DSI_PROTOCOL_ENGINE1 + 0x54)

/*
 * For Tronlong Lnnolux_AT070TN83 LCD
 * multiplier/divider/div_h11_clkcfg is calculate with <2047.DSI timings extractor OMAP5.xlsx>
 */
uint32_t multiplier		 = 1782;
uint32_t divider		 = 39;
uint32_t dpll_ramp_rate		 = 0;
uint32_t dpll_ramp_level	 = 0;
uint32_t auto_dpll_mode		 = 0;
uint32_t div_m2_clkcfg		 = 0;
uint32_t div_m3_clkcfg		 = 0;
uint32_t div_h11_clkcfg		 = 8;
uint32_t div_h12_clkcfg		 = 0;
uint32_t div_h13_clkcfg		 = 0;
uint32_t div_h14_clkcfg		 = 0;
uint32_t div_h21_clkcfg		 = 0;
uint32_t div_h22_clkcfg		 = 0;
uint32_t div_h23_clkcfg		 = 0;
uint32_t div_h24_clkcfg		 = 0;
uint32_t emu_override_multiplier = 0;
uint32_t emu_override_divider	 = 0;
uint32_t regm4x_en		 = 0;
uint32_t lpmode_en		 = 0;
uint32_t relock_ramp_en		 = 0;
uint32_t dpll_dcoclkldo_pwdn	 = 0;
uint32_t dpll_driftguard_en	 = 0;
uint32_t emu_override_ctrl_en	 = 0;
uint32_t dcc_en			 = 0;
uint32_t selfreqdco		 = 0;


/* Bitfields */
#define regn_start		    8
#define regn_end		    1
#define regm_start		    20
#define regm_end		    9
#define regm_hsdiv_start	25
#define regm_hsdiv_end		21

#define SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE    (0x4A002100)
#define CTRL_CORE_DSS_PLL_CONTROL		            (0x438U)


void video_pll_conf(u32 lcd_type)
{
	uint32_t regval, temp;

	switch (lcd_type) {
	case LCD_TYPE_043A:
		/* 4.3" LCD */
		divider =	 39 + 1;
		multiplier = 1800;
		div_h11_clkcfg = 9 + 1;
		break;
	case LCD_TYPE_056A:
		/* 5.6" LCD */
		divider =	 39 + 1;
		multiplier = 1764;
		div_h11_clkcfg = 9 + 1;
        break;
	case LCD_TYPE_070A:
		/* 7" LCD */
		divider = 39 + 1;
		multiplier = 1782;
		div_h11_clkcfg = 8 + 1;
		break;
	default:
		break;
	}

	/* Enable DSS SCP Interface - DSI_CLK_CTRL.CIO_CLK_ICG */
	regval  = HWREG(DSI_CLK_CTRL);
	regval |= 0x00004001;
	HWREG(DSI_CLK_CTRL) = regval;

	/*2) Enable PLL programming in CTRL_CORE_DSS_PLL_CONTROL */
	regval = HWREG(SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE + CTRL_CORE_DSS_PLL_CONTROL);
	regval &= ~(0x00000001); /* Enable VIDEO1 PLL */
	regval &= ~(0x00000018); /* DSI 1A Clocksel */
	HWREG(SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE + CTRL_CORE_DSS_PLL_CONTROL) = regval;

	/*3) DSI Power enable - DSI_CLK_CTRL.PLL_PWR_CMD */
	regval  = HWREG(DSI_CLK_CTRL);
	regval |= 0x80000000;
	HWREG(DSI_CLK_CTRL) = regval;

	/*4) Check for PLL reset status - PLL_STATUS.PLLCTRL_RESET_DONE*/
	while ((HWREG(base_addr + PLL_STATUS) & 0x1) != 0x1)
		udelay(1);

	/* PLL_AUTOMODE = manual */
	HWREG(base_addr + PLL_CONTROL) &= 0xFFFFFFFE;

	/* CONFIGURATION1 */
	regval = HWREG(base_addr + PLL_CONFIGURATION1);

	/* PLL_REGN -> (divider - 1) */
	temp = divider-1;
	regval = FLD_MOD((regval), temp, regn_start, regn_end);

	/* PLL_REGM -> dpll_config->multiplier */
	regval = FLD_MOD((regval), multiplier, regm_start, regm_end);

	/* M4_CLOCK_DIV -> dpll_config->div_h11_clkcfg*/
	temp = div_h11_clkcfg > 0 ? (div_h11_clkcfg - 1) : 0;
	regval = FLD_MOD((regval), temp, regm_hsdiv_start, regm_hsdiv_end);
	HWREG(base_addr + PLL_CONFIGURATION1) = regval;

	/* DSS PLL Programming - CONFIGURATION2 */
	regval = HWREG(base_addr + PLL_CONFIGURATION2);

	regval = FLD_MOD((regval), 1, 13, 13);	/* PLL_REFEN */
	regval = FLD_MOD((regval), 0, 14, 14);	/* DSIPHY_CLKINEN */
	regval = FLD_MOD((regval), 1, 20, 20);	/* HSDIVBYPASS */
	regval = FLD_MOD((regval), 3, 22, 21);	/* REF_SYSCLK = sysclk */
	regval = FLD_MOD((regval), 1, 16, 16);	/* CLOCK_EN */

	HWREG(base_addr + PLL_CONFIGURATION2) = regval;

	/* PLL_GO */
	HWREG(base_addr + PLL_GO) |= 0x1;
	while ((HWREG(base_addr + PLL_GO) & 0x1) != 0)
		udelay(1);

	while ((HWREG(base_addr + PLL_STATUS) & 0x2) != 0x2)
		udelay(1);

	/* PLL_CONFIGURATION2 */
	regval = HWREG(base_addr + PLL_CONFIGURATION2);
	regval = FLD_MOD((regval), 0, 0, 0);	/* PLL_IDLE */
	regval = FLD_MOD((regval), 0, 5, 5);	/* PLL_PLLLPMODE */
	regval = FLD_MOD((regval), 0, 6, 6);	/* PLL_LOWCURRSTBY */
	regval = FLD_MOD((regval), 0, 7, 7);	/* PLL_TIGHTPHASELOCK */
	regval = FLD_MOD((regval), 0, 8, 8);	/* PLL_DRIFTGUARDEN */
	regval = FLD_MOD((regval), 0, 10, 9);	/* PLL_LOCKSEL */
	regval = FLD_MOD((regval), 1, 13, 13);	/* PLL_REFEN */
	regval = FLD_MOD((regval), 1, 14, 14);	/* PHY_CLKINEN */
	regval = FLD_MOD((regval), 0, 15, 15);	/* BYPASSEN */
	regval = FLD_MOD((regval), 1, 16, 16);	/* CLOCK_EN */
	regval = FLD_MOD((regval), 0, 17, 17);	/* CLOCK_PWDN */
	regval = FLD_MOD((regval), 1, 18, 18);	/* PROTO_CLOCK_EN */
	regval = FLD_MOD((regval), 0, 19, 19);	/* PROTO_CLOCK_PWDN */
	regval = FLD_MOD((regval), 0, 20, 20);	/* HSDIVBYPASS */
	HWREG(base_addr + PLL_CONFIGURATION2) = regval;

	/* M4 Clock Ack - check before selecting this as source */
	while ((HWREG(base_addr + PLL_STATUS) & 0x80) != 0x80)
		udelay(1);

	regval = HWREG(base_addr + PLL_GO);
	regval = FLD_MOD(regval, 0, 0, 0);
	HWREG(base_addr + PLL_GO) = regval;
}

void print_video_pll_debug_info(void)
{
	printf("CTRL_CORE_DSS_PLL_CONTROL (0x4a002538) = 0x%08x\n",
	       HWREG(0x4a002538));
	printf("DSS_CTRL (0x58000040) = 0x%08x\n",
	       HWREG(0x58000040));
	{
		u32 i = 0;
		for (i = 0; i < 9; i++) {
			printf("0x%08x\t0x%08x\n",
			       0x58004300+(i*4),
			       HWREG(0x58004300+(i*4)));
		}
	}
}
