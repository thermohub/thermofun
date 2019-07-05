# Require a certain version of cmake
cmake_minimum_required(VERSION 3.0)

# Set the name of the project
project(ThermoFunGuiDemo VERSION 0.1.5 LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

add_definitions(-DFROM_SRC)

find_package(Qt5Widgets REQUIRED)
if ( NOT Qt5Widgets_FOUND )
  message(FATAL_ERROR "Package Qt5 required, but not found!")
endif( NOT Qt5Widgets_FOUND )

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package (Qt5WebEngineWidgets REQUIRED )
find_package (Qt5Charts REQUIRED )
# Add the include directories for the Qt 5 Widgets module to
# the compile lines.
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
include_directories(${Qt5WebEngineWidgets_INCLUDE_DIRS})

file(GLOB_RECURSE ThermoFunGuiDemo_SOURCES "*.cpp")
file(GLOB_RECURSE ThermoFunGuiDemo_HEADERS "*.h")

# Recursively collect all source files from the current directory
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB QRC_FILES ../*.qrc)

set (ThermoFunGuiDemo_INCLUDE_DIRS "")
foreach (_headerFile ${ThermoFunGuiDemo_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Foo_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ThermoFunGuiDemo_INCLUDE_DIRS)

# Set the list of compiler flags for gnu and clang compilers
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions("-std=c++11")
#    add_definitions("-std=c++11 -stdlib=libc++ -Wall -pedantic -Wno-variadic-macros -Wno-deprecated")
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_definitions("-fPIC")
    endif()
endif()

set(ThermoFunGuiDemo_FORMS ${UI_FILES})
set(RESOURCE ${QRC_FILES})

qt5_wrap_cpp(ThermoFunGuiDemo_HEADERS_MOC ${ThermoFunGuiDemo_HEADERS})
qt5_wrap_ui(ThermoFunGuiDemo_FORMS_HEADERS ${ThermoFunGuiDemo_FORMS})
qt5_add_resources(ThermoFunGuiDemo_SRCS ${RESOURCE})
#qt5_add_binary_resources(target inputfile ... OPTIONS ... DESTINATION ...)

include_directories(${CMAKE_BINARY_DIR}/ThermoFunGui/demo)
include_directories(${CMAKE_SOURCE_DIR}/../src/)
include_directories(${CMAKE_BINARY_DIR}/../../ThermoHubClient/)
#include_directories(${CMAKE_BINARY_DIR}/../)

add_executable(ThermoFunGuiDemo ${ThermoFunGuiDemo_SOURCES} ${ThermoFunGuiDemo_HEADERS_MOC} ${ThermoFunGuiDemo_FORMS_HEADERS} ${ThermoFunGuiDemo_SRCS})

target_link_libraries (ThermoFunGuiDemo jsonio jsonui jsonimpex yaml-cpp pugixml lua5.3 Qt5::Widgets Qt5::Core Qt5::WebEngineWidgets Qt5::Gui Qt5::PrintSupport Qt5::Charts boost_regex boost_system boost_filesystem curl velocypack thrift)
target_link_libraries (ThermoFunGuiDemo THERMOFUNGUI_STATIC)
target_link_libraries (ThermoFunGuiDemo THERMOHUBCLIENT_STATIC)
target_link_libraries (ThermoFunGuiDemo ThermoFun::ThermoFun)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ThermoFunGuiDemo.sh
        DESTINATION ${CMAKE_BINARY_DIR})

# Copy the binary file
add_custom_command(TARGET ThermoFunGuiDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ThermoFunGuiDemo>
    $/../../$<TARGET_FILE_NAME:ThermoFunGuiDemo>
    COMMENT "Copying ThermoFunGuiDemo to build directory.")


