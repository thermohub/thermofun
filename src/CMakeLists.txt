# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Set the list of names of the third-party targets and libraries
set(THIRDPARTY_TARGETS JSONIO)
set(THIRDPARTY_LIBS jsonio boost_regex boost_filesystem curl velocypack)
set(THERMOFUN_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/)

# Create the shared library using the collected source files
add_library(THERMOFUN_OBJECTS OBJECT ${HEADER_FILES} ${SOURCE_FILES})

# Specify the dependencies of THERMOFUN to ensure proper build sequence
add_dependencies(THERMOFUN_OBJECTS ${THIRDPARTY_TARGETS})

# Check if a shared library is to be built
if(BUILD_SHARED_LIBS)
    add_library(THERMOFUN_SHARED SHARED $<TARGET_OBJECTS:THERMOFUN_OBJECTS>)
    target_link_libraries(THERMOFUN_SHARED ${THIRDPARTY_LIBS} )
    set_target_properties(THERMOFUN_SHARED PROPERTIES OUTPUT_NAME thermofun)
    install(TARGETS THERMOFUN_SHARED DESTINATION lib)
endif()

# Check if a static library is to be built
if(BUILD_STATIC_LIBS)
    add_library(THERMOFUN_STATIC STATIC $<TARGET_OBJECTS:THERMOFUN_OBJECTS>)
    target_link_libraries(THERMOFUN_STATIC ${THIRDPARTY_LIBS} )
    set_target_properties(THERMOFUN_STATIC PROPERTIES OUTPUT_NAME thermofun)
    install(TARGETS THERMOFUN_STATIC DESTINATION lib)
endif()

# Install the header files preserving the directory hierarchy
install(DIRECTORY ${THERMOFUN_SOURCE_DIR}
    DESTINATION include/thermofun
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

