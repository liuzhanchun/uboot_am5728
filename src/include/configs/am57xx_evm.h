/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated.
 * Felipe Balbi <balbi@ti.com>
 *
 * Configuration settings for the TI Beagle x15 board.
 * See ti_omap5_common.h for omap5 common settings.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_AM57XX_EVM_H
#define __CONFIG_AM57XX_EVM_H

#define CONFIG_AM57XX

#define CONFIG_IODELAY_RECALIBRATION

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_MAX_RAM_BANK_SIZE	(2 << 30)       /* 2GB */

#define CONFIG_ENV_SIZE			(64 << 10)
#define CONFIG_ENV_IS_IN_FAT
#define FAT_ENV_INTERFACE		"mmc"
#define FAT_ENV_DEVICE_AND_PART		"1:1"
#define FAT_ENV_FILE			"uboot.env"

#define CONSOLEDEV			"ttyO2"
#define CONFIG_SYS_NS16550_COM1		UART1_BASE	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		UART2_BASE	/* UART2 */
#define CONFIG_SYS_NS16550_COM3		UART3_BASE	/* UART3 */
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

/* SPL */
#define CONFIG_SPL_YMODEM_SUPPORT

/* Define the default GPT table for eMMC */
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=rootfs,start=2MiB,size=-,uuid=${uuid_gpt_rootfs}"

#include <configs/ti_omap5_common.h>

/* Enhance our eMMC support / experience. */
#define CONFIG_CMD_GPT
#define CONFIG_EFI_PARTITION

/* CPSW Ethernet */
#define CONFIG_BOOTP_DNS		/* Configurable parts of CMD_DHCP */
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_DRIVER_TI_CPSW		/* Driver for IP block */
#define CONFIG_MII			/* Required in net/eth.c */
#define CONFIG_PHY_GIGE			/* per-board part of CPSW */
#define CONFIG_PHYLIB
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs longer aneg time at 1G */

#define CONFIG_SUPPORT_EMMC_BOOT

/* USB xHCI HOST */
#define CONFIG_USB_HOST
#define CONFIG_USB_XHCI_DWC3
#define CONFIG_USB_XHCI
#define CONFIG_USB_XHCI_OMAP
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2

#define CONFIG_OMAP_USB_PHY
#define CONFIG_OMAP_USB3PHY1_HOST

/* SATA */
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_CMD_SCSI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

/* EEPROM */
#define CONFIG_EEPROM_CHIP_ADDRESS 0x50
#define CONFIG_EEPROM_BUS_ADDRESS 0

/* enable setexpr command */
#define CONFIG_CMD_SETEXPR

#define MMC0_DEV_INDEX                0
#define MMC1_DEV_INDEX                1
#define LOGO_FILE_STORE               MMC1_DEV_INDEX

/* logo display is disable by default */
/* #define ENABLE_LOGO_DISPLAY           1 */
#define DRA7_OCMC1_BASE               (0x40400000)
#define DRA7_OCMC1_SIZE               (0x00100000)
#define DRA7_OCMC2_BASE               (0x40500000)
#define DRA7_OCMC2_SIZE               (0x00100000)
#define DRA7_LOGO_MAX_COMP_SIZE       (512*1024)
#define DRA7_LOGO_START               (DRA7_OCMC1_BASE)
#define DRA7_LOGO_MAX_SIZE            (DRA7_OCMC1_SIZE + DRA7_OCMC2_SIZE - DRA7_LOGO_MAX_COMP_SIZE)
#define DRA7_LOGO_COMP_START          (DRA7_OCMC1_BASE+DRA7_LOGO_MAX_SIZE)

/* #define CONFIG_LOGO_USE_COMPRESSION */
#ifdef CONFIG_LOGO_USE_COMPRESSION
#define CONFIG_LZO
#endif

#endif /* __CONFIG_AM57XX_EVM_H */
