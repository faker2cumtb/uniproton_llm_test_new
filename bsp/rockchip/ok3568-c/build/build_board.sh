#!/bin/bash

# 检查参数数量
if [ "$#" -ne 6 ]; then
    echo "ERROR: Compile mode is not specified for ($0)!" >&2
    exit 1
fi

# 设置环境变量
export HOME_PATH=../../../..

# 解析输入参数
TOOLCHAIN_PATH=$1
BOARD_PATH=$2
CPU_TYPE=$3
COMPILE_MODE=$4
APP_NAME=$5
TOOLCHAIN_PREFIX=$6

############################################################################
# 执行各个构建脚本
echo "Running build scripts..."

sh "./build_static.sh" "$BOARD_PATH" "$CPU_TYPE" "$COMPILE_MODE" || {
    echo "ERROR: build_static.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/metal/build_metal.sh" "$COMPILE_MODE" "$TOOLCHAIN_PATH" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_metal.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/openamp/build_openamp.sh" "$COMPILE_MODE" "$TOOLCHAIN_PATH" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_openamp.sh failed!" >&2
    exit 1
}

sh "./build_bsp.sh" "$TOOLCHAIN_PATH" "$BOARD_PATH" "$CPU_TYPE" "$COMPILE_MODE" "$APP_NAME" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_bsp.sh failed!" >&2
    exit 1
}

echo "Build completed successfully."
