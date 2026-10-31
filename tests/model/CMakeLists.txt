find_package(Threads REQUIRED)

# Unit tests of the model layer. Tests call runAll() functions that main.cpp lists.
# The frozen copy of the legacy parser is the reference of the equivalence test.
add_executable(tfun-unit
    main.cpp
    test_catalog.cpp
    ../methods/test_substance_methods.cpp
    test_diagnostics.cpp
    test_vocabulary.cpp
    test_ucum.cpp
    test_records.cpp
    test_format_detect.cpp
    test_normalize.cpp
    test_sha256.cpp
    test_legacy_reader.cpp
    test_legacy_equivalence.cpp
    frozen/ParseJsonToData_v073.cpp)
target_include_directories(tfun-unit PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tfun-unit PRIVATE ThermoFun)
if(TFUN_USE_AUTODIFF)
    target_link_libraries(tfun-unit PRIVATE autodiff::autodiff)
endif()
add_test(NAME tfun-unit COMMAND tfun-unit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Records never change, and threads can share them. The ThreadSanitizer job runs this program.
add_executable(test-records-immutable test_records_immutable.cpp)
target_include_directories(test-records-immutable PRIVATE ${CMAKE_SOURCE_DIR}/ThermoFun)
target_link_libraries(test-records-immutable PRIVATE ThermoFun Threads::Threads)
add_test(NAME records-immutable COMMAND test-records-immutable WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
