/*
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __EVB_RV1106_H
#define __EVB_RV1106_H

#include <configs/rv1106_common.h>

#ifndef CONFIG_SPL_BUILD

#define CONFIG_SYS_MMC_ENV_DEV 0

#define ROCKCHIP_DEVICE_SETTINGS \
			"stdout=serial\0" \
			"stderr=serial\0"
#undef CONFIG_CONSOLE_SCROLL_LINES
#define CONFIG_CONSOLE_SCROLL_LINES            10

#define CONFIG_ENV_OVERWRITE    1

#undef CONFIG_BOOTCOMMAND

#include <config_distro_defaults.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \ 
	func(MMC, mmc, 0) 

#include <config_distro_bootcmd.h>

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS       \
	"fdt_addr_r=0x00c00000\0" \
	"loadaddr=0x00800000\0" \
	"initrd_addr=0x00e00000\0" \
	"bootdir=/boot/\0" \
	"image=vmlinuz\0" \
	"console=ttyS0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=rv1106-cpnano.dtb\0" \
	"rd_file=initrd.img\0" \
	ENV_MEM_LAYOUT_SETTINGS         \
	ROCKCHIP_DEVICE_SETTINGS        \
	BOOTENV

/*
 * We made a deal: Not allow U-Boot to bring up thunder-boot kernel.
 *
 * Because the thunder-boot feature may require special memory layout
 * or other appointments, U-Boot can't handle all that. Let's go back
 * to SPL to bring up kernel.
 *
 * Note: bootcmd is only called in normal boot sequence, that means
 * we allow user to boot what they want in U-Boot shell mode.
 */
#ifdef CONFIG_SPL_KERNEL_BOOT
#define CONFIG_BOOTCOMMAND "reset"
#else
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "loadver;reset"
#endif

#endif /* !CONFIG_SPL_BUILD */
#endif /* __EVB_RV1106_H */
