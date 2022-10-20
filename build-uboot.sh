#!/bin/bash

./make.sh rk3588s_cp4b 
if [ "$?" == "0" ]; then
    rm -rf out
    mkdir -p out
    cp -f rk3588_spl_loader_*.bin out/loader.bin
    cp uboot.img out/
    echo "Compile U-boot OK!"
    md5sum out/*
fi
