#!/bin/sh
cd src
make CROSS_COMPILE=/opt/tools/gcc5.3/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- ARCH=arm am57xx_evm_defconfig 

make CROSS_COMPILE=/opt/tools/gcc5.3/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- ARCH=arm menuconfig 
make CROSS_COMPILE=/opt/tools/gcc5.3/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- ARCH=arm all -j2
