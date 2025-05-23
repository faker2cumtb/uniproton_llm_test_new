
function clone_openamp()
{
    echo "##################### git clone openamp start #####################"
    msge="###################### git clone openamp end ######################"
    pushd $HOME_DIR/components/openamp >/dev/null || exit 1
    rm -rf ./OpenAMP*
    tar -xzf ../../packages/OpenAMP/openamp-2022.10.1.tar.gz
    mv ./openamp-2022.10.1 ./OpenAMP
    patch -p1 -d OpenAMP <UniProton-patch-for-OpenAMP.patch
    popd >/dev/null
    echo $msge
}

function build_openamp()
{
    echo "###################### build openamp start ######################"
    msge="####################### build openamp end #######################"
    pushd $HOME_DIR/components/openamp/OpenAMP >/dev/null || exit 1
    rm -f $BOARD_DIR/libs/libopenamp.a
    rm -fr $BOARD_DIR/include/openamp
    local output_dir=$BUILD_DIR/components/openamp
    rm -fr $output_dir
    cmake . -B $output_dir -DCMAKE_TOOLCHAIN_FILE=cmake/platforms/uniproton_arm64_gcc.cmake \
                           -DTOOLCHAIN_PATH:STRING="$TOOLCHAIN_PATH" -DBUILD_DIR:STRING="$BUILD_DIR" \
                           -DCOMPILE_MODE:STRING="$COMPILE_MODE" -DTOOLCHAIN_PREFIX:STRING="$TOOLCHAIN_PREFIX"
    cd $output_dir
    make VERBOSE=1 install
    if [ $? -ne 0 ];then
        echo "make openamp failed!"
        exit 1
    fi
    popd >/dev/null
    echo $msge
}

BUILD_DIR="$PWD"
BOARD_DIR=$(dirname "$BUILD_DIR")
HOME_DIR=$BUILD_DIR/../../../../

COMPILE_MODE=$1
TOOLCHAIN_PATH=$2
TOOLCHAIN_PREFIX=$3
[ -n "$TOOLCHAIN_PATH" ] || { echo "ERROR: TOOLCHAIN_PATH is not set!" >&2 && exit 1; }
[ -n "$TOOLCHAIN_PREFIX" ] || { echo "ERROR: TOOLCHAIN_PREFIX is not set!" >&2 && exit 1; }

clone_openamp
build_openamp
