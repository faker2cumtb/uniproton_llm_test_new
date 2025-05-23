function extract_canopen()
{
    echo "###################### extract canopen start ######################"
    msge="####################### extract canopen end #######################"
    pushd $HOME_DIR/components/canopen >/dev/null || exit 1
    rm -fr CANopenNode
    tar -xzf ../../packages/canopen/CANopenNode-f96ffb2.tar.gz
    patch -p 1 -d CANopenNode <UniProton-patch-for-CANopenNode.patch
    popd >/dev/null
    echo $msge
}

function build_canopen()
{
    echo "###################### build canopen start ######################"
    msge="####################### build canopen end #######################"
    pushd $PORT_DIR >/dev/null || exit 1
    rm -f $BOARD_DIR/libs/$COMPILE_MODE/libcanopen.a
    rm -fr $BOARD_DIR/include/canopen
    local output_dir=$BUILD_DIR/components/canopen
    rm -fr $output_dir
    cmake -S . -B $output_dir -DCOMPILE_MODE:STRING="$COMPILE_MODE" -DTOOLCHAIN_PATH:STRING="$TOOLCHAIN_PATH" \
                              -DCC_OPTION:STRING="$CC_OPTION" -DPOSIX_OPTION:STRING="$POSIX_OPTION" \
                              -DBUILD_DIR:STRING="$BUILD_DIR" -DTOOLCHAIN_PREFIX:STRING="$TOOLCHAIN_PREFIX"

    cd $output_dir
    make && make install >/dev/null
    rc_make=$?
    if [ $rc_make -ne 0 ]
    then
        echo "ERROR: make canopen failed!" >&2
        exit 1
    fi
    popd >/dev/null
    echo $msge
}

BUILD_DIR="$PWD"
BOARD_DIR=$(dirname "$BUILD_DIR")
HOME_DIR=$BUILD_DIR/../../../../
PORT_DIR=$HOME_DIR/components/canopen/CANopenNode_port
COMPILE_MODE=$1
TOOLCHAIN_PATH=$2
TOOLCHAIN_PREFIX=$3


[ -d "$PORT_DIR" ] || { echo "ERROR: Directory ($PORT_DIR) does not exist!" >&2 && exit 1; }
[ -n "$TOOLCHAIN_PATH" ] || { echo "ERROR: TOOLCHAIN_PATH is not set!" >&2 && exit 1; }
[ -n "$TOOLCHAIN_PREFIX" ] || { echo "ERROR: TOOLCHAIN_PREFIX is not set!" >&2 && exit 1; }
[ -f $BOARD_DIR/CMakeLists.txt ] || { echo "ERROR: CMakeLists.txt for ($BOARD_DIR) does not exit!" >&2 && exit 1; }

CC_OPTION=$(sed -ne '/^set(CC_OPTION/{s/^[^"]*"//;s/")//;p;}' $BOARD_DIR/CMakeLists.txt)
POSIX_OPTION=$(sed -ne '/^set(POSIX_OPTION/{s/^[^"]*"//;s/")//;p;}' $BOARD_DIR/CMakeLists.txt)

extract_canopen
build_canopen
