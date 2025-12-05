message(STATUS "Building python")

# Configure the pyproject.toml script with the provided CMake variables
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml)

add_subdirectory(src/thermofun)

# Install where the python package is installed to CMAKE_INSTALL_PREFIX if not given
if(NOT DEFINED THERMOFUN_PYTHON_INSTALL_PREFIX)

# Install the ThermoFun python package using setuptools
install(CODE
"
   if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun>
                ${CMAKE_CURRENT_BINARY_DIR}/src/thermofun/$<TARGET_FILE_NAME:PyThermoFun>
      )
   endif()

   execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} -vv
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
"
)

else()

# If the path is already in Windows format (with backslashes), it can't be added directly
# to the string below, otherwise CMake will later complain about "Invalid escape sequence".
file(TO_CMAKE_PATH "${THERMOFUN_PYTHON_INSTALL_PREFIX}" THERMOFUN_PYTHON_INSTALL_PREFIX)
message(STATUS "THERMOFUN_PYTHON_INSTALL_PREFIX: ${THERMOFUN_PYTHON_INSTALL_PREFIX}")

# Install the ThermoFun python package using setuptools
install(CODE
"
   file(TO_NATIVE_PATH \"${THERMOFUN_PYTHON_INSTALL_PREFIX}\" THERMOFUN_PYTHON_INSTALL_PREFIX_NATIVE)

   if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun>
                ${CMAKE_CURRENT_BINARY_DIR}/src/thermofun/$<TARGET_FILE_NAME:PyThermoFun>
        )
    endif()

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install --prefix \${THERMOFUN_PYTHON_INSTALL_PREFIX_NATIVE} ${CMAKE_CURRENT_BINARY_DIR} -vv
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
"
)

endif()

