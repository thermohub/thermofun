#!/bin/bash

J=1

while [[ $# > 1 ]]
do
key="$1"

case $key in
    -j)
    J="$2"
    shift
    ;;
    *)

    ;;
esac
shift
done

CMAKE_FILE_DIR=$(pwd)/ThirdParty

mkdir -p ../../build-ThermoFun-gui/{debug,release}/thirdparty

cd ../../build-ThermoFun-gui/debug/thirdparty

cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.

make -j$J install

cd ../../release/thirdparty

cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=.

make -j$J install=$1

make -j$J install
