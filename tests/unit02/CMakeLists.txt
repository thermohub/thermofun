# Specify the minimum cmake version
cmake_minimum_required(VERSION 3.9)

# Specify the name of your project
project(test02)

# Find the ThermoFun dependency.
find_package(ThermoFun REQUIRED)

# Create an executable named app with source code main.cpp
add_executable(test02 src/main.cpp)

# Link your application against ThermoFun C++ library.
target_link_libraries(test02 ThermoFun::ThermoFun)
