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
add_test(NAME model-derivatives COMMAND test-model-derivatives ${CMAKE_SOURCE_DIR}/pytests/test-thermoengine-thermofun.json)

# The interface used by external codes is not changed
add_executable(test-interface src/interface.cpp)
target_include_directories(test-interface PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-interface PRIVATE ThermoFun autodiff::autodiff)
add_test(NAME interface COMMAND test-interface ${CMAKE_SOURCE_DIR}/pytests/test-thermoengine-thermofun.json)

# The analytical derivatives against references calculated with higher-order autodiff numbers
add_executable(test-analytic-derivatives src/reference_derivatives.cpp)
target_include_directories(test-analytic-derivatives PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test-analytic-derivatives PRIVATE ThermoFun autodiff::autodiff)
add_test(NAME analytic-derivatives COMMAND test-analytic-derivatives ${CMAKE_SOURCE_DIR}/pytests/test-thermoengine-thermofun.json)
