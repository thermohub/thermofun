# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set the directory where the third-party libraries are temporarily installed before installed to CMAKE_INSTALL_PREFIX
set(THIRDPARTY_DIR ${CMAKE_INSTALL_PREFIX})

# Install the third-party libraries first
include(InstallThirdPartyLibs)
