#!/bin/bash

BOARD=$1

case "$BOARD" in
  cp4b)
    cfg="rk3588s_coolpi4b" 
    ;;
  cpcm5)
    cfg="rk3588_coolpicm5" 
    ;;
  *)
    echo "Usage: $0 {cp4b|cpcm5}" >&2
    exit 0
    ;;
esac

./make.sh $cfg 
if [ "$?" == "0" ]; then
    rm -rf ${cfg}_out
    mkdir -p ${cfg}_out
    cp -f rk3588_spl_loader_*.bin ${cfg}_out/loader.bin
    cp uboot.img ${cfg}_out/
    echo "Compile U-boot OK!"
    md5sum ${cfg}_out/*
fi
