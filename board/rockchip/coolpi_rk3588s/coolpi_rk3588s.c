/*
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dwc3-uboot.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_GPIO_HUB_RST     141
#define CONFIG_GPIO_DIR_SWITCH  59

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xfc000000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.usb2_phyif_utmi_width = 16,
};

int usb_gadget_handle_interrupts(void)
{
	//dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
	//return dwc3_uboot_init(&dwc3_device_data);
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

void fes_hub_rst(void)
{
        int_gpio_set_level(CONFIG_GPIO_HUB_RST, "usb_hub_rst", 0);
        mdelay(20);
        int_gpio_set_level(CONFIG_GPIO_HUB_RST, "usb_hub_rst", 1);

        return ;
}

void dir_switch_sdio(void)
{
        int_gpio_set_level(CONFIG_GPIO_DIR_SWITCH, "switch_sdio", 1);

        return ;
}
