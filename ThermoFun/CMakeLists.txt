
# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

if(TFUN_BUILD_SHARED_LIBS)
    set(THERMOFUN_SHARED_LIB ${PROJECT_NAME} ${SUFFIX_SHARED_LIBS})
    # Enable automatic creation of a module definition (.def) file for a SHARED library on Windows.
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# Check if a static library must be built
if(TFUN_BUILD_STATIC_LIBS)
    set(THERMOFUN_STATIC_LIB ${PROJECT_NAME}${SUFFIX_STATIC_LIBS})
endif()

# Create a library using the collected source files
add_library(ThermoFun ${HEADER_FILES} ${SOURCE_FILES})

# Add aliases for ThermoFun shared and static libraries
add_library(ThermoFun::ThermoFun ALIAS ThermoFun)

# Link ThermoFun against all required dependencies
target_link_libraries(ThermoFun
    PUBLIC ChemicalFun::ChemicalFun
)

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
