# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

if(TFUN_BUILD_SHARED_LIBS)
    set(THERMOFUN_SHARED_LIB ${PROJECT_NAME}${SUFFIX_SHARED_LIBS})
    # Enable automatic creation of a module definition (.def) file for a SHARED library on Windows.
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    #add_library(XGEMS_SHARED SHARED $<TARGET_OBJECTS:XGEMS_OBJECT>)
    #target_link_libraries(XGEMS_SHARED GEMS3K-static)
    #set_target_properties(XGEMS_SHARED PROPERTIES OUTPUT_NAME xGEMS)
    #install(TARGETS XGEMS_SHARED DESTINATION "lib" COMPONENT libraries)
endif()

# Check if a static library must be built
if(TFUN_BUILD_STATIC_LIBS)
    set(THERMOFUN_STATIC_LIB ${PROJECT_NAME}${SUFFIX_STATIC_LIBS})
    #add_library(XGEMS_STATIC STATIC $<TARGET_OBJECTS:XGEMS_OBJECT>)
    #target_link_libraries(XGEMS_STATIC GEMS3K-static)
    #set_target_properties(XGEMS_STATIC PROPERTIES OUTPUT_NAME xGEMS)
    #install(TARGETS XGEMS_STATIC DESTINATION "lib" COMPONENT libraries)
endif()

# Create a library using the collected source files
add_library(ThermoFun ${HEADER_FILES} ${SOURCE_FILES})
# Add aliases for ThermoFun shared and static libraries
add_library(ThermoFun::ThermoFun ALIAS ThermoFun)

if(ChemicalFun_FOUND)
    target_link_libraries(ThermoFun PUBLIC ChemicalFun::ChemicalFun)
endif()

#if(fmt_FOUND)
#    target_link_libraries(ThermoFun PRIVATE fmt::fmt)
#endif()


if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    #link_directories("/usr/local/lib")
    if(spdlog_FOUND)
        target_link_libraries(ThermoFun PRIVATE spdlog::spdlog)
    endif()
else() 
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
# build options for MacOS
    if(fmt_FOUND)
    target_link_libraries(ThermoFun PRIVATE fmt::fmt)
    endif()
    if(spdlog_FOUND)
        set(SPDLOG_FMT_EXTERNAL ON)
        target_link_libraries(ThermoFun PRIVATE spdlog::spdlog)
    endif()
    else() 
# build options for Linux
# Find library and software dependencies
        #find_package(Boost 1.58.0 REQUIRED)
        find_package(nlohmann_json REQUIRED)
# Specify the dependency libraries to link against
        target_link_libraries(ThermoFun
#        PRIVATE Boost::boost
         PRIVATE nlohmann_json::nlohmann_json)
    endif()
endif()

# Specify the include directories of the library target
target_include_directories(ThermoFun
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Create install target for library
install(TARGETS ThermoFun
    EXPORT ThermoFunTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries)
    
# Install debug symbols
if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:ThermoFun>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT libraries
        OPTIONAL
    )
endif()

# Install the header files preserving the directory hierarchy
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ThermoFun
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
