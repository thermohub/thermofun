# Require a certain version of cmake
cmake_minimum_required(VERSION 3.16..4.2)

# Set the name of the project
project(ThermoFun VERSION 0.5.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MACOSX_RPATH ON)

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Use ccache to speed up repeated compilations
include(CCache)

# Ensure proper configuration if in a conda environment
include(CondaAware)

# Modify the TFUN_BUILD_* variables accordingly to BUILD_ALL
if(TFUN_BUILD_ALL MATCHES ON)
    set(BUILD_SHARED_LIBS          ON)
    set(BUILD_STATIC_LIBS          ON)
    set(TFUN_BUILD_TESTS           OFF)
    set(TFUN_BUILD_PYTHON           ON)
endif()

if (TFUN_WASM MATCHES ON)
    # Define which types of libraries to build
    option(BUILD_STATIC_LIBS "Build static libraries." ON)
    set(TFUN_BUILD_PYTHON               OFF)
else()
    # Ensure proper configuration if in a conda environment
    option(BUILD_SHARED_LIBS "Build shared libraries." ON)
    #option(BUILD_STATIC_LIBS "Build static libraries." ON)
    #option(TFUN_BUILD_TESTS "Build tests." OFF)
    option(TFUN_BUILD_PYTHON "Build the python wrappers and python package thermofun." ON)
    # Define if shared library should be build instead of static.
    
    # Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Used into conda only
if(DEFINED ENV{CONDA_PREFIX})
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" ON)
else()
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" OFF)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    # The build type selection for the project
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type for ${PROJECT_NAME}." FORCE)

    # The build type options for the project
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

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

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-Wall -Wno-ignored-attributes -Wno-pedantic -Wno-variadic-macros -Wno-deprecated)
endif()

# Set the thermofun source directory path
set(THERMOFUN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/ThermoFun)

# Set the include directories
include_directories(${THERMOFUN_SOURCE_DIR})
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    include_directories("/usr/local/include")
    link_directories("/usr/local/lib")
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    include_directories("c:/usr/local/include")
    link_directories("c:/usr/local/lib")
endif()

# Find all Reaktoro dependencies
include(ThermoFunFindDeps)

# Build thermofun library
add_subdirectory(ThermoFun)

# Copy Resources folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        DESTINATION ${CMAKE_BINARY_DIR})

# Build the tests
if(BUILD_TESTS)
    add_subdirectory(tests)
#else()
#    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

# Build python wraper
if(TFUN_BUILD_PYTHON)
    add_subdirectory(python)
endif()

# Install the cmake config files that permit users to use find_package(ThermoFun)
include(ThermoFunInstallCMakeConfigFiles)
