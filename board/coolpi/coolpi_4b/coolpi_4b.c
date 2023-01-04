/*
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/gpio.h>
#include <dwc3-uboot.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

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
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif

#define GPIO4_B5 141  /*usb hub rst*/
#define GPIO1_D3 59   /*sdio/spi-nor dir switch*/

void fes_hub_rst(void)
{
	int ret;

	ret = gpio_request(GPIO4_B5, "usb_hub_rst");
	if (ret) {
		printf("request for usb_hub_rst gpio failed:%d\n", ret);
		return;
	}

	ret = gpio_direction_output(GPIO4_B5, 0);
	mdelay(20);
	ret = gpio_direction_output(GPIO4_B5, 1);

}

int rk_board_late_init(void)
{
	int ret;

	ret = gpio_request(GPIO1_D3, "spi_sdio_switch");
	if (ret) {
		printf("request for spi_sdio_switch gpio failed:%d\n", ret);
		return ret;
	}

	/*
	 * switch to sdio mode, so the sdio wifi can work
	 * at when linux kernel boot.
	 */
	ret = gpio_direction_output(GPIO1_D3, 1);

	return 0;
}
