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

mkdir -p ../../../build-dbclient-test/debug/thirdparty

cd ../../../build-dbclient-test/debug/thirdparty

cmake $CMAKE_FILE_DIR -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=. -DCMAKE_PREFIX_PATH=$1

make -j$J 

make install

#sudo ldconfig
