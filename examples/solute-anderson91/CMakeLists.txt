# Specify the minimum cmake version
cmake_minimum_required(VERSION 3.9)

# Specify the name of your project
project(solute-anderson91-app)

# Find the ThermoFun dependency.
find_package(ThermoFun REQUIRED)

# Create an executable named app with source code main.cpp
add_executable(app main.cpp)

# Link your application against ThermoFun C++ library.
target_link_libraries(app ThermoFun::ThermoFun)
