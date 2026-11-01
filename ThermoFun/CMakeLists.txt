
# Recursively collect all header files from the current directory
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

#set(THERMOFUN_SHARED_LIB ${PROJECT_NAME}${SUFFIX_SHARED_LIBS})
#set(THERMOFUN_STATIC_LIB ${PROJECT_NAME}${SUFFIX_STATIC_LIBS})

# The constants of the methods: each file of Methods/ParameterSets goes into the library as bytes, so that they need no path when the library runs
file(GLOB PARAMETER_SET_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Methods/ParameterSets/*.yaml)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PARAMETER_SET_FILES})
set(PARAMETER_SET_ARRAYS "")
set(PARAMETER_SET_ROWS "")
set(PARAMETER_SET_COUNT 0)
foreach(file ${PARAMETER_SET_FILES})
    get_filename_component(name ${file} NAME)
    file(READ ${file} hex HEX)
    string(LENGTH "${hex}" length)
    math(EXPR bytes "${length} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex "${hex}")
    string(APPEND PARAMETER_SET_ARRAYS "const unsigned char file_${PARAMETER_SET_COUNT}[] = {${hex}};\n")
    string(APPEND PARAMETER_SET_ROWS "    {\"${name}\", file_${PARAMETER_SET_COUNT}, ${bytes}},\n")
    math(EXPR PARAMETER_SET_COUNT "${PARAMETER_SET_COUNT} + 1")
endforeach()
set(PARAMETER_SET_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/BuiltinParameterFiles.cpp)
file(CONFIGURE OUTPUT ${PARAMETER_SET_SOURCE} CONTENT "// Generated from Methods/ParameterSets: do not edit.
namespace ThermoFun::v2::detail {
struct BuiltinFile { const char* name; const unsigned char* text; unsigned long size; };
namespace { @PARAMETER_SET_ARRAYS@ }
extern const BuiltinFile builtin_parameter_files[] = {
@PARAMETER_SET_ROWS@};
extern const unsigned long builtin_parameter_file_count = @PARAMETER_SET_COUNT@;
}
" @ONLY)

# Create a library using the collected source files
add_library(ThermoFun ${HEADER_FILES} ${SOURCE_FILES} ${PARAMETER_SET_SOURCE})

# Add aliases for ThermoFun shared and static libraries
add_library(ThermoFun::ThermoFun ALIAS ThermoFun)

# The headers need C++20, also in the code of a user of the library
target_compile_features(ThermoFun PUBLIC cxx_std_20)

# Link ThermoFun against all required dependencies
target_link_libraries(ThermoFun PUBLIC ChemicalFun::ChemicalFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(ThermoFun PUBLIC
        $<BUILD_INTERFACE:autodiff::autodiff>
        $<INSTALL_INTERFACE:autodiff::autodiff>)
else()
    # plain numbers instead of autodiff::real (see Common/Real.hpp); also set for the projects that use ThermoFun
    target_compile_definitions(ThermoFun PUBLIC THERMOFUN_NO_AUTODIFF)
endif()

set_source_files_properties(ThermoEngine.cpp PROPERTIES COMPILE_DEFINITIONS "THERMOFUN_VERSION_STRING=\"${PROJECT_VERSION}\"")

# rapidyaml: only IO/Yaml/Yaml.cpp includes it, and its symbols stay inside the library
set_source_files_properties(IO/Yaml/Yaml.cpp PROPERTIES
    INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/third_party/rapidyaml
    COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden>")

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
# (the folders "detail" hold headers that use nlohmann_json; they are not installed, nor the inside of the engine)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ThermoFun
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.yaml" PATTERN "README.md"
    PATTERN "detail" EXCLUDE
    PATTERN "DatabaseImpl.h" EXCLUDE
    PATTERN "ThermoEngineImpl.h" EXCLUDE)
