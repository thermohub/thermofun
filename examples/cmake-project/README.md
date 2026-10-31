# A CMake project that uses ThermoFun

This example shows how a project finds ThermoFun with CMake and links to it.

`CMakeLists.txt` uses these commands:

```cmake
find_package(ThermoFun REQUIRED)
target_link_libraries(app ThermoFun::ThermoFun)
```

To build and run:

```bash
cmake -S . -B build
cmake --build build
./build/app ../../Resources/databases/aq17-thermofun.json Quartz
```

If ThermoFun is not in a standard place, give it with `-DCMAKE_PREFIX_PATH=<install folder>`. The target gives the compiler C++20: your project does not have to set it.
