#!/bin/bash

TOOLCHAIN_PATH=$1
BOARD_PATH=$2
CPU_TYPE=$3
COMPILE_MODE=$4
TOOLCHAIN_PREFIX=$6

BSP_NAME="${CPU_TYPE}Bsp"
echo "##################### build $BSP_NAME start #####################"

copy_headers() {
    mkdir -p "$2"
    # cp -af "$1"/*.h "$2" 2>/dev/null || true
    cp -af "$1"/*.h "$2"
}

copy_directories() {
    mkdir -p "$2"
    cp -af "$1"/* "$2" 2>/dev/null || true
}

copy_headers "../config" "../include"
copy_headers "../src" "../include/src"
copy_headers "../apps" "../include/apps"
copy_headers "../../common/hal" "../include/hal"
copy_headers "../../common/rk3568/hal" "../include/hal"
copy_headers "../../common/rk3568/drv" "../include/drv"
copy_headers "../../../../components/rtdriver/common/include" "../include/rtdriver"
copy_directories "../../../../components/rtdriver/drivers/include" "../include/rtdriver"
copy_directories "../../../../components/mica/include" "../include"

cmake -S .. -B $BOARD_PATH \
      -DBSP_LIB:STRING=$BSP_NAME \
      -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH \
      -DCPU_TYPE:STRING=$CPU_TYPE \
      -DCOMPILE_MODE:STRING=$COMPILE_MODE \
      -DTOOLCHAIN_PREFIX:STRING=$TOOLCHAIN_PREFIX

pushd $BOARD_PATH
make $BSP_NAME install
if [ $? -ne 0 ]; then
    echo "make $BSP_NAME failed!"
    exit 1
fi
popd

rm -rf $BOARD_PATH

# 输出结束信息
echo "##################### build $BSP_NAME end #####################"
