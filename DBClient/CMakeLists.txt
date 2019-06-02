# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Set the name of the project
project(thermodbclient)

find_library(BOOSTRE_LIB boost_regex)
if(NOT BOOSTRE_LIB)
  message(FATAL_ERROR "boost_regex library not found")
endif()

find_library(BOOSTFILE_LIB boost_filesystem)
if(NOT BOOSTFILE_LIB)
  message(FATAL_ERROR "boost_filesystem library not found")
endif()

find_library(CURL_LIB curl)
if(NOT CURL_LIB)
  message(FATAL_ERROR "curl library not found")
endif()

find_library(VELOCYPACK_LIB velocypack)
if(NOT VELOCYPACK_LIB)
  message(FATAL_ERROR "velocypack library not found")
endif()

find_library(JSONIO_LIB jsonio)
if(NOT JSONIO_LIB)
  message(FATAL_ERROR "jsonio library not found")
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
option(THERMODBCLIENT_BUILD_SHARED_LIBS "Build shared libraries." ON)
option(THERMODBCLIENT_BUILD_STATIC_LIBS "Build static libraries." ON)

# Set the thermofun source directory path
set(THERMODBCLIENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/DBClient)

# Set the include directories
include_directories(${THERMODBCLIENT_SOURCE_DIR})

# Build thermodbclient library
# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_definitions(-DFROM_SRC)

# Set the list of names of the third-party targets and libraries
#set(THIRDPARTY_TARGETS JSONIO)
set(THIRDPARTY_LIBS jsonio boost_regex boost_filesystem curl velocypack)
set(THERMODBCLIENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/)

# Create the shared library using the collected source files
add_library(THERMODBCLIENT_OBJECTS OBJECT ${HEADER_FILES} ${SOURCE_FILES})

# Specify the dependencies of THERMOFUN to ensure proper build sequence
#add_dependencies(THERMOFUN_OBJECTS ${THIRDPARTY_TARGETS})

# Check if a shared library is to be built
if(THERMODBCLIENT_BUILD_SHARED_LIBS)
    add_library(THERMODBCLIENT_SHARED SHARED $<TARGET_OBJECTS:THERMODBCLIENT_OBJECTS>)
    target_link_libraries(THERMODBCLIENT_SHARED ${THIRDPARTY_LIBS} )
    set_target_properties(THERMODBCLIENT_SHARED PROPERTIES OUTPUT_NAME thermodbclient)
    install(TARGETS THERMODBCLIENT_SHARED DESTINATION lib)
endif()

# Check if a static library is to be built
if(THERMODBCLIENT_BUILD_STATIC_LIBS)
    add_library(THERMODBCLIENT_STATIC STATIC $<TARGET_OBJECTS:THERMODBCLIENT_OBJECTS>)
    target_link_libraries(THERMODBCLIENT_STATIC ${THIRDPARTY_LIBS} )
    set_target_properties(THERMODBCLIENT_STATIC PROPERTIES OUTPUT_NAME thermodbclient)
    install(TARGETS THERMODBCLIENT_STATIC DESTINATION lib)
endif()

# Install the header files preserving the directory hierarchy
install(DIRECTORY ${THERMODBCLIENT_SOURCE_DIR}
    DESTINATION include/thermodbclient
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")


