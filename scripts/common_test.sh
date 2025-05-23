#!/bin/bash
set -e

export TOOLCHAIN_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr
export TOOLCHAIN_PREFIX=aarch64-openeuler-linux

echo "当前执行环境的绝对路径是: $(pwd)"


if ! ls uniproton.elf; then
    echo "gen binary file"
    sh build_app.sh uniproton
fi

POSIX_APP=(
    "UniProton_test_posix_time_interface" \
    "UniProton_test_posix_thread_sem_interface" \
    "UniProton_test_posix_thread_pthread_interface" \
    "UniProton_test_posix_malloc_interface" \
    "UniProton_test_posix_stdlib_interface" \
    "UniProton_test_posix_ipc_interface" \
    "UniProton_test_posix_signal_interface" \
    "UniProton_test_posix_string_interface" \
    "UniProton_test_posix_regex_interface" \
    "UniProton_test_posix_prng_interface" \
    "UniProton_test_posix_rwflag_interface"
)

PROXY_APP=(
    "UniProton_test_proxy_posix_interface"
)

RHEALSTONE_APP=(
    "deadlock-break" \
    "interrupt-latency" \
    "message-latency" \
    "semaphore-shuffle" \
    "task-preempt" \
    "task-switch"
)

UART_APP=(
    "UniProton_test_drivers_inode_interface" \
    "UniProton_test_drivers_uart_interface" \
    "UniProton_test_shell_interface"
)

MATH_APP=(
    "UniProton_test_posix_math_interface"
)

EXIT_APP=(
    "UniProton_test_posix_exit_interface"
)

KERNEL_APP=(
    "UniProton_test_sem" \
    "UniProton_test_rr_sched" \
    "UniProton_test_mmu" \
    "UniProton_test_ir" \
    "UniProton_test_priority"
)

export ALL="${POSIX_APP[*]} ${RHEALSTONE_APP[*]} ${MATH_APP[*]} ${EXIT_APP[*]} ${KERNEL_APP[*]}"
# export ALL="${RHEALSTONE_APP[*]}"
# export ALL="UniProton_test_posix_exit_interface"

echo $ALL

function build()
{
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

    if [[ $APP == "UniProton_test_posix_math_interface" ]]; then
        cp $BUILD_DIR/libc-test/src/UniProton_test_posix_math_* $BUILD_DIR
        all_math_app=$(ls $BUILD_DIR/UniProton_test_posix_math_*)
        echo "所有app : $all_math_app"
        for one_math in ${all_math_app}; do
            file_name=${one_math#*/*}
            cp $BUILD_DIR/$file_name $file_name.elf

            $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objcopy -O binary $BUILD_DIR/$file_name $file_name.bin
            $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objdump -D ./$file_name.elf > $file_name.asm

        done
        rm -rf $BUILD_DIR
    
    elif [[ $APP == "UniProton_test_posix_exit_interface" ]]; then
        cp $BUILD_DIR/conformance/UniProton_test_posix_exit_* $BUILD_DIR
        all_exit_app=$(ls $BUILD_DIR/UniProton_test_posix_exit_*)
        for one_exit in ${all_exit_app}; do
            file_name=${one_exit#*/*}
            cp $BUILD_DIR/$file_name $file_name.elf
            $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objcopy -O binary $BUILD_DIR/$file_name $file_name.bin
            $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objdump -D ./$file_name.elf > $file_name.asm
        done
        rm -rf $BUILD_DIR

    else

        cp "./$BUILD_DIR/$APP" "$APP.elf"

        $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objcopy -O binary "./$APP.elf" "$APP.bin"
        $TOOLCHAIN_PATH/bin/$TOOLCHAIN_PREFIX-objdump -D "./$APP.elf" > "$APP.asm"
        rm -rf $BUILD_DIR
    fi
}


