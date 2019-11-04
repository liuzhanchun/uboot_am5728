#!/bin/sh
cd src
make CROSS_COMPILE=/opt/tools/gcc5.3/arm-linux-gnueabihf/bin/arm-linux-gnueabihf- ARCH=arm distclean 
