function clone_modbus()
{
    echo "###################### git clone modbus start ######################"
    msge="####################### git clone modbus end #######################"
    pushd $HOME_DIR/components/modbus >/dev/null || exit 1
    rm -fr libmodbus
    tar -xzf ../../packages/libmodbus/libmodbus-3.1.10.tar.gz
    mv libmodbus-3.1.10 libmodbus
    patch -p 1 -d libmodbus <UniProton-patch-for-libmodbus.patch
    popd >/dev/null
    echo $msge
}

function config_modbus()
{
    echo "######################## config modbus start ########################"
    msge="######################### config modbus end #########################"
    pushd $HOME_DIR/components/modbus >/dev/null || exit 1
    cd libmodbus || exit 1
    local output_dir=$BUILD_DIR/libmodbus
    rm -fr $output_dir
    ./autogen.sh
    ./configure --prefix $output_dir --enable-static=yes --disable-tests \
        CC=$TOOLCHAIN_GCC --build=x86 --host=${TOOLCHAIN_PREFIX} \
        CPPFLAGS="-I$BUILD_DIR/../bsp -I$BUILD_DIR/../bsp/hal -I$BUILD_DIR/../include -I$HOME_DIR/output/libc/include" \
        CFLAGS="$CC_OPTION"
    rc_config=$?
    if [ $rc_config -ne 0 ]
    then
        echo "ERROR: configure modbus failed!" >&2
        exit 1
    fi
    popd >/dev/null
    echo $msge
}

function build_modbus()
{
    echo "###################### build modbus-port start ######################"
    msge="####################### build modbus-port end #######################"
    pushd $PORT_DIR >/dev/null || exit 1
    rm -f $BOARD_DIR/libs/$COMPILE_MODE/libmodbus.a
    rm -fr $BOARD_DIR/include/modbus
    local output_dir=$BUILD_DIR/components/modbus
    rm -fr $output_dir
    cmake -S . -B $output_dir -DCOMPILE_MODE:STRING="$COMPILE_MODE" -DTOOLCHAIN_PATH:STRING="$TOOLCHAIN_PATH" \
                              -DCC_OPTION:STRING="$CC_OPTION" -DPOSIX_OPTION:STRING="$POSIX_OPTION" \
                              -DBUILD_DIR:STRING="$BUILD_DIR" -DTOOLCHAIN_PREFIX="$TOOLCHAIN_PREFIX"

    cd $output_dir
    make && make install >/dev/null
    rc_make=$?
    if [ $rc_make -ne 0 ]
    then
        echo "ERROR: make modbus failed!" >&2
        exit 1
    fi
    popd >/dev/null
    echo $msge
}

BUILD_DIR="$PWD"
BOARD_DIR=$(dirname "$BUILD_DIR")
HOME_DIR=$BUILD_DIR/../../../../
PORT_DIR=$HOME_DIR/components/modbus/libmodbus_port
COMPILE_MODE=$1
TOOLCHAIN_PATH=$2
TOOLCHAIN_PREFIX=$3

[ -d "$PORT_DIR" ] || { echo "ERROR: Directory ($PORT_DIR) does not exist!" >&2 && exit 1; }
[ -n "$TOOLCHAIN_PATH" ] || { echo "ERROR: TOOLCHAIN_PATH is not set!" >&2 && exit 1; }
[ -n "$TOOLCHAIN_PREFIX" ] || { echo "ERROR: TOOLCHAIN_PREFIX is not set!" >&2 && exit 1; }
[ -f $BOARD_DIR/CMakeLists.txt ] || { echo "ERROR: CMakeLists.txt for ($BOARD_DIR) does not exit!" >&2 && exit 1; }

TOOLCHAIN_GCC=$TOOLCHAIN_PATH/bin/${TOOLCHAIN_PREFIX}-gcc
CC_OPTION=$(sed -ne '/^set(CC_OPTION/{s/^[^"]*"//;s/")//;p;}' $BOARD_DIR/CMakeLists.txt)
POSIX_OPTION=$(sed -ne '/^set(POSIX_OPTION/{s/^[^"]*"//;s/")//;p;}' $BOARD_DIR/CMakeLists.txt)

clone_modbus
config_modbus
build_modbus
