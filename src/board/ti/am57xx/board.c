/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <palmas.h>
#include <sata.h>
#include <usb.h>
#include <asm/omap_common.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sata.h>
#include <asm/arch/gpio.h>
#include <asm/arch/omap.h>
#include <environment.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <ti-usb-phy-uboot.h>
#include <fs.h>
#include <mmc.h>

#include "../common/board_detect.h"
#include "mux_data.h"
#include <lzc_config.h>

#define AM572X_GPIO7_BASE           0x48051000
#define AM572X_GPIO_OE_OFF          0x134
#define AM572X_GPIO_SETDATA_OUT_OFF 0x194

#define board_is_x15()		board_ti_is("BBRDX15_")
#define board_is_x15_revb1()	(board_ti_is("BBRDX15_") && \
				 (strncmp("B.10", board_ti_get_rev(), 3) <= 0))
#define board_is_am572x_evm()	board_ti_is("AM572PM_")
#define board_is_am572x_evm_reva3()	\
				(board_ti_is("AM572PM_") && \
				 (strncmp("A.30", board_ti_get_rev(), 3) <= 0))
#define board_is_am572x_idk()	board_ti_is("AM572IDK")
#define board_is_am571x_idk()	board_ti_is("AM571IDK")

#ifdef CONFIG_DRIVER_TI_CPSW
#include <cpsw.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_EMMC_BOOTPART_SEL	GPIO_TO_PIN(3, 23)
#define GPIO_ETH_LCD		GPIO_TO_PIN(2, 22)
/* GPIO 7_11 */
#define GPIO_DDR_VTT_EN 203

#define SYSINFO_BOARD_NAME_MAX_LEN	45

const struct omap_sysinfo sysinfo = {
	"Board: UNKNOWN(BeagleBoard X15?) REV UNKNOWN\n"
};

static const struct dmm_lisa_map_regs beagle_x15_lisa_regs = {
	.dmm_lisa_map_3 = 0x80740300,
	.is_ma_present  = 0x1
};

static const struct dmm_lisa_map_regs am571x_idk_lisa_regs = {
	.dmm_lisa_map_3 = 0x80640100,
	.is_ma_present  = 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	if (board_is_am571x_idk())
		*dmm_lisa_regs = &am571x_idk_lisa_regs;
	else
		*dmm_lisa_regs = &beagle_x15_lisa_regs;
}

static const struct emif_regs beagle_x15_emif1_ddr3_400mhz_emif_regs = {
	.sdram_config_init	= 0x61840BB2,
	.sdram_config		= 0x61840BB2,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000030D4,
	.ref_ctrl_final		= 0x00000C30,
	.sdram_tim1		= 0xCAAAE523,
	.sdram_tim2		= 0x208F7FDA,
	.sdram_tim3		= 0x407F88B8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x5007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x00244009,
	.emif_ddr_phy_ctlr_1	= 0x0E244009,
	.emif_ddr_ext_phy_ctrl_1 = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

static const struct emif_regs beagle_x15_emif1_ddr3_532mhz_emif_regs = {
	.sdram_config_init	= 0x61851BB2,
	.sdram_config		= 0x61851BB2,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000040F1,
	.ref_ctrl_final		= 0x00001035,
	.sdram_tim1		= 0xCCCF36B3,
	.sdram_tim2		= 0x30BF7FDA,
	.sdram_tim3		= 0x407F8BA8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x5007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x0024400B,
	.emif_ddr_phy_ctlr_1	= 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1 = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

/* Ext phy ctrl regs 1-35 */
static const u32 beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs[] = {
	0x10040100,
	0x00910091,
	0x00950095,
	0x009B009B,
	0x009E009E,
	0x00980098,
	0x00340034,
	0x00350035,
	0x00340034,
	0x00310031,
	0x00340034,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x00480048,
	0x004A004A,
	0x00520052,
	0x00550055,
	0x00500050,
	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

static const struct emif_regs beagle_x15_emif2_ddr3_400mhz_emif_regs = {
	.sdram_config_init	= 0x61840BB2,
	.sdram_config		= 0x61840BB2,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000030D4,
	.ref_ctrl_final		= 0x00000C30,
	.sdram_tim1		= 0xCAAAE523,
	.sdram_tim2		= 0x208F7FDA,
	.sdram_tim3		= 0x407F88B8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x5007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x00244009,
	.emif_ddr_phy_ctlr_1	= 0x0E244009,
	.emif_ddr_ext_phy_ctrl_1 = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

static const struct emif_regs beagle_x15_emif2_ddr3_532mhz_emif_regs = {
	.sdram_config_init	= 0x61851BB2,
	.sdram_config		= 0x61851BB2,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000040F1,
	.ref_ctrl_final		= 0x00001035,
	.sdram_tim1		= 0xCCCF36B3,
	.sdram_tim2		= 0x30BF7FDA,
	.sdram_tim3		= 0x407F8BA8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x5007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x0024400B,
	.emif_ddr_phy_ctlr_1	= 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1 = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

static const u32 beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs[] = {
	0x10040100,
	0x00910091,
	0x00950095,
	0x009B009B,
	0x009E009E,
	0x00980098,
	0x00340034,
	0x00350035,
	0x00340034,
	0x00310031,
	0x00340034,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x00480048,
	0x004A004A,
	0x00520052,
	0x00550055,
	0x00500050,
	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	switch (emif_nr) {
	case 1:
		*regs = &beagle_x15_emif1_ddr3_532mhz_emif_regs;
		break;
	case 2:
		*regs = &beagle_x15_emif2_ddr3_532mhz_emif_regs;
		break;
	}
}

void emif_get_ext_phy_ctrl_const_regs(u32 emif_nr, const u32 **regs, u32 *size)
{
	switch (emif_nr) {
	case 1:
		*regs = beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs;
		*size = ARRAY_SIZE(beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs);
		break;
	case 2:
		*regs = beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs;
		*size = ARRAY_SIZE(beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs);
		break;
	}
}

struct vcores_data beagle_x15_volts = {
	.mpu.value		= VDD_MPU_DRA7,
	.mpu.efuse.reg		= STD_FUSE_OPP_VMIN_MPU,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value		= VDD_EVE_DRA7,
	.eve.efuse.reg		= STD_FUSE_OPP_VMIN_DSPEVE,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value		= VDD_GPU_DRA7,
	.gpu.efuse.reg		= STD_FUSE_OPP_VMIN_GPU,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS45,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value		= VDD_CORE_DRA7,
	.core.efuse.reg		= STD_FUSE_OPP_VMIN_CORE,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS6,
	.core.pmic		= &tps659038,

	.iva.value		= VDD_IVA_DRA7,
	.iva.efuse.reg		= STD_FUSE_OPP_VMIN_IVA,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS45,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data am571x_idk_volts = {
	.mpu.value		= VDD_MPU_DRA7,
	.mpu.efuse.reg		= STD_FUSE_OPP_VMIN_MPU,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask	= OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value		= VDD_EVE_DRA7,
	.eve.efuse.reg		= STD_FUSE_OPP_VMIN_DSPEVE,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value		= VDD_GPU_DRA7,
	.gpu.efuse.reg		= STD_FUSE_OPP_VMIN_GPU,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value		= VDD_CORE_DRA7,
	.core.efuse.reg		= STD_FUSE_OPP_VMIN_CORE,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value		= VDD_IVA_DRA7,
	.iva.efuse.reg		= STD_FUSE_OPP_VMIN_IVA,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS45,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data am572x_idk_volts = {
	.mpu.value		= VDD_MPU_DRA7,
	.mpu.efuse.reg		= STD_FUSE_OPP_VMIN_MPU,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value		= VDD_EVE_DRA7,
	.eve.efuse.reg		= STD_FUSE_OPP_VMIN_DSPEVE,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value		= VDD_GPU_DRA7,
	.gpu.efuse.reg		= STD_FUSE_OPP_VMIN_GPU,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value		= VDD_CORE_DRA7,
	.core.efuse.reg		= STD_FUSE_OPP_VMIN_CORE,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value		= VDD_IVA_DRA7,
	.iva.efuse.reg		= STD_FUSE_OPP_VMIN_IVA,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

#ifdef CONFIG_SPL_BUILD
/* No env to setup for SPL */
static inline void setup_board_eeprom_env(void) { }

/* Override function to read eeprom information */
void do_board_detect(void)
{
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);
}

#else	/* CONFIG_SPL_BUILD */

/* Override function to read eeprom information: actual i2c read done by SPL*/
void do_board_detect(void)
{

	char *bname = NULL;
	int rc;
	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	if (board_is_x15())
		bname = "BeagleBoard X15";
	else if (board_is_am572x_evm())
		bname = "AM572x LZC EVM";
	else if (board_is_am572x_idk())
		bname = "AM572x IDK";
	else if (board_is_am571x_idk())
		bname = "AM571x IDK";

	if (bname)
		snprintf(sysinfo.board_string, SYSINFO_BOARD_NAME_MAX_LEN,
			 "Board: %s REV %s\n", bname, board_ti_get_rev());
}

static void setup_board_eeprom_env(void)
{
	char *name = "beagle_x15";
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		return;

	if (board_is_x15()) {
		if (board_is_x15_revb1())
			name = "beagle_x15_revb1";
		else
			name = "beagle_x15";
	} else if (board_is_am572x_evm()) {
		if (board_is_am572x_evm_reva3())
			name = "am57xx_evm_reva3";
		else
			name = "am57xx_evm";
	} else if (board_is_am572x_idk()) {
		name = "am572x_idk";
	} else if (board_is_am571x_idk()) {
		name = "am571x_idk";
	} else {
		printf("Unidentified board claims %s in eeprom header\n",
		       board_ti_get_name());
	}

}

#endif	/* CONFIG_SPL_BUILD */

void vcores_update(void)
{
	if (board_is_am572x_idk())
		*omap_vcores = &am572x_idk_volts;
	else if (board_is_am571x_idk())
		*omap_vcores = &am571x_idk_volts;
}

void hw_data_init(void)
{
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra7xx_dplls;
	*omap_vcores = &beagle_x15_volts;
	*ctrl = &dra7xx_ctrl;
}

bool am571x_idk_needs_lcd(void)
{
	bool needs_lcd;

	gpio_request(GPIO_ETH_LCD, "nLCD_Detect");
	if (gpio_get_value(GPIO_ETH_LCD))
		needs_lcd = false;
	else
		needs_lcd = true;

	gpio_free(GPIO_ETH_LCD);

	return needs_lcd;
}

void am57xx_select_emmcbootpart(void)
{
	gpio_request(GPIO_EMMC_BOOTPART_SEL, "emmcbootpart_sel");
	if (gpio_get_value(GPIO_EMMC_BOOTPART_SEL)) {
		setenv("emmcbootpart", "1:2");
	}
	else {
		setenv("emmcbootpart", "1:3");
	}

	gpio_free(GPIO_EMMC_BOOTPART_SEL);
}

int board_init(void)
{
	gpmc_init();
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
static u64 mac_to_u64(u8 mac[6])
{
	int i;
	u64 addr = 0;

	for (i = 0; i < 6; i++) {
		addr <<= 8;
		addr |= mac[i];
	}

	return addr;
}

static void u64_to_mac(u64 addr, u8 mac[6])
{
	mac[5] = addr;
	mac[4] = addr >> 8;
	mac[3] = addr >> 16;
	mac[2] = addr >> 24;
	mac[1] = addr >> 32;
	mac[0] = addr >> 40;
}

void board_set_ethaddr(void)
{
	uint8_t mac_addr[6];
	int i;
	u64 mac1, mac2;
	u8 mac_addr1[6], mac_addr2[6];
	int num_macs;
	/*
	 * Export any Ethernet MAC addresses from EEPROM.
	 * On AM57xx the 2 MAC addresses define the address range
	 */
	board_ti_get_eth_mac_addr(0, mac_addr1);
	board_ti_get_eth_mac_addr(1, mac_addr2);

	if (is_valid_ethaddr(mac_addr1) && is_valid_ethaddr(mac_addr2)) {
		mac1 = mac_to_u64(mac_addr1);
		mac2 = mac_to_u64(mac_addr2);

		/* must contain an address range */
		num_macs = mac2 - mac1 + 1;
		/* <= 50 to protect against user programming error */
		if (num_macs > 0 && num_macs <= 50) {
			for (i = 0; i < num_macs; i++) {
				u64_to_mac(mac1 + i, mac_addr);
				if (is_valid_ethaddr(mac_addr)) {
					eth_setenv_enetaddr_by_index("eth",
								     i + 2,
								     mac_addr);
				}
			}
		}
	}
}
#endif

#if defined(ENABLE_LOGO_DISPLAY)

#define LCD_BACKLIGHT_PINMUX_ADDR            0x4A003500 // GPIO3_7
static void enable_lcd_backlight(void)
{
	writel((M14 | PIN_OUTPUT_PULLUP), LCD_BACKLIGHT_PINMUX_ADDR);
}

static void mmc_load_logo(void)
{
	struct mmc *mmc;
	u32 logo_len, file_len;
	char *logo_file;
	char dev_index[2];
	int ret;

#ifdef CONFIG_LOGO_USE_COMPRESSION
	size_t decomp_len;
#endif

	mmc = find_mmc_device(LOGO_FILE_STORE);
	if (!mmc) {
		printf("no mmc device at slot %d\n", LOGO_FILE_STORE);
		return;
	}

	if (mmc_init(mmc))
		printf("mmc%d device init failed!\n", LOGO_FILE_STORE);

	sprintf(dev_index, "%d", LOGO_FILE_STORE);
	fs_set_blk_dev("mmc", dev_index, FS_TYPE_FAT);

	invalidate_dcache_range(DRA7_LOGO_START,
				DRA7_LOGO_START + DRA7_LOGO_MAX_SIZE +
				DRA7_LOGO_MAX_COMP_SIZE);

	logo_file = getenv("logofile");

	/* set the default logo file */
	if (!logo_file) {
		logo_file = "logo.yuv";
		printf("env logofile not set!\n");
		printf("set logofile to logo.yuv!\n");
	}

#ifdef CONFIG_LOGO_USE_COMPRESSION
	ret = fs_read(logo_file, DRA7_LOGO_COMP_START, 0, 0, &file_len);
	decomp_len = DRA7_LOGO_MAX_SIZE;
	lzop_decompress((u8 *)DRA7_LOGO_COMP_START, file_len,
					(u8 *)DRA7_LOGO_START, &decomp_len);
	logo_len = decomp_len;
#else
	ret = fs_read(logo_file, DRA7_LOGO_START, 0, 0, &file_len);
	logo_len =  file_len;
#endif

	flush_cache(DRA7_LOGO_START, logo_len);
}

extern void display_logo(void);

#endif

FILEINFO strInitSpiFile[] =
{
	{INDEX_APP_QT, "QT", "qt_text",  0x100000},

};
void UpdateFileUsb(int nIndex, const char *strInfo, const char *strName, unsigned long uSize)
{
	int ret = 1;
	char strCmd[100] = {0};
	char strRes[100] = {0};
	//判断这个文件是否存在		
	sprintf(strCmd, "fatsize usb %s %s/%s", usb_part,UPDATE_PATH_FOLDER, strName);
	ret=run_command(strCmd,0); 
	printf("ret=%d\n",ret);

	if(ret == 0)
	{
		////从U盘加载文件
		sprintf(strCmd, "fatload usb %s %lx %s/%s", usb_part, FILE_SAVE_DDR_ADDR, UPDATE_PATH_FOLDER, strName);
		run_command(strCmd,0);

		//写入mmc
		//ext4write mmc 1:2 0xc2000000  /333.txt 0xa
		sprintf(strCmd, "ext4write mmc 1:2  %lx /%s %lx",FILE_SAVE_DDR_ADDR, strName, uSize);
		run_command(strCmd,0);
  
		sprintf(strRes, "%s existent \n", strName);
		puts(strRes);
	}
	else
	{
		sprintf(strRes, "%s non-existent \n", strName);
		puts(strRes);
	}
	

}

int UpdateSystemFromUSB()
{
	char strCmd[100] = {0};
	sprintf(usb_part, "0:");
	
	run_command("usb start", 0);
	run_command("usb part 0",0);

	if (0 == my_do_fat_ls()) //判断是否有u盘插入
	{
		int i = 0;
		sprintf(strCmd, "fatls usb %s %s", usb_part,UPDATE_PATH_FOLDER);
		run_command(strCmd, 0);
		for (i = 0; i < sizeof(strInitSpiFile) / sizeof(FILEINFO); i++)
		{
			UpdateFileUsb(strInitSpiFile[i].nIndex, strInitSpiFile[i].fileInfo, strInitSpiFile[i].fileName, strInitSpiFile[i].ufileSize);		
		}
		return 0;
	}
	return -1;
}

int board_late_init(void)
{
	char *idk_lcd;
	u32 reg_val;

	setup_board_eeprom_env();

#if defined(ENABLE_LOGO_DISPLAY)
	mmc_load_logo();
	display_logo();
	mdelay(50);
	enable_lcd_backlight();
#endif

	/*
	 * DEV_CTRL.DEV_ON = 1 please - else palmas switches off in 8 seconds
	 * This is the POWERHOLD-in-Low behavior.
	 */
	palmas_i2c_write_u8(TPS65903X_CHIP_P1, 0xA0, 0x1);

	/* TBD: Add LCD panel detection once information is available */
	if (am571x_idk_needs_lcd())
		idk_lcd = "osd101t2045"; /* Default to legacy LCD */
	else
		idk_lcd = "no";
	setenv("idk_lcd", idk_lcd);

#if !defined(CONFIG_SPL_BUILD)
	board_set_ethaddr();

	am57xx_select_emmcbootpart();


	#ifdef ENABLE_USB_UPDATA
	if(UpdateSystemFromUSB()==0)
	{
		printf("update success\n");
	}
	else
	{
		printf("update fail\n");
	}
	#endif
	

	/* TL5728 som led2 */
	reg_val = __raw_readl(AM572X_GPIO7_BASE + AM572X_GPIO_OE_OFF);
	reg_val &= ~(BIT(9));
	__raw_writel(reg_val, AM572X_GPIO7_BASE + AM572X_GPIO_OE_OFF);

	reg_val = __raw_readl(AM572X_GPIO7_BASE + AM572X_GPIO_SETDATA_OUT_OFF);
	reg_val|= BIT(9);
	__raw_writel(reg_val, AM572X_GPIO7_BASE + AM572X_GPIO_SETDATA_OUT_OFF);

#endif

	

	return 0;
}

void set_muxconf_regs(void)
{
	do_set_mux32((*ctrl)->control_padconf_core_base,
		     early_padconf, ARRAY_SIZE(early_padconf));
}

#ifdef CONFIG_IODELAY_RECALIBRATION
void recalibrate_iodelay(void)
{
	const struct pad_conf_entry *pconf;
	const struct iodelay_cfg_entry *iod;
	int pconf_sz, iod_sz;
	int ret;

	if (board_is_am572x_idk()) {
		pconf = core_padconf_array_essential_am572x_idk;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_am572x_idk);
		iod = iodelay_cfg_array_am572x_idk;
		iod_sz = ARRAY_SIZE(iodelay_cfg_array_am572x_idk);
	} else if (board_is_am571x_idk()) {
		pconf = core_padconf_array_essential_am571x_idk;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_am571x_idk);
		iod = iodelay_cfg_array_am571x_idk;
		iod_sz = ARRAY_SIZE(iodelay_cfg_array_am571x_idk);
	} else {
		/* Common for X15/GPEVM */
		pconf = core_padconf_array_essential_x15;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_x15);
		/* There never was an SR1.0 X15.. So.. */
		if (omap_revision() == DRA752_ES1_1) {
			iod = iodelay_cfg_array_x15_sr1_1;
			iod_sz = ARRAY_SIZE(iodelay_cfg_array_x15_sr1_1);
		} else {
			/* Since full production should switch to SR2.0  */
			iod = iodelay_cfg_array_x15_sr2_0;
			iod_sz = ARRAY_SIZE(iodelay_cfg_array_x15_sr2_0);
		}
	}

	/* Setup I/O isolation */
	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* Do the muxing here */
	do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);

	/* Now do the weird minor deltas that should be safe */
	if (board_is_x15() || board_is_am572x_evm()) {
		if (board_is_x15_revb1() || board_is_am572x_evm_reva3()) {
			pconf = core_padconf_array_delta_x15_sr2_0;
			pconf_sz = ARRAY_SIZE(core_padconf_array_delta_x15_sr2_0);
		} else {
			pconf = core_padconf_array_delta_x15_sr1_1;
			pconf_sz = ARRAY_SIZE(core_padconf_array_delta_x15_sr1_1);
		}
		do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);
	}

	if (board_is_am571x_idk()) {
		if (am571x_idk_needs_lcd()) {
			pconf = core_padconf_array_vout_am571x_idk;
			pconf_sz = ARRAY_SIZE(core_padconf_array_vout_am571x_idk);
		} else {
			pconf = core_padconf_array_icss1eth_am571x_idk;
			pconf_sz = ARRAY_SIZE(core_padconf_array_icss1eth_am571x_idk);
		}
		do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);
	}

	/* Setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iod, iod_sz);
err:
	/* Closeup.. remove isolation */
	__recalibrate_iodelay_end(ret);
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#ifdef CONFIG_OMAP_HSMMC
int platform_fixup_disable_uhs_mode(void)
{
	return omap_revision() == DRA752_ES1_1;
}
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	u32 reg_val;

	/* TL5728 som led1 */
	reg_val = __raw_readl(AM572X_GPIO7_BASE + AM572X_GPIO_OE_OFF);
	reg_val &= ~(BIT(7));
	__raw_writel(reg_val, AM572X_GPIO7_BASE + AM572X_GPIO_OE_OFF);

	reg_val = __raw_readl(AM572X_GPIO7_BASE + AM572X_GPIO_SETDATA_OUT_OFF);
	reg_val|= BIT(7);
	__raw_writel(reg_val, AM572X_GPIO7_BASE + AM572X_GPIO_SETDATA_OUT_OFF);

	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_relocate_spec();
	if (getenv_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3
static struct dwc3_device usb_otg_ss1 = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = DRA7_USB_OTG_SS1_BASE,
	.tx_fifo_resize = false,
	.index = 0,
};

static struct dwc3_omap_device usb_otg_ss1_glue = {
	.base = (void *)DRA7_USB_OTG_SS1_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 0,
};

static struct ti_usb_phy_device usb_phy1_device = {
	.pll_ctrl_base = (void *)DRA7_USB3_PHY1_PLL_CTRL,
	.usb2_phy_power = (void *)DRA7_USB2_PHY1_POWER,
	.usb3_phy_power = (void *)DRA7_USB3_PHY1_POWER,
	.index = 0,
};

static struct dwc3_device usb_otg_ss2 = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = DRA7_USB_OTG_SS2_BASE,
	.tx_fifo_resize = false,
	.index = 1,
};

static struct dwc3_omap_device usb_otg_ss2_glue = {
	.base = (void *)DRA7_USB_OTG_SS2_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 1,
};

static struct ti_usb_phy_device usb_phy2_device = {
	.usb2_phy_power = (void *)DRA7_USB2_PHY2_POWER,
	.index = 1,
};

int usb_gadget_handle_interrupts(int index)
{
	u32 status;

	status = dwc3_omap_uboot_interrupt_status(index);
	if (status)
		dwc3_uboot_handle_interrupt(index);

	return 0;
}
#endif /* CONFIG_USB_DWC3 */

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_OMAP)
int board_usb_init(int index, enum usb_init_type init)
{
	enable_usb_clocks(index);
	switch (index) {
	case 0:
		if (init == USB_INIT_DEVICE) {
			printf("port %d can't be used as device\n", index);
			disable_usb_clocks(index);
			return -EINVAL;
		}
		break;
	case 1:
		if (init == USB_INIT_DEVICE) {
#ifdef CONFIG_USB_DWC3
			usb_otg_ss2.dr_mode = USB_DR_MODE_PERIPHERAL;
			usb_otg_ss2_glue.vbus_id_status = OMAP_DWC3_VBUS_VALID;
			ti_usb_phy_uboot_init(&usb_phy2_device);
			dwc3_omap_uboot_init(&usb_otg_ss2_glue);
			dwc3_uboot_init(&usb_otg_ss2);
#endif
		} else {
			printf("port %d can't be used as host\n", index);
			disable_usb_clocks(index);
			return -EINVAL;
		}

		break;
	default:
		printf("Invalid Controller Index\n");
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
#ifdef CONFIG_USB_DWC3
	switch (index) {
	case 0:
	case 1:
		if (init == USB_INIT_DEVICE) {
			ti_usb_phy_uboot_exit(index);
			dwc3_uboot_exit(index);
			dwc3_omap_uboot_exit(index);
		}
		break;
	default:
		printf("Invalid Controller Index\n");
	}
#endif
	disable_usb_clocks(index);
	return 0;
}
#endif /* defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_OMAP) */

#ifdef CONFIG_DRIVER_TI_CPSW

/* Delay value to add to calibrated value */
#define RGMII0_TXCTL_DLY_VAL		((0x3 << 5) + 0x8)
#define RGMII0_TXD0_DLY_VAL		((0x3 << 5) + 0x8)
#define RGMII0_TXD1_DLY_VAL		((0x3 << 5) + 0x2)
#define RGMII0_TXD2_DLY_VAL		((0x4 << 5) + 0x0)
#define RGMII0_TXD3_DLY_VAL		((0x4 << 5) + 0x0)
#define VIN2A_D13_DLY_VAL		((0x3 << 5) + 0x8)
#define VIN2A_D17_DLY_VAL		((0x3 << 5) + 0x8)
#define VIN2A_D16_DLY_VAL		((0x3 << 5) + 0x2)
#define VIN2A_D15_DLY_VAL		((0x4 << 5) + 0x0)
#define VIN2A_D14_DLY_VAL		((0x4 << 5) + 0x0)

static void cpsw_control(int enabled)
{
	/* VTP can be added here */
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	int ret;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	uint32_t ctrl_val;

	/* try reading mac address from efuse */
	mac_lo = readl((*ctrl)->control_core_mac_id_0_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_0_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

	mac_lo = readl((*ctrl)->control_core_mac_id_1_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_1_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!getenv("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("eth1addr", mac_addr);
	}

	ctrl_val = readl((*ctrl)->control_core_control_io1) & (~0x33);
	ctrl_val |= 0x22;
	writel(ctrl_val, (*ctrl)->control_core_control_io1);

	/* The phy address for the AM57xx IDK are different than x15 */
	if (board_is_am572x_idk() || board_is_am571x_idk()) {
		cpsw_data.slave_data[0].phy_addr = 0;
		cpsw_data.slave_data[1].phy_addr = 1;
	}

	ret = cpsw_register(&cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	return ret;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
/* VTT regulator enable */
static inline void vtt_regulator_enable(void)
{
	if (omap_hw_init_context() == OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL)
		return;

	gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
	gpio_direction_output(GPIO_DDR_VTT_EN, 1);
}

int board_early_init_f(void)
{
	vtt_regulator_enable();
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (board_is_x15() && !strcmp(name, "am57xx-beagle-x15"))
		return 0;
	else if (board_is_am572x_evm() && !strcmp(name, "am57xx-beagle-x15"))
		return 0;
	else if (board_is_am572x_idk() && !strcmp(name, "am572x-idk"))
		return 0;
	else if (board_is_am571x_idk() && !strcmp(name, "am571x-idk"))
		return 0;
	else
		return -1;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif
