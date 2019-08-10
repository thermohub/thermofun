# Script for compiling boost libraries using mingw64 bit
# untar the boost sources
#tar jxf boost_1_65_1.tar.bz2

cd boost_1_65_1/
 
# build the bjam program provided with Boost
./bootstrap.sh --without-icu
 
# build a cross compilation parameter file and update the version of Gcc (here 4.4)
echo "using gcc : 7.3.0 : x86_64-w64-mingw32-g++
        :
        <rc>x86_64-w64-mingw32-windres
        <archiver>x86_64-w64-mingw32-ar
;" > user-config.jam
 
# build boost and install them into the directory $PREFIX
./bjam toolset=gcc target-os=windows variant=release threading=multi threadapi=win32\
    link=shared runtime-link=shared --prefix=$PREFIX --user-config=user-config.jam -j 2\
    --without-mpi --without-python -sNO_BZIP2=1 -sNO_ZLIB=1 --layout=tagged install
