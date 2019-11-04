/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * Based on previous work by:
 * Robert Tivy <rtivy@ti.com>
 * Amarinder Bindra <a-bindra@ti.com>
 * Sundar Raman <sunds@ti.com>
 * Angela Stegmaier  <angelabaker@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <remoteproc.h>
#include <asm/arch/mux_dra7xx.h>
#include "display.h"

#define DPLL_TIMEOUT                 5000
#define L4_CFG_TARG                  0x4A000000
#define L4_WKUP_TARG                 0x4AE00000
#define IPU2_TARGET_TARG             0x55000000
#define IPU1_TARGET_TARG             0x58800000
#define CTRL_MODULE_CORE             (L4_CFG_TARG + 0x2000)
#define CM_CORE_AON                  (L4_CFG_TARG + 0x5000)
#define CM_CORE                      (L4_CFG_TARG + 0x8000)
#define PRM                          (L4_WKUP_TARG + 0x6000)
#define MPU_CM_CORE_AON              (CM_CORE_AON + 0x300)
#define IPU_CM_CORE_AON              (CM_CORE_AON + 0x500)
#define RTC_CM_CORE_AON              (CM_CORE_AON + 0x740)
#define VPE_CM_CORE_AON              (CM_CORE_AON + 0x760)
#define COREAON_CM_CORE              (CM_CORE + 0x600)
#define CORE_CM_CORE                 (CM_CORE + 0x700)
#define CAM_CM_CORE                  (CM_CORE + 0x1000)
#define DSS_CM_CORE                  (CM_CORE + 0x1100)
#define L3INIT_CM_CORE               (CM_CORE + 0x1300)
#define L4PER_CM_CORE                (CM_CORE + 0x1700)
#define CKGEN_PRM                    (PRM + 0x100)
#define IPU_PRM                      (PRM + 0x500)
#define CORE_PRM                     (PRM + 0x700)
#define WKUPAON_CM                   (PRM + 0x1800)

#define CM_CLKMODE_DPLL_DSP          (0x4A005234)
#define CM_DSP1_CLKSTCTRL            (0x4A005400)
#define CM_DSP2_CLKSTCTRL            (0x4A005600)
#define DSP1_PRM_BASE                (0x4AE06400)
#define DSP2_PRM_BASE                (0x4AE07B00)
#define DSP1_SYS_MMU_CONFIG          (0x40D00018)
#define DSP2_SYS_MMU_CONFIG          (0x41500018)

/* CTRL_CORE_CONTROL_DSP1_RST_VECTOR in TRM */
#define DSP1_BOOTADDR                (0x4A00255C)
/* CTRL_CORE_CONTROL_DSP2_RST_VECTOR in TRM */
#define DSP2_BOOTADDR                (0x4A002560)
#define DRA7XX_CTRL_CORE_DSP_RST_VECT_MASK	(0x3FFFFF << 0)

#define CM_L3MAIN1_CLKSTCTRL         (CORE_CM_CORE + 0x000)
#define CM_IPU2_CLKSTCTRL            (CORE_CM_CORE + 0x200)
#define CM_DMA_CLKSTCTRL             (CORE_CM_CORE + 0x300)
#define CM_EMIF_CLKSTCTRL            (CORE_CM_CORE + 0x400)
#define CM_L4CFG_CLKSTCTRL           (CORE_CM_CORE + 0x600)

#define CM_DSS_CLKSTCTRL             (DSS_CM_CORE + 0x00)
#define CM_CAM_CLKSTCTRL             (CAM_CM_CORE + 0x00)
#define CM_COREAON_CLKSTCTRL         (COREAON_CM_CORE + 0x00)
#define CM_L3INIT_CLKSTCTRL          (L3INIT_CM_CORE + 0x00)
#define CM_GMAC_CLKSTCTRL            (L3INIT_CM_CORE + 0xC0)
#define CM_L4PER_CLKSTCTRL           (L4PER_CM_CORE + 0x000)
#define CM_L4PER_TIMER11_CLKCTRL     (CM_L4PER_CLKSTCTRL + 0x30)
#define CM_L4PER_TIMER3_CLKCTRL      (CM_L4PER_CLKSTCTRL + 0x40)
#define CM_L4PER_TIMER4_CLKCTRL      (CM_L4PER_CLKSTCTRL + 0x48)
#define CM_L4PER_TIMER9_CLKCTRL      (CM_L4PER_CLKSTCTRL + 0x50)
#define CM_L4PER2_CLKSTCTRL          (L4PER_CM_CORE + 0x1FC)
#define CM_L4PER3_CLKSTCTRL          (L4PER_CM_CORE + 0x210)
#define CM_MPU_CLKSTCTRL             (MPU_CM_CORE_AON + 0x00)
#define CM_RTC_CLKSTCTRL             (RTC_CM_CORE_AON + 0x00)
#define CM_VPE_CLKSTCTRL             (VPE_CM_CORE_AON + 0x00)
#define CM_WKUPAON_CLKSTCTRL         (WKUPAON_CM + 0x00)

#define RM_IPU1_RSTCTRL              (IPU_PRM + 0x10)
#define RM_IPU1_RSTST                (IPU_PRM + 0x14)
#define CM_IPU1_CLKSTCTRL            (IPU_CM_CORE_AON + 0x0)
#define CM_IPU1_IPU1_CLKCTRL         (IPU_CM_CORE_AON + 0x20)
#define CM_IPU2_IPU2_CLKCTRL         (CORE_CM_CORE + 0x220)
#define CM_IPU_CLKSTCTRL             (IPU_CM_CORE_AON + 0x40)
#define CM_IPU_MCASP1_CLKCTRL        (IPU_CM_CORE_AON + 0x50)
#define CM_IPU_TIMER5_CLKCTRL        (IPU_CM_CORE_AON + 0x58)
#define CM_IPU_TIMER6_CLKCTRL        (IPU_CM_CORE_AON + 0x60)
#define CM_IPU_TIMER7_CLKCTRL        (IPU_CM_CORE_AON + 0x68)
#define CM_IPU_TIMER8_CLKCTRL        (IPU_CM_CORE_AON + 0x70)
#define CM_L4PER2_L4_PER2_CLKCTRL    (L4PER_CM_CORE + 0x0C)
#define CM_L4PER3_L4_PER3_CLKCTRL    (L4PER_CM_CORE + 0x14)
#define CM_L4PER_I2C1_CLKCTRL        (L4PER_CM_CORE + 0xA0)
#define CM_L4PER_I2C2_CLKCTRL        (L4PER_CM_CORE + 0xA8)
#define CM_L4PER_I2C3_CLKCTRL        (L4PER_CM_CORE + 0xB0)
#define CM_L4PER_I2C4_CLKCTRL        (L4PER_CM_CORE + 0xB8)
#define CM_L4PER_L4_PER1_CLKCTRL     (L4PER_CM_CORE + 0xC0)
#define CM_CAM_VIP1_CLKCTRL          (CAM_CM_CORE + 0x20)
#define CM_CAM_VIP2_CLKCTRL          (CAM_CM_CORE + 0x28)
#define CM_CAM_VIP3_CLKCTRL          (CAM_CM_CORE + 0x30)
#define CM_DMA_DMA_SYSTEM_CLKCTRL    (CORE_CM_CORE + 0x320)
#define CM_L3INSTR_L3_MAIN_2_CLKCTRL (CORE_CM_CORE + 0x728)
#define CM_DSS_DSS_CLKCTRL           (DSS_CM_CORE + 0x20)
#define CM_VPE_VPE_CLKCTRL           (VPE_CM_CORE_AON + 0x04)

#define RM_IPU2_RSTCTRL              (CORE_PRM + 0x210)
#define RM_IPU2_RSTST                (CORE_PRM + 0x214)

#define CTRL_CORE_CONTROL_IO_2       (CTRL_MODULE_CORE + 0x558)

#define CTRL_CORE_CORTEX_M4_MMUADDRTRANSLTR 0x4A002358
#define CTRL_CORE_CORTEX_M4_MMUADDRLOGICTR  0x4A00235C

#define PAGESIZE_1M                          0x0
#define PAGESIZE_64K                         0x1
#define PAGESIZE_4K                          0x2
#define PAGESIZE_16M                         0x3
#define LE                                   0
#define BE                                   1
#define ELEMSIZE_8                           0x0
#define ELEMSIZE_16                          0x1
#define ELEMSIZE_32                          0x2
#define MIXED_TLB                            0x0
#define MIXED_CPU                            0x1

#define PGT_SMALLPAGE_SIZE                   0x00001000
#define PGT_LARGEPAGE_SIZE                   0x00010000
#define PGT_SECTION_SIZE                     0x00100000
#define PGT_SUPERSECTION_SIZE                0x01000000

#define PGT_L1_DESC_PAGE                     0x00001
#define PGT_L1_DESC_SECTION                  0x00002
#define PGT_L1_DESC_SUPERSECTION             0x40002

#define PGT_L1_DESC_PAGE_MASK                0xfffffC00
#define PGT_L1_DESC_SECTION_MASK             0xfff00000
#define PGT_L1_DESC_SUPERSECTION_MASK        0xff000000

#define PGT_L1_DESC_SMALLPAGE_INDEX_SHIFT    12
#define PGT_L1_DESC_LARGEPAGE_INDEX_SHIFT    16
#define PGT_L1_DESC_SECTION_INDEX_SHIFT      20
#define PGT_L1_DESC_SUPERSECTION_INDEX_SHIFT 24

#define PGT_L2_DESC_SMALLPAGE               0x02
#define PGT_L2_DESC_LARGEPAGE               0x01

#define PGT_L2_DESC_SMALLPAGE_MASK          0xfffff000
#define PGT_L2_DESC_LARGEPAGE_MASK          0xffff0000

#define DRA7_RPROC_CMA_BASE_IPU1             0x9d000000
#define DRA7_RPROC_CMA_BASE_IPU2             0x95800000
#define DRA7_RPROC_CMA_BASE_DSP1             0x99000000
#define DRA7_RPROC_CMA_BASE_DSP2             0x9f000000

#define DRA7_RPROC_CMA_SIZE_IPU1             0x02000000
#define DRA7_RPROC_CMA_SIZE_IPU2             0x03800000
#define DRA7_RPROC_CMA_SIZE_DSP1             0x04000000
#define DRA7_RPROC_CMA_SIZE_DSP2             0x00800000

#define VAYU_ES10_CHIPSETID                  0x5436

static struct display_panel_array {
    u32 index;
    char *display_panel;
} display[] = {
    {LCD_TYPE_043A, "TL043A"}, {LCD_TYPE_056A, "TL056A"}, {LCD_TYPE_070A, "TL070A"},
    {LCD_TYPE_INVALID, NULL},
};

static int get_display_type(void)
{
    char *display_panel;
    u8 i;

    display_panel = getenv("display_panel");
    if (!display_panel)
        return LCD_TYPE_INVALID;

    for (i = 0; display[i].index != LCD_TYPE_INVALID; i++) {
        if (!strcmp(display[i].display_panel, display_panel))
            return display[i].index;
    }

    return LCD_TYPE_INVALID;
}

void display_logo(void)
{
	u32 reg;
	u32 display_type = LCD_TYPE_070A;
	u32 val;

    display_type = get_display_type();
    if (display_type == LCD_TYPE_INVALID)
        return;

	debug("Enabling DSS Clock domain\n");
	__raw_writel(0x2, CM_DSS_CLKSTCTRL);

	/* enable DSS */
	reg = __raw_readl(CTRL_CORE_CONTROL_IO_2);
	__raw_writel((reg | 0x1), CTRL_CORE_CONTROL_IO_2);
	reg = __raw_readl(CM_DSS_DSS_CLKCTRL);
	reg = ((reg & ~0x00000003) | 0x00003F00 | 0x2);
	__raw_writel(reg, CM_DSS_DSS_CLKCTRL);

	debug("Enabling DSS\n");
	/* checking if DSS is enabled */
	while ((__raw_readl(CM_DSS_DSS_CLKCTRL) & 0x00030000) != 0);

	debug("Enabling LCD\n");
	video_pll_conf(display_type);
	setup_display(display_type, DRA7_LOGO_START);
}
