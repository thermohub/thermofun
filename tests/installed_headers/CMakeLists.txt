# Compiles every installed ThermoFun header on its own, against an install
# prefix (THERMOFUN_PREFIX) with only <prefix>/include on the include path.
# Catches includes that only resolve inside the source tree, e.g.
# #include "ThermoProperties.h" from a subdirectory. Standalone project: it uses
# find_package(ThermoFun), so it is driven by tests/installed_headers/run.cmake.
cmake_minimum_required(VERSION 3.16)
project(installed-headers CXX)

find_package(ThermoFun REQUIRED CONFIG PATHS ${THERMOFUN_PREFIX} NO_DEFAULT_PATH)

set(inc ${THERMOFUN_PREFIX}/include)
file(GLOB_RECURSE headers RELATIVE ${inc} ${inc}/ThermoFun/*.h ${inc}/ThermoFun/*.hpp)
list(LENGTH headers n)
if(n EQUAL 0)
    message(FATAL_ERROR "No ThermoFun headers found under ${inc}")
endif()

set(sources)
foreach(h ${headers})
    string(MAKE_C_IDENTIFIER ${h} id)
    set(cpp ${CMAKE_CURRENT_BINARY_DIR}/gen/${id}.cpp)
    file(WRITE ${cpp} "#include <${h}>\n")
    list(APPEND sources ${cpp})
endforeach()

add_library(installed-headers OBJECT ${sources})
target_link_libraries(installed-headers PRIVATE ThermoFun::ThermoFun)
target_compile_features(installed-headers PRIVATE cxx_std_20)
