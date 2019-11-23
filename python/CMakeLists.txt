# Find pybind11
#find_package(pybind11 2.3.0 REQUIRED)

# Build the Python wrappers of ThermoFun library
add_subdirectory(pyThermoFun)

# Build the Python package for ThermoFun
add_subdirectory(thermofun)
