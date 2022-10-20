/*
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Copyright (c) 2020 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIGS_RK3588_EVB_H
#define __CONFIGS_RK3588_EVB_H

#include <configs/rk3588_common.h>

#ifndef CONFIG_SPL_BUILD

#define CONFIG_ENV_OVERWRITE	1

#undef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS \
		"stdin=serial\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
        "fdt_addr_r=0x17000000\0" \
        "loadaddr_=0x12000000\0" \
        "loadaddr=0x10000000\0" \
        "initrd_addr=0x18000000\0" \
        "bootdir=/boot/\0" \
        "image=vmlinuz\0" \
        "console=ttyS0\0" \
        "fdt_high=0xffffffff\0" \
        "initrd_high=0xffffffff\0" \
        "fdt_file=rk3588s.dtb\0" \
        "rd_file=initrd.img\0" \
	ROCKCHIP_DEVICE_SETTINGS

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
          "loadver;reset" 
#endif
#endif
