# Collect the header and source files of PyThermoFun
file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Include the ThermoFun/python directory
include_directories(${PROJECT_SOURCE_DIR}/python)

# Create the PyThermoFun library
pybind11_add_module(PyThermoDBClient ${SOURCE_FILES} NO_EXTRAS) # NO_EXTRAS used to decrease linking time, but causes larger module file size

# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoDBClient LINK_PRIVATE THERMOFUN_SHARED)
# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoDBClient LINK_PRIVATE THERMODBCLIENT_SHARED)

# Copy the Python extension module PyThermoFun to the build directory
add_custom_command(TARGET PyThermoDBClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoDBClient>
    $/../../$<TARGET_FILE_NAME:PyThermoDBClient>
    COMMENT "Copying the Python extension module PyThermoDBClient to the build directory.")
