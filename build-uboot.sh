#!/bin/bash

BOARD=$1
RV1106="0"

case "$BOARD" in
  cp4b)
    cfg="rk3588s_coolpi4b" 
    ;;
  cm5)
    cfg="rk3588_coolpicm5" 
    ;;
  nano)
    cfg="rv1106_coolpinano"
    RV1106="1"
    ;;
  *)
    echo "Usage: $0 {cp4b|cm5|nano}" >&2
    exit 0
    ;;
esac

if [ $RV1106 == "1" ]; then
    U_SRC=`pwd`
    ARCH=`uname -m`
    if [ "$ARCH" == "x86_64" ]; then
        export CROSS_COMPILE=arm-rockchip830-linux-uclibcgnueabihf-
        TOOLCHAIN_ARM32=$U_SRC/toolchain32uc/bin
        export PATH=$TOOLCHAIN_ARM32:$PATH
    fi

    ./make.sh $cfg CROSS_COMPILE=arm-rockchip830-linux-uclibcgnueabihf- --spl-new
    if [ "$?" == "0" ]; then
        rm -rf ${cfg}_out
        mkdir -p ${cfg}_out
        cp -f rv1106_download*.bin ${cfg}_out/download.bin
        cp -f rv1106_idblock* ${cfg}_out/idblock.img
        cp uboot.img ${cfg}_out/
        cp nano-package-file ${cfg}_out/package-file
        ./tools/mkenvimage-nano -s 0x10000 -p 0x0 -o ${cfg}_out/env.img $U_SRC/nano-env.txt
        ./tools/afptool -pack ${cfg}_out ${cfg}_out/update_tmp.img || pause
        ./tools/rkImageMaker -RK1106 ${cfg}_out/download.bin ${cfg}_out/update_tmp.img ${cfg}_out/coolpi-nano.img -os_type:androidos || pause
        rm ${cfg}_out/update_tmp.img
        echo "Compile U-boot NANO OK!"
        md5sum ${cfg}_out/*
    fi
else
    ./make.sh $cfg
    if [ "$?" == "0" ]; then
        rm -rf ${cfg}_out
        mkdir -p ${cfg}_out
        cp -f rk3588_spl_loader_*.bin ${cfg}_out/loader.bin
        cp uboot.img ${cfg}_out/
        echo "Compile U-boot OK!"
        md5sum ${cfg}_out/*
    fi
fi
