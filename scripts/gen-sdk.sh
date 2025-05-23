#!/bin/bash
[ $# -ne 2 ] && echo "usage: sh $0 <tag, eg: GSK> <board, eg: gsk-e2000q>" >&2 && exit 1
[ $# -gt 0 ] && SDK_TGT=$1
[ $# -gt 1 ] && BOARD=$2
DIR_TOP=$(dirname $PWD)
DIR_SRC=$(basename $PWD)
DIR_TGT=$DIR_SRC-$SDK_TGT-$BOARD
COMMIT_ID=$(git rev-parse --short HEAD 2>/dev/null)
MODES="debug release"

[ ! -d /opt/buildtools/ ] && echo "ERROR: Please running this script in docker!" >&2 && exit 1

[ ! -e config.xml ] && echo "ERROR: Please running this script at top dir!" >&2 && exit 1

DIR_BOARD=$(find bsp -type d -name "$BOARD")
([ -z "$BOARD" ] || [ -z "$DIR_BOARD" ]) && echo "ERROR: The board ($BOARD) is not supported yet!" >&2 && exit 1
NUM_DIR_BOARD=$(echo "$DIR_BOARD" | wc -l)
[ $NUM_DIR_BOARD -gt 1 ] && echo "ERROR: Too many directories found for board ($BOARD)!" >&2 && exit 1

CPU_TYPE=$(sed -n -e '/^export CPU_TYPE=/{s/.*=//;p;}' $DIR_BOARD/build/build_app.sh | tail -n 1)
[ -z "$CPU_TYPE" ] && echo "ERROR: Can not get CPU_TYPE for board ($BOARD)!" >&2 && exit 1

FILE_DEFCONFIG=$(ls $DIR_BOARD/config/defconfig 2>/dev/null)
[ -z "$FILE_DEFCONFIG" ] && echo "ERROR: Can not find defconfig for board ($BOARD)!" >&2 && exit 1
NUM_FILE_DEFCONFIG=$(echo "$FILE_DEFCONFIG" | wc -l)
[ $NUM_FILE_DEFCONFIG -gt 1 ] && echo "ERROR: Too many defconfig found for board ($BOARD)!" >&2 && exit 1

cd .. || exit 1
[ -d $DIR_TGT ] && echo "ERROR: Directory ($PWD/$DIR_TGT) already exists!" >&2 && exit 1
trap "rm -fr $DIR_TOP/$DIR_TGT" EXIT HUP INT QUIT TERM
cp -r $DIR_SRC $DIR_TGT && cd $DIR_TGT || exit 1

function build_sdk()
{
    git clean -dfx --exclude=$DIR_BOARD/include --exclude=$DIR_BOARD/libs >/dev/null
    local mode=$1
    [ ".$mode" == ".debug" ] && \
        sed -i -e 's%UniProton_compile_mode>.*<%UniProton_compile_mode>debug<%' config.xml && \
        sed -i -e 's%^.*CONFIG_OS_GDB_STUB.*$%CONFIG_OS_GDB_STUB=y%' $FILE_DEFCONFIG
    [ ".$mode" == ".release" ] && \
        sed -i -e 's%UniProton_compile_mode>.*<%UniProton_compile_mode>release<%' config.xml && \
        sed -i -e 's%^.*CONFIG_OS_GDB_STUB.*$%#CONFIG_OS_GDB_STUB=y%' $FILE_DEFCONFIG
    cd /$DIR_TOP/$DIR_TGT/$DIR_BOARD/build && sh build_app.sh || exit 1
}
for mode in $MODES
do
    echo "INFO: Build sdk for $mode ..."
    pushd . >/dev/null
    build_sdk $mode
    popd >/dev/null
done

echo "INFO: Clean sdk ..."
git clean -dfx --exclude=$DIR_BOARD/include --exclude=$DIR_BOARD/libs >/dev/null
DIR_TMP=$(mktemp -d -p . -u -t tmp.XXXXX)
mv $DIR_BOARD $DIR_TMP && rm -fr bsp && mkdir -p $(dirname $DIR_BOARD) && mv $DIR_TMP $DIR_BOARD
rm -fr $DIR_BOARD/CMakeLists.txt $DIR_BOARD/config $DIR_BOARD/src
find $DIR_BOARD/build -type f -name "*.sh" | grep -v -P "/build_app\.sh$" | xargs rm -f
sed -i -e '/\(import_kconfig.*defconfig\|\((\|#\)[ ]*CONFIG_\)/{d}' $DIR_BOARD/apps/CMakeLists.txt
perl -i -pe 'undef $/;s/\npushd .*? build_board\.sh .*? popd\n//smg' $DIR_BOARD/build/build_app.sh
sed -i -e '/\(HOME_PATH\|PRT_CONF_FILE\|defconfig\|-objdump\)/{d}' $DIR_BOARD/build/build_app.sh
rm -f build.py config.xml
rm -fr build components packages platform scripts src
find cmake/tool_chain -type f | grep -v -P "/${CPU_TYPE}_" | xargs rm -f
find demos -mindepth 1 -maxdepth 1 -type d | grep -v -P "/common$" | xargs rm -fr
find . -type f -name "*.in" | xargs rm -f
rm -fr .gitignore .git

echo "INFO: Test sdk ..."
pushd . >/dev/null
cd /$DIR_TOP/$DIR_TGT/$DIR_BOARD/build && sh build_app.sh || exit 1
popd >/dev/null

echo "INFO: Pack sdk ..."
cd .. && rm -f $DIR_TGT.tar.gz && tar -czf $DIR_TGT.tar.gz $DIR_TGT || exit 1
curr_date=$(date +%Y%m%d)
check_sum=$(md5sum $DIR_TGT.tar.gz | cut -d' ' -f1)
sdk_file=$DIR_TOP/$DIR_SRC/$DIR_TGT-$curr_date-$COMMIT_ID-$check_sum.tar.gz
mv $DIR_TGT.tar.gz $sdk_file || exit 1

echo "INFO: Done. SDK file is $sdk_file"
