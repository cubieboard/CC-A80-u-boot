#!/bin/bash
set -e
make distclean CROSS_COMPILE=arm-linux-gnueabi- && make -j8 sun9iw1p1 CROSS_COMPILE=arm-linux-gnueabi-
make CROSS_COMPILE=arm-linux-gnueabi- -j4 fes  boot0 sboot

