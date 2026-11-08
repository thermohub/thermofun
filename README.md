# ThermoFun

[![ci-build](https://github.com/thermohub/thermofun/actions/workflows/ci-build.yml/badge.svg)](https://github.com/thermohub/thermofun/actions/workflows/ci-build.yml)
[![DOI](https://joss.theoj.org/papers/10.21105/joss.04624/status.svg)](https://doi.org/10.21105/joss.04624)

ThermoFun calculates the standard-state thermodynamic properties of substances and reactions at a temperature and a
pressure: G, H, S, Cp, V, log K and others, with their derivatives with respect to T and P, their uncertainties and a
status. It is a C++ library with a Python package, and is used by GEMS3K, GEM-Selektor, xGEMS and Reaktoro.

- **Data:** ThermoHub files of format 2.0 (YAML or JSON, one record or a whole dataset), and the legacy ThermoFun JSON
  files, which convert to format 2.0 with `python -m thermofun.convert`.
- **Methods:** heat-capacity integrals, volume and Landau terms of minerals, equations of state of fluids, HKF and other
  models of aqueous species, models of water and its dielectric constant, log K functions of reactions. Each method also
  runs alone, without a database.
- **Units:** SI in and out: T in K, P in Pa, energies in J/mol, V in m3/mol.

**[Documentation](docs/index.md):** tutorials, how-to pages, the format and the methods.
Coming from a version before 1.0: read [Migrate from the old API](docs/how-to/migrate-from-the-old-api.md) and the
[CHANGELOG](CHANGELOG.md). Some results change, and the volume is now in m3/mol.

If you use ThermoFun in your work, please cite the [JOSS paper](https://doi.org/10.21105/joss.04624).

## A first calculation in Python

A record in format 2.0, in the file `calcite.yaml`:

```yaml
thermohub: 2.0.0-draft.1
substances:
  Calcite:
    formula: CaCO3
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: "-1129176 J/mol", H: "-1207370 J/mol", S: "91.71 J/(mol*K)", Cp: "83.47 J/(mol*K)", V: "36.934 cm3/mol"}
    methods:
      - method: CpPolynomial
        segments:
          - {t_min: 298.15, t_max: 1200, parameters: {a0: 104.52, a1: 0.02192, a2: -2.59408e6}}
      - method: ConstantVolume
```

```python
import thermofun as tf

engine = tf.ThermoEngine("calcite.yaml")
p = engine.thermoPropertiesSubstance(373.15, 1e7, "Calcite")   # T in K, P in Pa
print(p.gibbs_energy.val, p.gibbs_energy.ddt, p.volume.val)    # J/mol, dG/dT = -S in J/(mol K), m3/mol

engine = tf.ThermoEngine("Resources/databases/aq17-thermofun.json")   # a legacy file
water = engine.thermoPropertiesSubstance(373.15, 0, "H2O@")          # P = 0 is the saturation pressure
print(water.gibbs_energy.val, engine.saturationPressure(373.15))

batch = tf.ThermoBatch("Resources/databases/aq17-thermofun.json")    # tables of many points
batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
batch.thermoPropertiesSubstance([[25, 1], [100, 1], [200, 100]], ["Al+3", "OH-", "SiO2@"],
                                ["gibbs_energy", "entropy", "volume"]).toCSV("results.csv")
```

## In C++

```cpp
#include <ThermoFun/ThermoFun.h>
#include <iostream>

int main()
{
    ThermoFun::ThermoEngine engine("Resources/databases/aq17-thermofun.json");
    double P = 1e8;                                              // Pa; 0 means the saturation pressure, written back here
    const auto p = engine.thermoPropertiesSubstance(373.15, P, "H2O@");
    std::cout << p.gibbs_energy.val << " " << p.gibbs_energy.ddt << " " << p.volume.val << "\n";
}
```

In CMake: `find_package(ThermoFun)` and `target_link_libraries(app ThermoFun::ThermoFun)`. Include only
`ThermoFun/ThermoFun.h`: the other installed headers are what it includes.

## Install

With conda:

```bash
conda install -c conda-forge thermofun
```

From the source, with the dependencies from conda (Linux, macOS and Windows):

```bash
conda install -n base conda-devenv
git clone https://github.com/thermohub/thermofun.git && cd thermofun
conda devenv                      # makes the environment thermofun from environment.devenv.yml
conda activate thermofun
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

The CMake options are in [Install from the source](docs/how-to/install-from-source.md). ThermoFun needs a C++20
compiler, ChemicalFun, spdlog, nlohmann_json and autodiff (fetched from GitHub if CMake does not find it), and pybind11 for
Python.

Data from the ThermoHub database: [thermohubclient](https://github.com/thermohub/thermohubclient)
(`conda install -c conda-forge thermohubclient`).

## License

ThermoFun is free software under the GNU Lesser General Public License, version 3 or (at your option) any later version
([LICENSE](LICENSE); the GNU GPL v3 it refers to is in [LICENSE.GPL-3.0](LICENSE.GPL-3.0)). Files carry a short
`SPDX-License-Identifier` line. Some files are derived from other projects (Reaktoro, GEMS3K) and keep the notice of
their origin: see [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

The license is compatible with [Reaktoro](https://github.com/reaktoro/reaktoro) (LGPL 2.1 or later), which uses ThermoFun:
the files derived from Reaktoro are LGPL-2.1-or-later, and the rest of ThermoFun is LGPL-3.0-or-later, which the "or
later" of Reaktoro's license allows to combine.

## Bugs and contributions

Report a bug at [ThermoFun's issues](https://github.com/thermohub/thermofun/issues/new), with a small example that shows
it. To contribute, fork the repository, make your change in a branch, and open a pull request to
`thermohub/thermofun`. The [contributing pages](docs/contributing/golden-tests.md) say how the results are checked.
