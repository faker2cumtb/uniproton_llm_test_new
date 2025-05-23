#git clone https://gitee.com/openeuler/libboundscheck.git
tar -xzf ../../../../packages/libboundscheck/libboundscheck-1b5b2300.tar.gz

[ -d ../include ] || mkdir ../include
cp libboundscheck/include/* ../include

cp libboundscheck/include/* ../../../../platform/libboundscheck/include
cp libboundscheck/src/* ../../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ../../../../

BOARD_PATH=$1
CPU_TYPE=$2
COMPILE_MODE=$3

python build.py $CPU_TYPE || exit 1

LIB_DIR=$BOARD_PATH/libs/$COMPILE_MODE
INC_DIR=$BOARD_PATH/include

[ -d "$LIB_DIR" ] || mkdir $LIB_DIR
cp output/libboundscheck/lib/$CPU_TYPE/* $LIB_DIR
cp output/UniProton/lib/$CPU_TYPE/* $LIB_DIR

[ -d "$INC_DIR" ] || mkdir $INC_DIR
cp -r output/UniProton/config/$CPU_TYPE/prt_buildef.h $INC_DIR
cp -r output/libc/include/* $INC_DIR
cp -r platform/libboundscheck/include/* $INC_DIR
cp -r src/include/uapi/* $INC_DIR

[ -d $INC_DIR/cpu ] || mkdir $INC_DIR/cpu
cp -a src/arch/cpu/armv8 $INC_DIR/cpu
find $INC_DIR/cpu -type f | grep -v -E "\.h$" | xargs rm -f

[ -d $INC_DIR/arch ] || mkdir $INC_DIR/arch
cp -a src/arch/include/*.h $INC_DIR/arch

[ -d $INC_DIR/utility ] || mkdir $INC_DIR/utility
cp -a src/utility/lib/include/*.h $INC_DIR/utility

[ -d $INC_DIR/kernel ] || mkdir $INC_DIR/kernel
cp -a src/core/kernel/include/*.h $INC_DIR/kernel

if [ -d "output/linux/include" ] ; then
    cp -r output/linux/include/* $INC_DIR
fi

popd
