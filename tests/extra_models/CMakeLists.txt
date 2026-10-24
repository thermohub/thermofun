# Tests the standard property models of the extra models: Tait EoS (Holland and Powell, 2011), Extended UNIQUAC,
# NASA polynomials and interpolation over a table of T and P. The reference values are in extra_reference.csv.
add_executable(test-extra-models src/main.cpp)
target_include_directories(test-extra-models PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-extra-models PRIVATE ThermoFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(test-extra-models PRIVATE autodiff::autodiff)
endif()
add_test(NAME extra-models COMMAND test-extra-models
    ${CMAKE_CURRENT_SOURCE_DIR}/extra_models_db.json ${CMAKE_CURRENT_SOURCE_DIR}/extra_reference.csv)
