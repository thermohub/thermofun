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
