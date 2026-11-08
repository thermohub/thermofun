# Changelog

All notable changes to ThermoFun are in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Changed, read before you update

- **Gases:** G of a gas with a fluid method or `IdealGas` has `RT ln(P/Pr)` and `RT ln(phi)` by default, on every path (defaults `apply_pressure_correction_to_gas_props = true`, `cancel_fugacity_coefficient_in_gas_props = false`; before: `false` and `true`). A consumer that adds these terms itself counts them twice with the defaults: for example +26 to +30 kJ/mol for CH4, CO, CO2 and H2 at 473.15 K and 1 kbar. Call `EnginePreferences::useStandardStateOfGases()` to keep the old values. GEMS3K releases before this change must not be used with ThermoFun 1.0.
- **A substance defined by a reaction that does not change with P, with its own pressure method** (for example
  `ConstantVolume`): the other species of the reaction are taken at max(Pr, Psat, P at which water has 420 kg/m3).
  Above the critical temperature of water they were at P, and the pressure came in twice (dG/dP was V of the record plus
  that of the other species) with a jump of G at Tcr. Results change near and above 374 °C.
- **`AndersonEtAl91`: H** is now G + T S plus the constant of the reference values. It lacked -T K (alpha(Tr) - alpha(T)),
  so dH/dT was not Cp (checked against Dolejš 2013, Eq. 106). G, S, Cp do not change; H, U, A do.
- **Legacy files: the value 7777777 is read as no value** (LEG-QRK-013), the mark of GEMS exports. In slop98, G and H of
  40 minerals (for example almandine, pyrope, chloritoid) had it, and the engine used it as G = 7.78 MJ/mol. G and H of
  these records are now not defined; the migration report lists them.
- **P = 0 is the saturation pressure for every record.** A record that does not use water (a mineral, a gas, a reaction
  with a volume term) was calculated at 0 Pa, and in a reaction from its reactants the order of the reactants decided
  which species got Psat. For example NpO2SiO3@ of PSI/Nagra moves by 293 J/mol at 573 K.
  The saturation pressure comes from the water record of the engine, else from the Wagner-Pruss curve.
- **A state point without a meaning is refused** (`ValueError` in Python, `std::invalid_argument` in C++), at every
  calculation: T or P not finite, T <= 0, P < 0, and P = 0 (the saturation pressure of water) at or above the critical
  temperature of water, 647.096 K. Before, P = 0 above Tcr gave a value at 0 Pa for minerals and gases and an unclear
  error for water and solutes; a NaN temperature could be served from the cache.
- **Copy assignment of `ThermoEngine` copies.** `b = a` made two names for one engine: settings and results were
  shared. It now makes a copy, as the copy constructor does. **Threads:** one engine for each thread; copy it (cheap,
  the records are shared). `docs/explanation/data-model.md` gives the rule.
- **autodiff is always used.** The CMake option `TFUN_USE_AUTODIFF` is gone (review Q2): a build with plain numbers had
  other public types and derivatives silently 0. `-DTFUN_USE_AUTODIFF=OFF` stops the configuration. `thermofun.with_autodiff`
  stays, always True.
- **The installed CMake package finds its targets from its own folder**, also when spdlog is in another prefix (review B3).
- **The solvent is chosen once.** If the database has no `H2O@` (for example PSI-TDB, `H2O(l)`), the engine chose the
  solvent at the first aqueous species it calculated, and results before that took the log K data at 1 bar: AgCl(cr) at 573 K and 1 kbar
  was -116386 J/mol first and -117860 J/mol after Cl-. The engine now chooses it when it is made or a setting changes, and
  a calculation no longer changes `solvent_symbol` or `solvent_state` of the preferences.

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
- `EnginePreferences::useStandardStateOfGases()` (C++ and Python): G(T, Pr) for a gas, the defaults before 1.0, in one call.
- `Methods/ParameterSets/DataPressure.yaml` (`rho_min`) and `SolventMethod::pressureAtDensity`.
- `Status.outofbounds` in Python.
- CNV-008: a warning when data use the energy reference `dft_formation_0K`, which is an estimate (the energy at 0 K taken
  as the enthalpy of formation at 298.15 K). The conventions page states it, and the approximation of
  `gibbs_helmholtz_formation`.
- `SoluteHP98` has two implementations: `consistent` (the default; S, H and Cp are the exact derivatives of G) and
  `published` (the formulas of Holland and Powell 1998, as ThermoFun had them before 1.0). G and V are the same in both.
  Choose with `implementation` on the method of a record, or `method_implementation` of the engine.
- `ThermoEngine::saturationPressure(T)` (C++ and Python): the pressure that P = 0 means, so Python can read it.

- Built-in constants of the methods in `ThermoFun/Methods/ParameterSets/`, each with a description, its units and its source. A record can replace a value for itself, and a model can have variants (`default: false`, chosen with `parameter_set`).
- `THERMOFUN_PARAMETER_PATH`: a folder with more variants, read with no rebuild.
- The CMake option `THERMOFUN_PHYSICAL_CONSTANTS`: `codata1986` (the default) or `codata2018`.
- The dielectric models `HelgesonKirkham74` and `BradleyPitzer79` (issue 82). The HKF coefficients of Tanger and Helgeson (1988) now give their published volume and heat capacity.
- Symbolic methods of substances and reactions, with the CMake option `TFUN_USE_SYMENGINE`, and `python -m thermofun.method_tool` with its source forms.
- `ThermoEngine::calculationPath`: how a record is calculated. It calculates nothing.
- Derivatives by a built-in constant, also through the water into a solute.
- `TableSetup` (C++ and Python): tables from a YAML setup or a template of the library, in the layouts long, transposed and grid, as CSV, Markdown or LaTeX. `python -m thermofun.tables`, and `tf.table` in Python.
- Tutorials, how-to pages, a glossary, and reference and explanation pages in `docs/`.

### Changed

- A gas with a fluid method or `IdealGas` has `RT ln(P/Pr)` in G on every path. A consumer that adds this term sets the two options of the engine.
- `WaterHGK84` with `reaktoro` has its triple point at 273.16 K, as the other water models. G of water changes by about 0.63 J/mol.
- R is 8.31451 J/(mol K) in Churakov-Gottschalk too, and ln(10) has all its digits.
- `JohnsonNorton91` with `gems` reads the water of the record. Water in the gas state now has a dielectric constant that agrees with its density.
- `docs/GUIDE.md` and `docs/python.md` are now separate pages: see `docs/index.md`.

### Fixed

- On the steam tables, G and A of water now have the term `-Str (T - Ttr)`. At 283 K and 100 bar, G was -466 J/mol and is now 166 J/mol.
- U and A of the triple point data `NEA_HGK` and `NEA_IAPWS`: the volume of water was in J/bar and the volume of the gases in L.
- A misspelled `parameter_set` is an error. Before, the record used the default values with no message.
- A reaction from its reactants that needs a substance from this reaction gives an error that names the cycle.
- The method tool keeps logarithms apart: `log(T**a/Tr**a)` overflowed for a large `a`.
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
  Python.
- The v2 format can be YAML: a file or a text, read in C++ with the vendored rapidyaml, in C++ and Python alike. Diagnostics give the
  line and the column, for JSON too. PyYAML is not needed.
- `Database::toJson`, `toYaml` and `save` write all records in the v2 format. A legacy value is written as its short decimal (`-56.484`); use `readable=False` for
  the exact value. See `docs/how-to/add-data.md`.
- Numbers in data files and units are read the same in all locales. In a program with a decimal comma, as Qt can set, `-1234.5` in a v2 YAML file gave -1234 with no error, and an
  equation with `0.5` was read wrongly.
- `Database::validate` reports the problems between records. A v2 file can use `include`, and `Database` reads a dataset folder. `toLegacyJson` and
  `save(path, DataFormat::Legacy)` write the legacy format. `jsonString()` of an edited record gives the record as it is now; before, it gave the old text. A v2 file without the key
  `thermohub` is refused, and a v2 file with methods that conflict is refused at the load.
- The hydrogen ion is 0 only in the geochemical convention, and only if its record gives no other value. Before, a record with the formula `H+` always gave 0 and its data was ignored.
- New guide: `docs/how-to/add-data.md`. A test runs its examples and checks its method tables against the code. New in Python:
  `thermofun.methods.catalog()` lists all methods and their parameters. A molar volume in a v2 file can be in J/bar.
- New method `LandauHP11`: the Landau term of Holland and Powell (2011). It is for v2 data: it has no legacy name.

### Added (tests)
- C++ unit test target `tfun-unit` (`tests/model/`): the parser is compared with a frozen copy for every record of
  the test databases.
- Layer check of the includes (`tests/layers/check_includes.py`), run in CI.

- Model databases with published parameters (`pytests/data/published/`) and tests against the published values
  (`pytests/test_published_models.py`): Dolejš and Manning (2010), 7 minerals; Marshall and Franck (1981).
- The synthetic fixtures r14 and s37 use published values instead of invented ones.
