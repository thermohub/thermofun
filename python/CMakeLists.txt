# Find pybind11
find_package(pybind11 2.3.0 REQUIRED)

# Build the Python wrappers of ThermoFun library
add_subdirectory(pyThermoFun)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.py
        DESTINATION ${CMAKE_BINARY_DIR})

if(TFUN_BUILD_DBCLIENT)
    add_subdirectory(pyDBClient)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/test-dbc.py
        DESTINATION ${CMAKE_BINARY_DIR})
#else()
#    add_subdirectory(fungui EXCLUDE_FROM_ALL)
endif()

# Build the Python package for ThermoFun
# add_subdirectory(thermofun)
