#! /bin/bash

common_dep(){
    yum --version
    if [ $? -eq 0 ]; then
        yum install -y python3 tar git xz make flex gcc expect cmake ninja-build
    else
        apt install -y python3 tar git xz-utils make flex gcc expect cmake ninja-build
    fi
}

python_dep(){
    pip3 install -i https://mirrors.bfsu.edu.cn/pypi/web/simple/ --trusted-host mirrors.bfsu.edu.cn -r scripts/requirements.txt
}

prepare_sdk(){
    sdk_url="http://192.168.110.40/repo_list/default/uniproton/buildtools.tar"
    curl -o toolchain.tar $sdk_url
    tar -xvf toolchain.tar -C /opt > tar.log
}

set_bootenv(){
    last_part=$(echo "$1" | cut -d'/' -f3)
    boot_url="http://192.168.110.40/repo_list/default/uniproton/uboot.img"
    zephyr_tftp_path="uniproton/$last_part/uniproton.bin"
    uboot_path="$4/uniproton/uboot.img"
    directory="$4/uniproton/$last_part/"
    if [ ! -d "$directory" ]; then
        mkdir -p "$directory"
        echo "目录 $directory 创建成功。"
    else
        echo "目录 $directory 已经存在。"
    fi
    curl -o $uboot_path $boot_url

    process_ids=$(lsof $1 | awk 'NR>1' | awk '{print $2}')
    if [ -n "$process_ids" ]; then
        echo "Killing processes: $process_ids"
        echo "$process_ids" | xargs kill
    else
        echo "No processes to kill"
    fi
    
    python3 ./scripts/set_bootenv.py --port $1 --board-ip $2 --tftp-ip $3 --board-type $5 --zephyr-tftp-path $zephyr_tftp_path 
}


main(){
    if [ $6 != "rtos" ]; then
        echo "hypervisor deployment type, skip cfg bootenv"
        return 0
    fi
    
    common_dep
    if [ $? -ne 0 ]; then
        echo "common_dep install fail"
        return 1
    fi

    set -e
    python_dep
    if [ $? -ne 0 ]; then
        echo "python_dep install fail"
        return 1
    fi

    if [ $# -gt 2 ]; then
        set_bootenv $@
        if [ $? -ne 0 ]; then
            echo "bootenv cfg fail"
            return 1
        fi
    fi

    # prepare_sdk $@
    # if [ $? -ne 0 ]; then
    #     echo "toolchain cfg fail"
    #     return 1
    # fi

}

main $@