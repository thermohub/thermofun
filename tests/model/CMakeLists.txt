# Unit tests of the model layer. Tests call runAll() functions that main.cpp lists.
# The frozen copy of the legacy parser is the reference of the equivalence test.
add_executable(tfun-unit
    main.cpp
    test_catalog.cpp
    test_diagnostics.cpp
    test_vocabulary.cpp
    test_ucum.cpp
    test_records.cpp
    test_legacy_equivalence.cpp
    frozen/ParseJsonToData_v073.cpp)
target_include_directories(tfun-unit PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tfun-unit PRIVATE ThermoFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(tfun-unit PRIVATE autodiff::autodiff)
endif()
add_test(NAME tfun-unit COMMAND tfun-unit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
