message(STATUS "Building python library ")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Collect the source files of PyThermoFun
file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Include the ThermoFun/python directory
include_directories(${PROJECT_SOURCE_DIR}/python)

# Create the PyThermoFun library
pybind11_add_module(PyThermoFun ${SOURCE_FILES} NO_EXTRAS) # NO_EXTRAS used to decrease linking time, but causes larger module file size

# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoFun PUBLIC ThermoFun::ThermoFun)
