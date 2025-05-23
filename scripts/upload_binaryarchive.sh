#! /bin/bash
common_upload_git(){
    # 配置变量
    REPO_URL="ssh://git@192.168.110.189:2222/rtos/binaryarchive.git"
    REPO_DIR="binaryarchive"
    BRANCH="main"
    APP_PATH="/tmp/"
    SOURCE_DIR=$1
    DIST_DIR=$2

    cd $APP_PATH

    # 克隆或更新仓库
    if [ -d "$REPO_DIR" ]; then
        cd $REPO_DIR
        git pull
    else
        git clone --branch $BRANCH $REPO_URL $REPO_DIR
        cd $REPO_DIR
    fi

    # 检查并创建目标目录（如果它不存在）
    if [ ! -d "$DIST_DIR" ]; then
        mkdir -p "$DIST_DIR"
        if [ $? -ne 0 ]; then
            echo "Failed to create directory: $DIST_DIR"
            exit 1
        fi
        echo "Directory created: $DIST_DIR"
    fi

    # 将生成的BIN文件复制到归档目录
    cp $SOURCE_DIR/uniproton.bin $DIST_DIR
    cp $SOURCE_DIR/uniproton.elf $DIST_DIR

    # 添加文件到Git
    git add $DIST_DIR

    # 提交更改
    git commit -m "Update uniproton binary at $(date +'%Y-%m-%d %H:%M:%S')"

    # 推送更改到远程仓库
    git push origin $BRANCH
}

pack_uniproton_binary(){
    CODE_PATH=$1
    EMB_TYPES=$2
    BUILD_DIR="$CODE_PATH/bsp/$EMB_TYPES/build"
    DIST_DIR="uniproton/$EMB_TYPES/"
    
    common_upload_git $BUILD_DIR $DIST_DIR  
}

pack_uniproton_sdk(){
    CODE_PATH=$1
    EMB_TYPES=$2
    # 拷贝到 NAS 制品地址
    NAS_DIR="/mnt/NAS/02_硬实时OS/制品发布库/"
    short_nas_timestamp=$(date +'%Y-%m-%d')
    mkdir -p ${NAS_DIR}/uniproton/${EMB_TYPES}/${short_nas_timestamp}
    mv $CODE_PATH/*.tar.gz ${NAS_DIR}/uniproton/${EMB_TYPES}/${short_nas_timestamp}
}


main(){
    
    set -e

    pack_uniproton_binary $@
    if [ $? -ne 0 ]; then
        echo "pack_uniproton_binary fail"
        return 1
    fi

    pack_uniproton_sdk $@
    if [ $? -ne 0 ]; then
        echo "pack_uniproton_binary fail"
        return 1
    fi

}

main $@
