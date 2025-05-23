#!/bin/bash
set -e
source ../../../../scripts/common_test.sh
export CPU_TYPE=rk3588
COMPILE_MODE="release"

if [ "$1" == "all" ] || [ "$1" == "" ]
then
    for i in $ALL
    do
        build $i
    done
elif [ "$1" != "" ]
then
    build $1
fi

if ! ls ../../../../*.tar.gz;then
    echo "gen sdk file"
    cd ../../../../ && sh scripts/gen-sdk.sh GSK tl3588-evm
fi
