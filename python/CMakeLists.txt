# Check if a desired version for python has been provided
if(NOT PYBIND11_PYTHON_VERSION AND NOT PYTHON_EXECUTABLE)
    # If a prython version not given, use the default one in the system 
    find_package(PythonInterp REQUIRED) # PYTHON_EXECUTABLE variable is set here
endif()

# Find pybind11 using either a given PYTHON_EXECUTABLE or set previously
find_package(pybind11 REQUIRED)

# Build the Python wrappers of ThermoFun library
add_subdirectory(pyThermoFun)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.py
        DESTINATION ${CMAKE_BINARY_DIR})

# Build the Python package for ThermoFun
# add_subdirectory(thermofun)
