# Require a certain version of cmake
cmake_minimum_required(VERSION 3.0)

# Set the name of the project
project(ThermoFun VERSION 0.1.10 LANGUAGES CXX)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

include_directories("${CMAKE_SOURCE_DIR}/../dependencies/include")

else() # build for linux

endif()

if (BUILD_WINDOWS)
set(CMAKE_CXX_STANDARD 11)
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/x86_64-w64-mingw32)

include_directories("/usr/local/include")
link_directories("/usr/local/lib")

else() # build for linux
endif()

# Use ccache to speed up repeated compilations
include(CCache)

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Ensure proper configuration if in a conda environment
include(CondaAware)

# Define which types of libraries to build
option(TFUN_BUILD_SHARED_LIBS "Build shared libraries." ON)
option(TFUN_BUILD_STATIC_LIBS "Build static libraries." ON)
#option(TFUN_BUILD_TESTS "Build tests." OFF)
option(TFUN_BUILD_GUI "Build ThermoFunGui library." OFF)
option(TFUN_BUILD_HUBCLIENT "Build ThermoFun with ThermoHubClient." OFF)
option(TFUN_BUILD_GUIDEMO "Build ThermoFunGui demo." OFF)
option(TFUN_BUILD_PYTHON "Build the python wrappers and python package thermofun." ON)
#option(REFRESH_THIRDPARTY "Refresh thirdparty libraries." OFF)

# Modify the TFUN_BUILD_* variables accordingly to BUILD_ALL
if(TFUN_BUILD_ALL MATCHES ON)
    set(TFUN_BUILD_SHARED_LIBS          ON)
    set(TFUN_BUILD_STATIC_LIBS          ON)
#   set(TFUN_BUILD_TESTS                ON)
    set(TFUN_BUILD_GUI                  ON)
    set(TFUN_BUILD_HUBCLIENT            ON)
    set(TFUN_BUILD_GUIDEMO              ON)
    set(TFUN_BUILD_PYTHON               ON)
endif()

# DBClient is necessary when building the GUI DEMO
if(TFUN_BUILD_GUI MATCHES ON OR TFUN_BUILD_GUIDEMO MATCHES ON)
    set(TFUN_BUILD_HUBCLIENT             ON)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    # The build type selection for the project
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type for ${PROJECT_NAME}." FORCE)

    # The build type options for the project
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

# Define if shared library should be build instead of static.
option(BUILD_SHARED_LIBS "Build shared libraries." ON)

# Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the list of compiler flags for MSVC compiler
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options(
        /D_SCL_SECURE_NO_WARNINGS
        /D_CRT_SECURE_NO_WARNINGS=1
        /MP4
        /EHsc
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        /DNOMINMAX
    )
endif()

# Set the thermofun source directory path
set(THERMOFUN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/ThermoFun)

# Set the include directories
include_directories(${THERMOFUN_SOURCE_DIR})
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    include_directories("/usr/local/include")
    link_directories("/usr/local/lib")
endif()

# Build thermofun library
add_subdirectory(ThermoFun)

# Copy Resources folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        DESTINATION ${CMAKE_BINARY_DIR})

# Build the tests
#if(BUILD_TESTS)
#    add_subdirectory(tests)
#else()
#    add_subdirectory(tests EXCLUDE_FROM_ALL)
#endif()

# Build ThermoFunGui library
if(TFUN_BUILD_HUBCLIENT)
    add_subdirectory(ThermoHubClient)
#else()
#    add_subdirectory(ThermoHubClient EXCLUDE_FROM_ALL)
endif()

# Build ThermoFunGui library
if(TFUN_BUILD_GUI)
    add_subdirectory(ThermoFunGui)
#else()
#    add_subdirectory(ThermoFunGui EXCLUDE_FROM_ALL)
endif()

# Build python wraper
if(TFUN_BUILD_PYTHON)
    add_subdirectory(python)
#else()
#    add_subdirectory(python EXCLUDE_FROM_ALL)
endif()

# Install the cmake config files that permit users to use find_package(ThermoFun)
include(ThermoFunInstallCMakeConfigFiles)
