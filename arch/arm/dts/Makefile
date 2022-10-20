#
# SPDX-License-Identifier:	GPL-2.0+
#

dtb-$(CONFIG_ARCH_ROCKCHIP) += \
	rk3588s-cp4.dtb

# Compile embeded kernel dts to dtb, and put it behind of u-boot.dtb
EMBED_KERN_DTB_PATH := $(CONFIG_EMBED_KERNEL_DTB_PATH:"%"=%)
EMBED_KERN_DTS_PATH := $(subst dtb,dts, $(EMBED_KERN_DTB_PATH))
ifneq ($(wildcard $(EMBED_KERN_DTS_PATH)),)
EMBED_KERN_DTB=$(shell echo $(EMBED_KERN_DTB_PATH) | awk -F '/' '{ print $$NF }')
dtb-y += $(EMBED_KERN_DTB)
endif

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
