# Require a certain version of cmake
cmake_minimum_required(VERSION 3.0)

# Set the name of the project
project(ThermoFun VERSION 0.3.3 LANGUAGES CXX)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Use ccache to speed up repeated compilations
include(CCache)

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Modify the TFUN_BUILD_* variables accordingly to BUILD_ALL
if(TFUN_BUILD_ALL MATCHES ON)
    set(TFUN_BUILD_SHARED_LIBS          ON)
    set(TFUN_BUILD_STATIC_LIBS          ON)
#   set(TFUN_BUILD_TESTS                ON)
    set(TFUN_BUILD_PYTHON               ON)
endif()

if (TFUN_WASM MATCHES ON)
    # Define which types of libraries to build
    option(TFUN_BUILD_STATIC_LIBS "Build static libraries." ON)
    set(TFUN_BUILD_PYTHON               OFF)
    # Set the list of compiler flags for Clang compiler
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-std=c++11 -Wall -Wno-ignored-attributes -Wno-pedantic -Wno-variadic-macros -Wno-deprecated -fcxx-exceptions -frtti)
    endif()
else()
    # Ensure proper configuration if in a conda environment
    include(CondaAware)
    option(TFUN_BUILD_SHARED_LIBS "Build shared libraries." ON)
    option(TFUN_BUILD_STATIC_LIBS "Build static libraries." OFF)
    #option(TFUN_BUILD_TESTS "Build tests." OFF)
    option(TFUN_BUILD_PYTHON "Build the python wrappers and python package thermofun." ON)
    # Define if shared library should be build instead of static.
    option(BUILD_SHARED_LIBS "Build shared libraries." ON)

    # Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    # The build type selection for the project
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type for ${PROJECT_NAME}." FORCE)

    # The build type options for the project
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

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

# Find all Reaktoro dependencies
include(ThermoFunFindDeps)

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

# Build python wraper
if(TFUN_BUILD_PYTHON)
    add_subdirectory(python)
endif()

# Install the cmake config files that permit users to use find_package(ThermoFun)
include(ThermoFunInstallCMakeConfigFiles)
