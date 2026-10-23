# Substances exported from a database: volume equations, a water solvent with constant Cp,
# a gas defined by a reaction and a CG fluid
add_executable(test-gems-export src/main.cpp)
target_include_directories(test-gems-export PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-gems-export PRIVATE ThermoFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(test-gems-export PRIVATE autodiff::autodiff)
endif()
add_test(NAME gems-export COMMAND test-gems-export ${CMAKE_CURRENT_SOURCE_DIR}/gems_export_data.json)
