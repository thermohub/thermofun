To cross-compile, install the mingw, here the 64 bit gcc-like build tools. On ubuntu, with:

$ sudo apt-get install g++-mingw-w64 mingw-w64-{tools,x86-64-dev}
(https://swarminglogic.com/article/2014_11_crosscompile)


/// (1) Boost libraries

use install-boost.sh script


/// (2) curl cross compile - donload soirce code and unzip (had problems setting the right compiler)
/// has to be 64-bit outherwhise will produce an error when compiling jsonio - incompatible 

./configure --host=x86_64-w64-mingw32

/// custom folder
cd $HOME/mingw64/curl-7.62.0
./configure --host=x86_64-w64-mingw32
make
DESTDIR=$HOME/mingw64 make install


/// (3) Velocypak
Modify CMakeList file 

!! - invesitgate why for jsonio the shared library has the same name as the static one jsonio.dll.
For Velocypak and thermofun different name is necessary! 

/// CMakeList

cmake_minimum_required(VERSION 2.8)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/x86_64-w64-mingw32)


/// Misc
// possible to use for linking in cmake 
file(GLOB LIBS "${BOOST_PATH}/libboost*.a")
target_link_libraries(${TARGET} PRIVATE ${LIBS})

/// changes to jsonio
- uint to unsigned int 
- access() is provided from <unistd.h>
-  #include <pwd.h> - only for linux 