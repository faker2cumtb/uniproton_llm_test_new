
function clone_metal()
{
    echo "####################### git clone libmetal start #######################"
    msge="####################### git clone libmetal end #######################"
    pushd $HOME_DIR/components/metal >/dev/null || exit 1
    rm -rf ./libmetal*
    tar -xzf ../../packages/libmetal/libmetal-2022.10.0.tar.gz
    mv ./libmetal-2022.10.0 ./libmetal
    patch -p1 -d libmetal < UniProton-patch-for-libmetal.patch
    popd >/dev/null
    echo $msge
}

function build_metal()
{
    echo "###################### build metal start ######################"
    msge="####################### build metal end #######################"
    pushd $HOME_DIR/components/metal/libmetal >/dev/null || exit 1
    rm -f $BOARD_DIR/libs/$COMPILE_MODE/libmetal.a
    rm -fr $BOARD_DIR/include/metal
    local output_dir=$BUILD_DIR/components/metal
    rm -fr $output_dir
    cmake . -B $output_dir -DCMAKE_TOOLCHAIN_FILE=cmake/platforms/uniproton_arm64_gcc.cmake \
                           -DTOOLCHAIN_PATH:STRING="$TOOLCHAIN_PATH" -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF \
                           -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF -DBUILD_DIR:STRING="$BUILD_DIR" \
                           -DCOMPILE_MODE:STRING="$COMPILE_MODE" -DTOOLCHAIN_PREFIX:STRING="$TOOLCHAIN_PREFIX"
    cd $output_dir
    make VERBOSE=1 install
    if [ $? -ne 0 ];then
    	echo "make metal failed!"
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

clone_metal
build_metal