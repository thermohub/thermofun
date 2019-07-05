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


# Set some necessary definitions to compile JSONUI
add_definitions(-DFROM_SRC)

set(ThermoFunGui_SOURCES ${SOURCE_FILES} )
set(ThermoFunGui_HEADERS ${HEADER_FILES} )
set(ThermoFunGui_FORMS ${UI_FILES})

qt5_wrap_cpp(ThermoFunGui_HEADERS_MOC ${ThermoFunGui_HEADERS})
qt5_wrap_ui(ThermoFunGui_FORMS_HEADERS ${ThermoFunGui_FORMS})

set(RESOURCE ${QRC_FILES})
qt5_add_resources(ThermoFunGui_SRCS ${RESOURCE})
#qt5_add_binary_resources(target inputfile ... OPTIONS ... DESTINATION ...)

include_directories(${CMAKE_BINARY_DIR}/ThermoFunGui/src)
include_directories(${CMAKE_BINARY_DIR}/../ThermoHubClient)
# access to thirdpary includes
include_directories(${CMAKE_BINARY_DIR}/../../../include)

# Create the shared library using the collected source files
add_library(THERMOFUNGUI_OBJECTS OBJECT ${ThermoFunGui_HEADERS} ${ThermoFunGui_SOURCES} ${ThermoFunGui_HEADERS_MOC} ${ThermoFunGui_FORMS_HEADERS} ${ThermoFunGui_SRCS} )
#target_link_libraries(BSONUI_OBJECTS Qt5::Widgets Qt5::Core Qt5::Gui)

# Specify the dependencies of BSONIO to ensure proper build sequence
#add_dependencies(ThermoFunGui_OBJECTS ${THIRDPARTY_TARGETS})

# Check if a shared library is to be built
if(TFUN_BUILD_SHARED_LIBS)
    add_library(THERMOFUNGUI_SHARED SHARED $<TARGET_OBJECTS:THERMOFUNGUI_OBJECTS>)
    target_link_libraries(THERMOFUNGUI_SHARED ${THIRDPARTY_LIBS} )
if( ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_link_libraries(THERMOFUNGUI_SHARED Qt5::Widgets Qt5::Core Qt5::WebEngineWidgets Qt5::Gui Qt5::PrintSupport Qt5::Charts thrift jsonio jsonui ThermoFun::ThermoFun ThermoHubClient )
endif()
    set_target_properties(THERMOFUNGUI_SHARED PROPERTIES OUTPUT_NAME ThermoFunGui)
    install(TARGETS THERMOFUNGUI_SHARED DESTINATION lib)
endif()

# Check if a static library is to be built
if(TFUN_BUILD_STATIC_LIBS)
    add_library(THERMOFUNGUI_STATIC STATIC $<TARGET_OBJECTS:THERMOFUNGUI_OBJECTS>)
    target_link_libraries(THERMOFUNGUI_STATIC ${THIRDPARTY_LIBS} )
    set_target_properties(THERMOFUNGUI_STATIC PROPERTIES OUTPUT_NAME ThermoFunGui)
    install(TARGETS THERMOFUNGUI_STATIC DESTINATION lib)
endif()

# Install the header files preserving the directory hierarchy
install(FILES ${HEADER_FILES}
        DESTINATION include/ThermoFunGui )
