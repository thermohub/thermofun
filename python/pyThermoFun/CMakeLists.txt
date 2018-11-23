# Set cmake version requirement
cmake_minimum_required(VERSION 3.0)

# Find pybind11 package, which is also going to find the python libraries and executable
find_package(pybind11 REQUIRED)

if(NOT pybind11_FOUND)
    set(BUILD_PYTHON OFF)
    message(WARNING "pybind11 was not found. "
        "Cannot build python wrappers and packages for ThremoFun.")
endif()
    
# Check if python was found
if(NOT PYTHONINTERP_FOUND)
    set(BUILD_PYTHON OFF)
    message(WARNING "Python executable was not found. "
        "Cannot build python wrappers and packages for ThermoFun.")
endif()
########

# Include the parent directory of pyThermoFun
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Collect the header and source files of PyThermoFun
file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Include the local Eigen directory so that pybind11 can work
#include_directories(${CMAKE_SOURCE_DIR}/Reaktoro/Math)

# Create the PyThermoFun library
pybind11_add_module(PyThermoFun ${SOURCE_FILES} NO_EXTRAS) # NO_EXTRAS used to decrease linking time, but causes larger module file size

#link_directories(/usr/local/lib)

# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoFun LINK_PRIVATE thermofun)

# Allow cotire to add Eigen header files to the generated prefix header file
#set_target_properties(PyReaktoro PROPERTIES
#    COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/Reaktoro/Math/Eigen")

# Apply cotire to PyReaktoro target to improve compilation time
#cotire(PyReaktoro)

# Create an install target for PyReaktoro
#install(TARGETS PyReaktoro DESTINATION "lib" COMPONENT interfaces)

# Copy the Python extension module PyReaktoro to the reaktoro/core directory
add_custom_command(TARGET PyThermoFun POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun>
    $../../../tests/$<TARGET_FILE_NAME:PyThermoFun>
    COMMENT "Copying the Python extension module thermofun to the /tests directory.")
