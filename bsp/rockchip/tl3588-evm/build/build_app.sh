###################### build boards ######################
# export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
# export TOOLCHAIN_PREFIX=aarch64-none-elf
export TOOLCHAIN_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr
export TOOLCHAIN_PREFIX=aarch64-openeuler-linux

SCRIPT_DIR=$(dirname $(realpath $0))
BOARD_PATH=$(basename $(dirname $SCRIPT_DIR))
BOARD_PATH=$(basename $(dirname $(dirname $SCRIPT_DIR)))/$BOARD_PATH
BOARD_PATH=$(basename $(dirname $(dirname $(dirname $SCRIPT_DIR))))/$BOARD_PATH
export BOARD_PATH

export CPU_TYPE=rk3588
export APP_NAME=uniproton

# export ALL="task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"
export ALL=$APP_NAME

HOME_PATH=$PWD/../../../..
PRT_CONF_FILE=$HOME_PATH/config.xml
COMPILE_MODE="release"

cp -f "../config/defconfig" "$HOME_PATH/build/uniproton_config/config_armv8_$CPU_TYPE/"

if [ -f "$PRT_CONF_FILE" ]; then
    COMPILE_MODE=$(sed -n -e '/UniProton_compile_mode/{s/.*_mode>\(.*\)<.*/\1/;p;}' "$PRT_CONF_FILE")
fi # PRT_CONF_FILE

for x in "$@"; do
    if [ "$x" == "-d" ]; then
        COMPILE_MODE="debug"
    else
        if [ -n "$COMP_APPS" ]; then
            COMP_APPS="$COMP_APPS $x"
        else
            COMP_APPS=$x
        fi
    fi
done

set -- $COMP_APPS

pushd "$HOME_PATH/$BOARD_PATH/build" && {
    sh build_board.sh \
        "$TOOLCHAIN_PATH" \
        "$BOARD_PATH" \
        "$CPU_TYPE" \
        "$COMPILE_MODE" \
        "$APP_NAME" \
        "$TOOLCHAIN_PREFIX" || exit 1
} && popd

###################### build apps ######################
function build() {
    export APP=$1
    export BUILD_DIR=apps

    cmake -S ../apps -B $BUILD_DIR \
        -DAPP:STRING=$APP \
        -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH \
        -DCPU_TYPE:STRING=$CPU_TYPE \
        -DCOMPILE_MODE:STRING=$COMPILE_MODE \
        -DTOOLCHAIN_PREFIX:STRING=$TOOLCHAIN_PREFIX

    pushd $BUILD_DIR && {
        make $APP || exit 1
    } && popd

    cp "./$BUILD_DIR/$APP" "$APP.elf"

    $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objcopy -O binary "./$APP.elf" "$APP.bin"
    $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objdump -D "./$APP.elf" > "$APP.asm"
    rm -rf $BUILD_DIR
}

if [ "$1" == "all" ] || [ "$1" == "" ]; then
    for i in $ALL; do
        build $i
    done
elif [ "$1" != "" ]; then
    build $1
fi
