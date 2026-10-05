# Usage: cmake -DBINARY_DIR=<ThermoFun build dir> -DSOURCE_DIR=<this dir>
#              -DWORK_DIR=<scratch dir> [-DPREFIX_PATH=<deps prefixes>] [-DGENERATOR=<g>]
#              [-DBUILD_TYPE=<cfg>] [-DPLATFORM=<arch>] [-DCONFIG=<cfg>] -P run.cmake
# BUILD_TYPE is for single-config generators, CONFIG (ctest -C) for multi-config ones (MSVC, Xcode).
string(REPLACE "|" ";" PREFIX_PATH "${PREFIX_PATH}")
file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

set(cfg_args)
if(GENERATOR)
    list(APPEND cfg_args -G ${GENERATOR})
endif()
if(PLATFORM)
    list(APPEND cfg_args -A ${PLATFORM})
endif()
if(BUILD_TYPE)
    list(APPEND cfg_args -DCMAKE_BUILD_TYPE=${BUILD_TYPE})
endif()
if(NOT CONFIG)
    set(CONFIG ${BUILD_TYPE})
endif()
if(NOT CONFIG)
    set(CONFIG Release)
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --install ${BINARY_DIR} --config ${CONFIG} --prefix ${WORK_DIR}/prefix
                RESULT_VARIABLE r)
if(r)
    message(FATAL_ERROR "install failed")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -S ${SOURCE_DIR} -B ${WORK_DIR}/build ${cfg_args}
                        -DTHERMOFUN_PREFIX=${WORK_DIR}/prefix "-DCMAKE_PREFIX_PATH=${PREFIX_PATH}"
                RESULT_VARIABLE r)
if(r)
    message(FATAL_ERROR "configure failed")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build ${WORK_DIR}/build --config ${CONFIG} -j RESULT_VARIABLE r)
if(r)
    message(FATAL_ERROR "an installed header does not compile on its own")
endif()
