# Copy the thermohubclient directory to the binary directory
file(COPY . DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Define a variable with the name of the compiled python module file as defined by pybind11 (which is used in the file setup.py.in)
set(THERMOHUBCLIENT_PYTHON_MODULE_FILENAME PyThermoHubClient${PYTHON_MODULE_EXTENSION})

# Configure the setup.py file
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

# Configure the setup.py script with the provided CMake variables
configure_file(${SETUP_PY_IN} ${SETUP_PY})

# Create a custom target to build thermohubclient python package during build stage
add_custom_target(thermohubclient ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoHubClient> 
        ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:PyThermoHubClient>
    COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} --quiet install --prefix=${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Set dependencies of thermohubclient target
add_dependencies(thermohubclient PyThermoHubClient)

# Set the path where the python package is installed to CMAKE_INSTALL_PREFIX if not given
if(NOT DEFINED THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX)
    set(THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()
# If the path is already in Windows format (with backslashes), it can't be added directly
# to the string below, otherwise CMake will later complain about "Invalid escape sequence".
file(TO_CMAKE_PATH "${THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX}" THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX)

# Install the thermohubclient python package using setuptools
install(CODE
"
    file(TO_NATIVE_PATH \"${THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX}\" THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX_NATIVE)

    if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../../lib/PyThermoHubClient.pdb)
        string(REPLACE .pyd .pdb THERMOHUBCLIENT_PDB_FILENAME \"${THERMOHUBCLIENT_PYTHON_MODULE_FILENAME}\")

        execute_process(
            COMMAND \${CMAKE_COMMAND} -E copy ../../lib/PyThermoHubClient.pdb \${THERMOHUBCLIENT_PDB_FILENAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install --prefix=\${THERMOHUBCLIENT_PYTHON_INSTALL_PREFIX_NATIVE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
"
)
