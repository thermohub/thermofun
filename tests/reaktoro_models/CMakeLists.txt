# Standard property models taken from Reaktoro: Holland and Powell (2011) Tait EoS, Extended UNIQUAC, NASA polynomials,
# interpolation over a table of T and P. The reference values come from Reaktoro (make_reference.py)
add_executable(test-reaktoro-models src/main.cpp)
target_include_directories(test-reaktoro-models PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-reaktoro-models PRIVATE ThermoFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(test-reaktoro-models PRIVATE autodiff::autodiff)
endif()
add_test(NAME reaktoro-models COMMAND test-reaktoro-models
    ${CMAKE_CURRENT_SOURCE_DIR}/reaktoro_models_db.json ${CMAKE_CURRENT_SOURCE_DIR}/reaktoro_reference.csv)
