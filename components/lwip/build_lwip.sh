
TOOLCHAIN_PATH=$1
CPU_TYPE=$2
COMPILE_MODE=$3
TOOLCHAIN_PREFIX=$4

echo "######################### build lwip #########################"
pushd .
cp ../../../../components/lwip/lwipopts.h ../../../../src/net/adapter/include/
mkdir -p components/lwip/build
cd components/lwip/build
rm -rf *

cmake ../../../../../../../components/lwip -DTOOLCHAIN_PATH=$TOOLCHAIN_PATH -DCPU_TYPE:STRING=$CPU_TYPE -DTOOLCHAIN_PREFIX=$TOOLCHAIN_PREFIX
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make lwip failed!"
	exit 1
fi
popd

cp -a ./components/lwip/output/lib/*.a ../libs/$COMPILE_MODE/