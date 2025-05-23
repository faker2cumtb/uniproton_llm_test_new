tar -xzf ../../../../packages/libboundscheck/libboundscheck-1b5b2300.tar.gz

[ -d ../include ] || mkdir ../include
cp libboundscheck/include/* ../include

cp libboundscheck/include/* ../../../../platform/libboundscheck/include
cp libboundscheck/src/* ../../../../platform/libboundscheck/src
rm -rf libboundscheck


pushd ../../../../
python build.py $1

[ -d bsp/semdrive/$1/libs ] || mkdir bsp/semdrive/$1/libs
cp output/UniProton/lib/$1/* bsp/semdrive/$1/libs
cp output/libboundscheck/lib/$1/lib* bsp/semdrive/$1/libs

cp -r output/libc bsp/semdrive/$1/include
cp -r src/include/uapi/* bsp/semdrive/$1/include
cp build/uniproton_config/config_cortex_r5_d9340_secure/prt_buildef.h bsp/semdrive/$1/include/
popd
