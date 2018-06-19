#!/bin/bash

J=8

while [[ $# > 2 ]]
do
key="$2"

case $key in
    -j)
    J="$3"
    shift
    ;;
    *)

    ;;
esac
shift
done

CMAKE_FILE_DIR=$(pwd)/ThirdParty

mkdir -p ../../build-DemoGUI/release/thirdparty

cd ../../build-DemoGUI/release/thirdparty

cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Release -DREFRESH_THIRDPARTY=OFF -DCMAKE_INSTALL_PREFIX=. -DCMAKE_PREFIX_PATH=$1

make -j$J 
sudo make install
sudo ldconfig

