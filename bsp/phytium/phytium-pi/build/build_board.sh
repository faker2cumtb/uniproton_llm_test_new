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

sh "$HOME_PATH/components/ethercat/build_fetch_igh.sh" || {
    echo "ERROR: build_fetch_igh.sh failed!" >&2
    exit 1
}

sh "./build_static.sh" "$BOARD_PATH" "$CPU_TYPE" "$COMPILE_MODE" || {
    echo "ERROR: build_static.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/lwip/build_lwip.sh" "$TOOLCHAIN_PATH" "$CPU_TYPE" "$COMPILE_MODE" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_lwip.sh failed!" >&2
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

sh "$HOME_PATH/components/canopen/build_canopen.sh" "$COMPILE_MODE" "$TOOLCHAIN_PATH" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_canopen.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/modbus/build_modbus.sh" "$COMPILE_MODE" "$TOOLCHAIN_PATH" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_modbus.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/security/mbedtls-3.6.2/build_mbedtls.sh" "$TOOLCHAIN_PATH" "$COMPILE_MODE" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_mbedtls.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/security/key_verify/build_verify.sh" "$TOOLCHAIN_PATH" "$COMPILE_MODE" "$TOOLCHAIN_PREFIX" || {
    echo "ERROR: build_verify.sh failed!" >&2
    exit 1
}

sh "$HOME_PATH/components/libcxx/libcxx_build.sh" "$TOOLCHAIN_PATH" "$BOARD_PATH" "$COMPILE_MODE"|| {
    echo "ERROR: libcxx_build.sh failed!" >&2
    exit 1
}

echo "Build completed successfully."
