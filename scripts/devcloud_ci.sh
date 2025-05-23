#! /bin/bash
# set -x
ls -al
path1=`pwd`
echo $path1

#add check app path here
BUILD_PATH=(
    "bsp/phytium/gsk-e2000q/" \
    "bsp/phytium/phytium-pi/" \
    "bsp/rockchip/tl3588-evm/" )

build_and_check_board(){
    cd $1/build
    echo "build $1"
    sh build_app.sh

    echo "当前路径： `pwd` "

    if ! ls *.bin ; then
        echo "build $1 failed: 没有找到 .bin 文件"
        return 1  
    fi
    
    echo "check $1 success"

    cd $path1
    return 0
}

main()
{
    # set -e
    for board in ${BUILD_PATH[@]}
    do
        build_and_check_board $board
        if [ $? -ne 0 ]; then
            echo "build $board fail"
        return 1
    fi
    done
    echo "build all board success"
    return 0
}

main $@
