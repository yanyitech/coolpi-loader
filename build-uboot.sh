#!/bin/bash

./make.sh coolpi-4b-rk3588s 
if [ "$?" == "0" ]; then
    rm -rf out
    mkdir -p out
    cp -f rk3588_spl_loader_*.bin out/loader.bin
    cp uboot.img out/
    echo "Compile U-boot OK!"
    md5sum out/*
fi
