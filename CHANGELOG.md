# Changelog

All notable changes to ThermoFun are in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Standard property models: Tait HP11 pressure correction, Extended UNIQUAC, NASA polynomials,
  and T-P table interpolation (from the branch `new_models`).
- Golden reference tests in `pytests/golden/`. They check that values, derivatives, errors and
  status codes do not change.
- Test of the legacy keys and method numbers (`pytests/golden/test_key_inventory.py`).
- Synthetic legacy fixtures for the methods that no bundled database uses (`pytests/data/legacy-synthetic/`).
- Comparison of two builds (`pytests/golden/compare_builds.py`) and the CI job `golden-diff`.
- Guide to the golden tests (`docs/contributing/golden-tests.md`).
- `ThermoEngine::setConventions` (C++ and Python). In Python, `engine.preferences` and `engine.conventions`
  can be assigned as a whole.
- `Database::revision`: a number that changes with each change of the records.

### Fixed

- Cached results are no longer returned after a change of the conventions, the preferences, the database
  conventions or the records. Before, a change in place (for example `engine.conventions.water_properties`)
  or `appendData` after a first calculation gave the old result.
- The installed CMake package config finds its dependencies before it loads the targets, and gives autodiff the
  Eigen headers when Eigen is found. Before, `find_package(ThermoFun)` failed in a project that had not found
  autodiff itself, and a ThermoFun built with a fetched autodiff could not be used where Eigen is installed.
- The parser read an uninitialised value when a substance has no `mass_per_mole` (or an element no entropy).
- `water_eos_iapws95_gems` in legacy files was ignored, so water gave no values. It is now read as IAPWS-95 (the Reaktoro
  code, as there is no GEMS code), with a warning.
- The status of Cv of a T, P table without Cp was `calculated`, with the value 0. It is now not defined.
- The Zhang and Duan (2005) water model now stops at 2000 K, the limit of the paper (it was 2273.15 K).
- The Cp polynomial used `a9` without the factor `sqrt(T)`, because of an integer division. Only `BaI2(g)` of heracles has this term.
- Cached results no longer depend on the calculation order. The reactants of a reaction-defined substance are
  calculated with the gas pressure correction on, and they shared the cache with plain calculations. For example,
  H2(g) at 273.15 K and 100 bar gave G = 5484.36 J/mol instead of 3235.96 J/mol after Fe4(OH)8Clwn(s).
- The Dolejš and Manning (2010) density model used the parameter `a` in kJ/mol in a formula in J/mol, so ΔrG and
  ΔrH were wrong by `a`. This model and the Marshall and Franck (1981) model now return log K (it was 0).
  Both are checked against the values published with them.

- Diagnostics (`Common/Diagnostics.h`): problems found while reading or checking data are collected with a code,
  the file, line and column, the record, the path in the record, a message and a fix. Every code has a fix text.

- Record model, first part (`Model/`, namespace `ThermoFun::v2`): quantities with uncertainty and typed vocabularies.
  Each list has one checked conversion, with aliases and a "did you mean" hint.

- Method catalog (`Methods/`): the specification of every built-in method, with the approved ids, the older ids as
  aliases, the parameters with SI and source units, the effects, the needs and the classes a method suits.

- Legacy reader (`IO/Legacy/`): reads legacy records into the record model, with a report of all fix-ups and of what
  is not used. `Substance`, `Reaction` and `Element` have `record()`.

- Method objects (`Methods/`, `thermofun.methods` in Python): a method is an object bound to its parameters, that runs
  alone with no database and no engine, and gives its properties with the derivatives with respect to T, P and each
  parameter. First methods: `ConstantProperties`, `ConstantCp`, `CpPolynomial`.

- `ThermoEngine` calculates a substance with method objects when all its methods have code. `engine.explain(T, P, symbol)`
  gives the state after each method. The results are the same as before.
- Water and its dielectric models are method objects too (`thermofun.methods.solventMethod`, `dielectricMethod`), and
  run alone. New option of `WaterZhangDuan05`: `dew_gibbs` gives the Gibbs energy of water of the DEW model.
- The aqueous solute methods are method objects (`thermofun.methods.substanceMethod`): `HKF88`, `SoluteHP98`,
  `AndersonEtAl91`, `AkinfievDiamond03`. A method alone takes the state of the solvent, and gives its derivatives with
  respect to each parameter.
- The hydrogen ion is found by its class, formula and charge, not by the symbol.
- The mineral and table methods are method objects: `LandauHP98`, `MurnaghanHP98`, `TaitHP11`, `VolumeBerman88`,
  `VolumeDorogokupets88`, `BirchMurnaghan47`, `CpExtendedUNIQUAC`, `NASA9`, `TPTable`.
- The fluid methods are method objects: `PRSV86`, `ChurakovGottschalk03`, `SoaveRedlichKwong72`, `SternerPitzer94`,
  `PengRobinson78`, `CorkHP91`. They give their derivatives with respect to each parameter.
- The reaction methods are method objects (`thermofun.methods.reactionMethod`). New: `LogKTable`, log K from a table in
  T, with a natural cubic spline or the Lagrange polynomial of GEMS.
- A cycle of defining reactions raises an error that names the cycle. Before, the stack overflowed.
- The getters of `Substance`, `Reaction` and `Element` answer from the record (`Compat/`). Arrays hold only the values
  that a method reads, and an error is positive. A value that passes through SI can differ from the file by rounding
  (for example -10465 cal gives -10464.999999999998).
- The old setters now edit the record. `setMethodGenEoS`, `setMethod_T` and `setMethod_P` raise an error for a code that has no
  method for that kind of record, for example `CTM_LGX` on a substance (`ValueError` in Python); before, the code was stored and
  did nothing. `setThermoParameters` raises an error that names the array and the position if an array with values misses a
  required parameter.
- The engine runs every record through method objects: the old `switch` evaluation is gone. New:
  `thermoPropertiesSubstanceDetailed` gives the result, its source, the methods in order and the conversions applied.
  `explain` is built on it, so it now needs the defining reaction of a substance in the database.
- `Substance`, `Reaction` and `Element` hold only their record. The old parser is removed, with the C++ functions
  `parseSubstance`, `parseReaction`, `parseElement` and the header `Common/ParseJsonToData.h`: use the constructors. The Python
  functions stay. `readConventions` is in `Database.h`.
- `Element::heatCapacity()` and `volume()` give 0 if the record has no such value; before, a missing value took the entropy or
  the atomic mass. `lowerP()` and `upperP()` give the limits of the file; before, they were not read.
- `ThermoFun/OptimizationUtils.h` is removed. The engine keeps its results in `Engine/ResultCache.h`.
- The settings of the engine are typed: `EnginePreferences::property_source`, `method_implementation`, and
  `EngineConventions::energy_reference`, `standard_pressure`, `concentration_scale`, `water_reference`. In Python they take the
  enum or a word. `apparent_properties` and `water_properties` still work. A wrong word now raises an error with a hint; before, it
  was ignored. A convention that is not calculated yet (for example `1atm`) raises an error.
- New: `setPropertySource` and `propertySource`: a substance from its methods or its defining reaction, a reaction from its methods
  or its reactants, for one record or for all.
- ThermoFun reads the v2 format in JSON: `Database`, `ThermoEngine`, and `Substance`, `Reaction`, `Element` for one record. Units of a
  scope, uncertainties, method implementations, interpolation, covariance blocks and diagnostics with the path of the value.
  `Database` gives `thermoDataSet`, `methods`, `parameterSets`, `dataSources` and `loadDiagnostics`; the objects give `record()` in
  Python. The v2 writer is next.
- The v2 format can be YAML: a file or a text, read in C++ with the vendored rapidyaml, in C++ and Python alike. Diagnostics give the
  line and the column, for JSON too. PyYAML is not needed.
- New method `LandauHP11`: the Landau term of Holland and Powell (2011). It is for v2 data: it has no legacy name.

### Added (tests)
- C++ unit test target `tfun-unit` (`tests/model/`): the parser is compared with a frozen copy for every record of
  the test databases.
- Layer check of the includes (`tests/layers/check_includes.py`), run in CI.

- Model databases with published parameters (`pytests/data/published/`) and tests against the published values
  (`pytests/test_published_models.py`): Dolejš and Manning (2010), 7 minerals; Marshall and Franck (1981).
- The synthetic fixtures r14 and s37 use published values instead of invented ones.
