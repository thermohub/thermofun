# Specify the minimum cmake version
cmake_minimum_required(VERSION 3.16..4.2)

# Specify the name of your project
project(test03)

# Find the ThermoFun dependency.
find_package(ThermoFun REQUIRED)

# Create an executable named app with source code main.cpp
add_executable(test03 src/main.cpp)

# Link your application against ThermoFun C++ library.
target_link_libraries(test03 ThermoFun::ThermoFun)
