#!/bin/bash

J=10

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

mkdir -p ../../build-DemoGUI/debug/thirdparty

cd ../../build-DemoGUI/debug/thirdparty

cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=. -DCMAKE_PREFIX_PATH=$1

make -j$J 
sudo make install
sudo ldconfig

#cd ../../release/thirdparty

#cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. -DCMAKE_PREFIX_PATH=$1

#make -j$J install

