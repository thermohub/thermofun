# Require a certain version of cmake
cmake_minimum_required(VERSION 3.0)

# Set the name of the project
project(thermofungui VERSION 0.1.5 LANGUAGES CXX)

# Set the cmake module path of the project
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#set(CMAKE_MACOSX_RPATH ON)

find_package(Qt5Widgets REQUIRED)
if ( NOT Qt5Widgets_FOUND )
  message(FATAL_ERROR "Package Qt5 required, but not found!")
endif( NOT Qt5Widgets_FOUND )

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package (Qt5WebEngineWidgets REQUIRED )
find_package (Qt5Charts REQUIRED )
find_package (Qt5Concurrent REQUIRED )
# Add the include directories for the Qt 5 Widgets module to
# the compile lines.
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
include_directories(${Qt5WebEngineWidgets_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

find_library(JSONUI_LIB jsonui)
if(NOT JSONUI_LIB)
  message(FATAL_ERROR "jsonui library not found")
endif()

find_library(THRIFT_LIB thrift)
if(NOT THRIFT_LIB)
  message(FATAL_ERROR "thrift library not found")
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Define which types of libraries to build
option(TFUN_BUILD_SHARED_LIBS "Build shared libraries." ON)
option(TFUN_BUILD_STATIC_LIBS "Build static libraries." ON)

# Set the thermofun source directory path
set(THERMOFUNGUI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/fungui/src)

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
#link_directories(${THIRDPARTY_LIBRARY_DIR})

# Build thermofun library
add_subdirectory(src)

if(TFUN_BUILD_DEMOGUI)
	add_subdirectory(demo)
#else()
#	add_subdirectory(demo EXCLUDE_FROM_ALL)
endif()
