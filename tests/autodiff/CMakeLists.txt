# Tests of the autodiff infrastructure (autodiff::real passes, error and status propagation). Header-only, so this links
# only against autodiff and not against the ThermoFun library.
add_executable(test-autodiff src/main.cpp)
target_include_directories(test-autodiff PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-autodiff PRIVATE autodiff::autodiff)
add_test(NAME autodiff COMMAND test-autodiff)

# Derivatives of the properties calculated by some models against finite differences
add_executable(test-model-derivatives src/models.cpp)
target_include_directories(test-model-derivatives PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-model-derivatives PRIVATE ThermoFun autodiff::autodiff)
add_test(NAME model-derivatives COMMAND test-model-derivatives)
