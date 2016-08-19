# Require a certain version of cmake
cmake_minimum_required(VERSION 2.8)

# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Set the list of compiler flags for gnu and clang compilers
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions("-std=c++11 -Wall -pedantic -Wno-variadic-macros -Wno-deprecated")
#    add_definitions("-std=c++11 -stdlib=libc++ -Wall -pedantic -Wno-variadic-macros -Wno-deprecated")
   if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
       add_definitions("-fPIC")
   endif()
endif()


# Set some necessary definitions to compile TCORRPT
# add_definitions(-D_MYNOZLIB)

# Set the list of names of the third-party targets and libraries
set(THIRDPARTY_TARGETS BSONIO)
set(THIRDPARTY_LIBS bsonio)
set(TCORRPT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/)

# Create the shared library using the collected source files
add_library(TCORRPT_OBJECTS OBJECT ${HEADER_FILES} ${SOURCE_FILES})

# Specify the dependencies of TCORRPT to ensure proper build sequence
add_dependencies(TCORRPT_OBJECTS ${THIRDPARTY_TARGETS})

# Check if a shared library is to be built
if(BUILD_SHARED_LIBS)
    add_library(TCORRPT_SHARED SHARED $<TARGET_OBJECTS:TCORRPT_OBJECTS>)
    target_link_libraries(TCORRPT_SHARED ${THIRDPARTY_LIBS} )
if( ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_link_libraries(TCORRPT_SHARED thrift lua bsonio)
endif()
    set_target_properties(TCORRPT_SHARED PROPERTIES OUTPUT_NAME tcorrpt)
    install(TARGETS TCORRPT_SHARED DESTINATION lib)
endif()

# Check if a static library is to be built
if(BUILD_STATIC_LIBS)
    add_library(TCORRPT_STATIC STATIC $<TARGET_OBJECTS:TCORRPT_OBJECTS>)
    target_link_libraries(TCORRPT_STATIC ${THIRDPARTY_LIBS} )
    set_target_properties(TCORRPT_STATIC PROPERTIES OUTPUT_NAME tcorrpt)
    install(TARGETS TCORRPT_STATIC DESTINATION lib)
endif()

# Install the header files preserving the directory hierarchy
install(DIRECTORY ${TCORRPT_SOURCE_DIR}
    DESTINATION include/tcorrpt
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

#install(FILES ${HEADER_FILES}
#        DESTINATION include/tcorrpt )
#install(DIRECTORY ${CMAKE_SOURCE_DIR} 
#    DESTINATION include/tocrrpt
#    FILES_MATCHING PATTERN "*.h")

