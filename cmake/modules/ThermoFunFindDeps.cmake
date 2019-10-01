# Find Boost library
#find_package(Boost REQUIRED)


# Find pybind11 library (if needed)
if(TFUN_BUILD_PYTHON)
    find_package(pybind11 REQUIRED)
    if(NOT pybind11_FOUND)
        message(WARNING "Could not find pybind11 - the Python module `thermofun` will not be built.")
        set(TFUN_BUILD_PYTHON OFF)
    else()
        message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
    endif()
endif()
