# Regression tests for ThermoFun/Engine/ResultCache.h. Header-only, so this does
# not link against the ThermoFun library - unlike the standalone example
# projects in the sibling directories, which expect an installed ThermoFun.
add_executable(test-memoization src/main.cpp)
target_include_directories(test-memoization PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME memoization COMMAND test-memoization)
