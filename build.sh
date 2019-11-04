#!/bin/sh
cd src
make CROSS_COMPILE=/opt/tools/gcc5.3/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- ARCH=arm am57xx_evm_defconfig all -j2
#make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm all -j2
