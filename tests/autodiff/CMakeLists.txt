# Tests of ThermoScalar built on autodiff::real. Header-only, so this links
# only against autodiff and not against the ThermoFun library.
add_executable(test-autodiff src/main.cpp)
target_include_directories(test-autodiff PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-autodiff PRIVATE autodiff::autodiff)
add_test(NAME autodiff COMMAND test-autodiff)
