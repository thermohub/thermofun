<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Install from the source

**You need:** a C++20 compiler, CMake, and the packages in `environment.devenv.yml`.

1. Configure and build. Use conda to get the dependencies, or see the `README.md`.

   ```
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j 4
   cmake --install build
   ```

2. Choose the options.

| CMake option | Default | What it does |
|---|---|---|
| `TFUN_BUILD_PYTHON` | ON | Builds the Python package `thermofun`. |
| `TFUN_BUILD_TESTS` | OFF | Builds the C++ tests. |
| `TFUN_USE_AUTODIFF` | ON | OFF uses plain numbers: no autodiff is needed, and all derivatives are 0. |
| `TFUN_USE_SYMENGINE` | OFF | ON runs [symbolic methods](add-a-method-as-data.md). It needs SymEngine. |
| `THERMOFUN_PHYSICAL_CONSTANTS` | `codata1986` | `codata2018` uses the current values of R, NA, F, e and k. See `ThermoFun/Methods/ParameterSets/`. |
| `BUILD_SHARED_LIBS`, `BUILD_STATIC_LIBS` | ON | The type of the library. |

Without autodiff, these properties are `notdefined`, because their value needs a derivative: `heat_capacity_cv` of substances, and `densityTP` of HGK water.

**Check:** run the tests.

```
ctest --test-dir build
python -m pytest
```

A project that uses `find_package(ThermoFun)` gets the same settings. See `examples/cmake-project/`.
