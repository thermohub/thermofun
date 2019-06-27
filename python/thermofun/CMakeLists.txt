# Copy the thermofun directory to the binary directory
file(COPY . DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Define a variable with the name of the compiled python module file as defined by pybind11 (which is used in the file setup.py.in)
set(THERMOFUN_PYTHON_MODULE_FILENAME PyThermoFun${PYTHON_MODULE_EXTENSION})

# Configure the setup.py file
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
#set(SETUP_PY ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

# Configure the setup.py script with the provided CMake variables
#configure_file(${SETUP_PY_IN} ${SETUP_PY})
configure_file(${SETUP_PY_IN})

# Create a custom target to build thermofun python package during build stage
add_custom_target(thermofun ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun> 
        ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:PyThermoFun>
    COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} --quiet install --prefix=${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Set dependencies of thermofun target
add_dependencies(thermofun PyThermoFun)

# Set the path where the python package is installed to CMAKE_INSTALL_PREFIX if not given
if(NOT DEFINED THERMOFUN_PYTHON_INSTALL_PREFIX)
    set(THERMOFUN_PYTHON_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

# Install the thermofun python package using setuptools
install(CODE
"
	file(TO_NATIVE_PATH \"${THERMOFUN_PYTHON_INSTALL_PREFIX}\" THERMOFUN_PYTHON_INSTALL_PREFIX_NATIVE)
	   
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install --prefix=\${THERMOFUN_PYTHON_INSTALL_PREFIX_NATIVE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
"
)
