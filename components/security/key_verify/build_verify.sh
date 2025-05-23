#!/bin/bash

function build_key_verify() {
    echo "###################### build key_verify start ######################"
    local msge="####################### build key_verify end #######################"

    local key_verify_dir=$HOME_DIR/components/security/key_verify
    local output_dir=$BUILD_DIR/components/security/key_verify

    pushd "$key_verify_dir" >/dev/null || exit 1

    # clean old files
    rm -f "$BOARD_DIR/libs/$COMPILE_MODE/libkey_verify.a"
    rm -fr "$BOARD_DIR/include/key_verify"
    rm -fr "$output_dir"

    # Using cmake to build
    cmake -S . -B "$output_dir" \
          -DTOOLCHAIN_PATH:STRING="$TOOLCHAIN_PATH" \
          -DCC_OPTION:STRING="$CC_OPTION" \
          -DPOSIX_OPTION:STRING="$POSIX_OPTION" \
          -DBUILD_DIR:STRING="$BUILD_DIR" \
          -DCOMPILE_MODE:STRING="$COMPILE_MODE" \
          -DTOOLCHAIN_PREFIX:STRING="$TOOLCHAIN_PREFIX"

    cd "$output_dir" || exit 1
    make && make install >/dev/null
    local rc_make=$?

    if [ $rc_make -ne 0 ]; then
        echo "ERROR: make key_verify failed!" >&2
        popd >/dev/null
        exit 1
    fi

    popd >/dev/null
    echo "$msge"
}

BUILD_DIR="$PWD"
BOARD_DIR=$(dirname "$BUILD_DIR")
HOME_DIR=$BUILD_DIR/../../../../
TOOLCHAIN_PATH=$1
COMPILE_MODE=$2
TOOLCHAIN_PREFIX=$3

CC_OPTION=$(sed -ne '/^set(CC_OPTION/{s/^[^"]*"//;s/")//;p;}' "$BOARD_DIR/CMakeLists.txt")
POSIX_OPTION=$(sed -ne '/^set(POSIX_OPTION/{s/^[^"]*"//;s/")//;p;}' "$BOARD_DIR/CMakeLists.txt")

build_key_verify
