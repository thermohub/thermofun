# Build gemcalc
file(GLOB_RECURSE CPPFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(CPPFILE ${CPPFILES})
    get_filename_component(CPPNAME ${CPPFILE} NAME_WE)
    add_executable(${CPPNAME} ${CPPFILE})
    target_link_libraries(${CPPNAME} THERMOFUN_SHARED)
    add_test(NAME ${CPPNAME} COMMAND ${CPPNAME})
endforeach()

add_custom_target(resources ALL COMMENT "Copy resources files to binary directory")
add_custom_command(
    TARGET resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/tests/dbclientTest/Resources
            ${CMAKE_BINARY_DIR}/tests/dbclientTest/Resources)
