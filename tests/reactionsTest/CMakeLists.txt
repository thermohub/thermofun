# Specify the minimum cmake version
cmake_minimum_required(VERSION 3.16..4.2)

# Specify the name of your project
project(reactions)

# Find the ThermoFun dependency.
find_package(ThermoFun REQUIRED)

# Create an executable named app with source code main.cpp
add_executable(reactions src/main.cpp)

# Link your application against ThermoFun C++ library.
target_link_libraries(reactions ThermoFun::ThermoFun)
