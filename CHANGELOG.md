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

### Added (tests)
- C++ unit test target `tfun-unit` (`tests/model/`): the parser is compared with a frozen copy for every record of
  the test databases.
- Layer check of the includes (`tests/layers/check_includes.py`), run in CI.

- Model databases with published parameters (`pytests/data/published/`) and tests against the published values
  (`pytests/test_published_models.py`): Dolejš and Manning (2010), 7 minerals; Marshall and Franck (1981).
- The synthetic fixtures r14 and s37 use published values instead of invented ones.
