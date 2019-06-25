# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Create a library using the collected source files
add_library(ThermoFun ${HEADER_FILES} ${SOURCE_FILES})

# Add aliases for ThermoFun shared and static libraries
add_library(ThermoFun::ThermoFun ALIAS ThermoFun)

# Find library and software dependencies
find_package(Boost 1.70.0 REQUIRED)
find_package(nlohmann_json 3.6.1 REQUIRED)

# Specify the dependency libraries to link against
target_link_libraries(ThermoFun
    PRIVATE Boost::boost
    PRIVATE nlohmann_json::nlohmann_json)

# Specify the include directories of the library target
target_include_directories(ThermoFun
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Create install target for library
install(TARGETS ThermoFun
    EXPORT ThermoFunTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install the header files preserving the directory hierarchy
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ThermoFun
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
