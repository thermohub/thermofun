# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Set the cmake module path of the project
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#set(CMAKE_MACOSX_RPATH ON)

# Set the name of the project
project(thermofungui)

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Define which types of libraries to build
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(BUILD_STATIC_LIBS "Build static libraries." ON)

# Set the thermofun source directory path
set(THERMOFUNGUI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/DemoGUI/src)

# Set the directory where the third-party libraries are temporarily installed before installed to CMAKE_INSTALL_PREFIX
#set(THIRDPARTY_DIR ${CMAKE_BINARY_DIR}/thirdparty)

# Set the include and library directories where the third party libraries were installed
#set(THIRDPARTY_INCLUDE_DIR ${THIRDPARTY_DIR}/include)
#set(THIRDPARTY_LIBRARY_DIR ${THIRDPARTY_DIR}/lib)

# Install the third-party libraries first
#include(InstallThirdPartyLibs)

# Set the include directories
include_directories(${THERMOFUNGUI_SOURCE_DIR})
#include_directories(${THIRDPARTY_INCLUDE_DIR})

# Set the lib directory of the third party libraries installed automatically
link_directories(${THIRDPARTY_LIBRARY_DIR})

# Build thermofun library
add_subdirectory(src)

