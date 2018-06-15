# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Set the name of the project
project(thermofun)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_MACOSX_RPATH ON)

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Define which types of libraries to build
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(BUILD_STATIC_LIBS "Build static libraries." ON)

# ThermoFun currently is not setup to produce a dynamic library using MSVC, only static
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(BUILD_SHARED_LIBS OFF)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set the list of compiler flags for GNU compiler
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-std=c++11 -Wall -Wno-misleading-indentation -Wno-ignored-attributes -Wno-pedantic -Wno-variadic-macros -Wno-deprecated)
endif()

# Set the list of compiler flags for Clang compiler
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-std=c++11 -Wall -Wno-ignored-attributes -Wno-pedantic -Wno-variadic-macros -Wno-deprecated)
endif()

# Set the list of compiler flags for Intel compiler
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    add_compile_options(-std=c++11 -Wall -Wno-variadic-macros -Wno-deprecated)
endif()

# Set the list of compiler flags for MSVC compiler
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options("/W0 -D_SCL_SECURE_NO_WARNINGS /MP4")
endif()


# Set the thermofun source directory path
set(THERMOFUN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

# Set the directory where the third-party libraries are temporarily installed before installed to CMAKE_INSTALL_PREFIX
set(THIRDPARTY_DIR ${CMAKE_BINARY_DIR}/thirdparty)

# Set the include and library directories where the third party libraries were installed
set(THIRDPARTY_INCLUDE_DIR ${THIRDPARTY_DIR}/include)
set(THIRDPARTY_LIBRARY_DIR ${THIRDPARTY_DIR}/lib)

# Set the include directories
include_directories(${THERMOFUN_SOURCE_DIR})
include_directories(${THIRDPARTY_INCLUDE_DIR})
include_directories(${BOOST_INCLUDE_DIR})

# Install the third-party libraries first
include(InstallThirdPartyLibs)

# Set the lib directory of the third party libraries installed automatically
link_directories(${THIRDPARTY_LIBRARY_DIR})

# Build thermofun library
add_subdirectory(src)
