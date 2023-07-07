/*
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/io.h>
#include <dwc3-uboot.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_GPIO_DIR_EMMC  145

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xffb00000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.usb2_phyif_utmi_width = 16,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	writel(0x01ff0000, 0xff000050); /* Resume usb2 phy to normal mode */
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif

static int int_gpio_set_level(int gpio, char *desc, int val)
{
        int ret = 0;

        ret = gpio_request(gpio, desc);
        if (ret < 0) {
                printf("request for %d failed:%d\n", gpio, ret);
                return -1;
        }
        gpio_direction_output(gpio, val);
        gpio_free(gpio);

        return 0;
}

void dir_switch_emmc(void)
{
#define GPIO4_IOC_BASE                  0xFF568000
#define GPIO4A_IOMUX_SEL_L              0x000
#define GPIO4A_IOMUX_SEL_H              0x004
#define GPIO4B_IOMUX_SEL_L              0x008

        //int_gpio_set_level(CONFIG_GPIO_DIR_EMMC, "switch_emmc", 1);
#if 1
	/* emmc iomux */
	writel(0xffff1111, GPIO4_IOC_BASE + GPIO4A_IOMUX_SEL_L);
	writel(0xffff1111, GPIO4_IOC_BASE + GPIO4A_IOMUX_SEL_H);
	writel(0x00ff0011, GPIO4_IOC_BASE + GPIO4B_IOMUX_SEL_L);
#endif

        return ;
}

#ifndef CONFIG_SPL_BUILD
static int do_load_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        uint32_t vlen = 0;
        char cmd_mod[128] = {0};

        //dir_switch_emmc();
        //run_command("mmc dev 0;", -1);
        run_command("run distro_bootcmd;", -1);
        printf("Loading order: tf - emmc\n");
        run_command("checkconf tf;", -1);
        if(!run_command("load mmc 1:1 ${loadaddr} ${bootdir}${image}", 0)) {
                run_command("load mmc 1:1 ${initrd_addr} ${bootdir}${rd_file}", -1);
                vlen = simple_strtoul(env_get("filesize"), NULL, 16);
                run_command("load mmc 1:1 ${fdt_addr_r} ${bootdir}${fdt_file}", -1);
                sprintf(cmd_mod, "bootz ${loadaddr} ${initrd_addr}:%x ${fdt_addr_r}", vlen);
                run_command(cmd_mod, -1);
                return 0;
        }
        run_command("checkconf mmc;", -1);
        if(!run_command("load mmc 0:1 ${loadaddr} ${bootdir}${image}", 0)) {
                run_command("load mmc 0:1 ${initrd_addr} ${bootdir}${rd_file}", -1);
                vlen = simple_strtoul(env_get("filesize"), NULL, 16);
                run_command("load mmc 0:1 ${fdt_addr_r} ${bootdir}${fdt_file}", -1);
                sprintf(cmd_mod, "bootz ${loadaddr} ${initrd_addr}:%x ${fdt_addr_r}", vlen);
                run_command(cmd_mod, -1);
                return 0;
        }

        mdelay(1000);

        return 0;
}

U_BOOT_CMD(loadver, 4, 0, do_load_version,
           "Load version",
           "Booting\n"
);

void init_board_env(void)
{
        char *temp = NULL;

	run_command("c read;", -1);

        env_set("board_name", "CoolPi Nano RV1106");

	temp = env_get("fixmac");
	if(temp) {
		if(strncmp(env_get("fixmac"), "yes", 3)) {
			env_set("fixmac", "yes");
			run_command("c write;", -1);
		}
	} else {
		env_set("fixmac", "yes");
		run_command("c write;", -1);
	}
	run_command("c read;", -1);
        env_set("board", "coolpi");
        env_set("soc", "rv1106");
}
#endif
