# Collect the header and source files of PyThermoFun
file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Include the ThermoFun/python directory
include_directories(${PROJECT_SOURCE_DIR}/python)

# Create the PyThermoFun library
pybind11_add_module(PyThermoFun ${SOURCE_FILES} NO_EXTRAS) # NO_EXTRAS used to decrease linking time, but causes larger module file size

# Link PyThermoFun against ThermoFun library
target_link_libraries(PyThermoFun LINK_PRIVATE THERMOFUN_SHARED)

# Copy the Python extension module PyThermoFun to the build directory
add_custom_command(TARGET PyThermoFun POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PyThermoFun>
    $/../../$<TARGET_FILE_NAME:PyThermoFun>
    COMMENT "Copying the Python extension module thermofun to the build directory.")