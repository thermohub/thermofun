cmake_minimum_required(VERSION 2.8)

set(CMAKE_AUTORCC ON)

project(guidemo)

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

file(GLOB_RECURSE guidemo_SOURCES "*.cpp")
file(GLOB_RECURSE guidemo_HEADERS "*.h")

# Recursively collect all source files from the current directory
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB QRC_FILES ../*.qrc)

set (guidemo_INCLUDE_DIRS "")
foreach (_headerFile ${guidemo_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Foo_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES guidemo_INCLUDE_DIRS)

# Set the list of compiler flags for gnu and clang compilers
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions("-std=c++11")
#    add_definitions("-std=c++11 -stdlib=libc++ -Wall -pedantic -Wno-variadic-macros -Wno-deprecated")
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_definitions("-fPIC")
    endif()
endif()

set(guidemo_FORMS ${UI_FILES})
set(RESOURCE ${QRC_FILES})

qt5_wrap_cpp(guidemo_HEADERS_MOC ${guidemo_HEADERS})
qt5_wrap_ui(guidemo_FORMS_HEADERS ${guidemo_FORMS})
qt5_add_resources(guidemo_SRCS ${RESOURCE})
#qt5_add_binary_resources(target inputfile ... OPTIONS ... DESTINATION ...)

include_directories(${CMAKE_BINARY_DIR}/fungui/demo)
include_directories(${CMAKE_SOURCE_DIR}/../src/)
#include_directories(${CMAKE_BINARY_DIR}/../)

add_executable(guidemo ${guidemo_SOURCES} ${guidemo_HEADERS_MOC} ${guidemo_FORMS_HEADERS} ${guidemo_SRCS})

target_link_libraries (guidemo jsonio jsonui Qt5::Widgets Qt5::Core Qt5::WebEngineWidgets Qt5::Gui Qt5::PrintSupport Qt5::Charts boost_regex boost_system boost_filesystem curl velocypack thrift)
target_link_libraries (guidemo THERMOFUNGUI_STATIC)
target_link_libraries (guidemo THERMOFUN_STATIC)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources
        DESTINATION ${CMAKE_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/guidemo.sh
        DESTINATION ${CMAKE_BINARY_DIR})

# Copy the binary file
add_custom_command(TARGET guidemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:guidemo>
    $/../../$<TARGET_FILE_NAME:guidemo>
    COMMENT "Copying guidemo to build directory.")


