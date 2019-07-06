# Collect the header and source files of PyThermoFun
file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Include the ThermoFun/python directory
include_directories(${PROJECT_SOURCE_DIR}/python)

# Create the PyThermoFun library
pybind11_add_module(PyThermoHubClient ${SOURCE_FILES} NO_EXTRAS) # NO_EXTRAS used to decrease linking time, but causes larger module file size

# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoHubClient LINK_PRIVATE ThermoFun::ThermoFun)
# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoHubClient LINK_PRIVATE THERMOHUBCLIENT_SHARED)

# Copy the Python extension module PyThermoFun to the build directory
add_custom_command(TARGET PyThermoHubClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoHubClient>
    $/../../$<TARGET_FILE_NAME:PyThermoHubClient>
    COMMENT "Copying the Python extension module PyThermoHubClient to the build directory.")
