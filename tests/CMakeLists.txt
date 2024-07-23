# Include the current directory so that cute test framework can be used
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add ReaktoroInterpreter tests to the build system
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(unit01)
add_subdirectory(unit02)
add_subdirectory(unit03)
add_subdirectory(unit04)
add_subdirectory(reactionsTest)
