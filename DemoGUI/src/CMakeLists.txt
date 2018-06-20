
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

set(CMAKE_AUTORCC ON)

# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB QRC_FILES ../*.qrc)

# Set the list of compiler flags for gnu and clang compilers
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions("-std=c++11")
#    add_definitions("-std=c++11 -stdlib=libc++ -Wall -pedantic -Wno-variadic-macros -Wno-deprecated")
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_definitions("-fPIC")
    endif()
endif()


# Set some necessary definitions to compile BSONUI
#add_definitions(-D_MYNOZLIB -DFROM_SRC)

set(thermofungui_SOURCES ${SOURCE_FILES} )
set(thermofungui_HEADERS ${HEADER_FILES} )
set(thermofungui_FORMS ${UI_FILES})

qt5_wrap_cpp(thermofungui_HEADERS_MOC ${thermofungui_HEADERS})
qt5_wrap_ui(thermofungui_FORMS_HEADERS ${thermofungui_FORMS})

set(RESOURCE ${QRC_FILES})
qt5_add_resources(thermofungui_SRCS ${RESOURCE})
#qt5_add_binary_resources(target inputfile ... OPTIONS ... DESTINATION ...)

include_directories(${CMAKE_BINARY_DIR}/DemoGUI/src)

# Create the shared library using the collected source files
add_library(THERMOFUNGUI_OBJECTS OBJECT ${thermofungui_HEADERS} ${thermofungui_SOURCES} ${thermofungui_HEADERS_MOC} ${thermofungui_FORMS_HEADERS} ${thermofungui_SRCS} )
#target_link_libraries(BSONUI_OBJECTS Qt5::Widgets Qt5::Core Qt5::Gui)

# Specify the dependencies of BSONIO to ensure proper build sequence
#add_dependencies(thermofungui_OBJECTS ${THIRDPARTY_TARGETS})

# Check if a shared library is to be built
if(BUILD_SHARED_LIBS)
    add_library(THERMOFUNGUI_SHARED SHARED $<TARGET_OBJECTS:THERMOFUNGUI_OBJECTS>)
    target_link_libraries(THERMOFUNGUI_SHARED ${THIRDPARTY_LIBS} )
if( ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  target_link_libraries(THERMOFUNGUI_SHARED Qt5::Widgets Qt5::Core Qt5::WebEngineWidgets Qt5::Gui Qt5::PrintSupport Qt5::Charts thrift  )
endif()
    set_target_properties(THERMOFUNGUI_SHARED PROPERTIES OUTPUT_NAME thermofungui)
    install(TARGETS THERMOFUNGUI_SHARED DESTINATION lib)
endif()

# Check if a static library is to be built
if(BUILD_STATIC_LIBS)
    add_library(THERMOFUNGUI_STATIC STATIC $<TARGET_OBJECTS:THERMOFUNGUI_OBJECTS>)
    target_link_libraries(THERMOFUNGUI_STATIC ${THIRDPARTY_LIBS} )
    set_target_properties(THERMOFUNGUI_STATIC PROPERTIES OUTPUT_NAME thermofungui)
    install(TARGETS THERMOFUNGUI_STATIC DESTINATION lib)
endif()

# Install the header files preserving the directory hierarchy
install(FILES ${HEADER_FILES}
        DESTINATION include/thermofungui )
