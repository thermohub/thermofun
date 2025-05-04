# Copy the thermofun directory to the binary directory
file(COPY . DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Define a variable with the name of the compiled python module file as defined by pybind11 (which is used in the file setup.py.in)
set(THERMOFUN_PYTHON_MODULE_FILENAME PyThermoFun${PYTHON_MODULE_EXTENSION})

# Configure the setup.py file
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in)
set(SETUP_PY ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

# Configure the setup.py script with the provided CMake variables
configure_file(${SETUP_PY_IN} ${SETUP_PY})

# https://www.scivision.dev/cmake-install-python-package/
# https://pip.pypa.io/en/stable/topics/local-project-installs/#editable-installs
# detect virtualenv and set Pip args accordingly
if(DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX})
  set(_PIP_ARG)
else()
  set(_PIP_ARG "--user")
endif()
#message(STATUS "_PIP_ARG: ${_PIP_ARG}")

# Create a custom target to build thermofun python package during build stage
add_custom_target(thermofun ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun> 
        ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:PyThermoFun>
    #COMMAND ${PYTHON_EXECUTABLE} -m pip install ${_PIP_ARG} -e ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${PYTHON_EXECUTABLE} -m pip install --prefix ${CMAKE_BINARY_DIR} --ignore-installed .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Set dependencies of thermofun target
add_dependencies(thermofun PyThermoFun)

# Set the path where the python package is installed to CMAKE_INSTALL_PREFIX if not given
if(NOT DEFINED THERMOFUN_PYTHON_INSTALL_PREFIX)

# Install the thermofun python package using setuptools
install(CODE
"
    if(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/../../lib/PyThermoFun.pdb\")
        string(REPLACE \".pyd\" \".pdb\" THERMOFUN_PDB_FILENAME \"${THERMOFUN_PYTHON_MODULE_FILENAME}\")

        execute_process(
            COMMAND \${CMAKE_COMMAND} -E copy \"../../lib/PyThermoFun.pdb\" \"\${THERMOFUN_PDB_FILENAME}\"
            WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")
    endif()

    execute_process(
        COMMAND \"${PYTHON_EXECUTABLE}\" -m pip install \"${CMAKE_CURRENT_BINARY_DIR}\"
        WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")
"
)

else()

# If the path is already in Windows format (with backslashes), it can't be added directly
# to the string below, otherwise CMake will later complain about "Invalid escape sequence".
# Convert Windows-style path to CMake-style path with forward slashes
#file(TO_CMAKE_PATH "${THERMOFUN_PYTHON_INSTALL_PREFIX}" THERMOFUN_PYTHON_INSTALL_PREFIX_CMAKE)

# Install the thermofun python package using setuptools
install(CODE
"
    if(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/../../lib/PyThermoFun.pdb\")
        string(REPLACE \".pyd\" \".pdb\" THERMOFUN_PDB_FILENAME \"${THERMOFUN_PYTHON_MODULE_FILENAME}\")

        execute_process(
            COMMAND \${CMAKE_COMMAND} -E copy \"../../lib/PyThermoFun.pdb\" \"\${THERMOFUN_PDB_FILENAME}\"
            WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")
    endif()

    execute_process(
        COMMAND \"${PYTHON_EXECUTABLE}\" -m pip install --prefix \"${THERMOFUN_PYTHON_INSTALL_PREFIX}\" \"${CMAKE_CURRENT_BINARY_DIR}\"
        WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")
"
)
s

endif()

