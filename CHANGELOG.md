# Changelog

All notable changes to ThermoFun are in this file.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

"Review Xn" names a finding of the external review of the v2 branch (November 2026); the entry says what it was.

### Changed, read before you update

- **V is in m3/mol** in all results (`volume`, `reaction_volume`, and `substanceProperties` of the water methods), SI as P
  in Pa, so dG/dP = V (review Q4, decision 10). Before: J/bar (1 J/bar = 1e-5 m3/mol). A consumer that converts V must
  change its factor: J/bar to cm3/mol was 10, m3/mol to cm3/mol is 1e6. C++: `THERMOFUN_VOLUME_SI` is defined in
  `ThermoEngine.h`; Python: `thermofun.volume_unit == "m3/mol"`. `ThermoBatch` and the tables give m3/mol unless another
  unit is asked for (J/bar and cm3/mol still work).
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
- **The namespace of the model is `ThermoFun::model`** (was `ThermoFun::v2`, a name tied to a version of the file
  format). `ThermoFun.h` keeps `ThermoFun::v2` as an alias until 2.0 (review Q5).
- **Only the public headers are installed** (45 of 144): ThermoEngine, Database, Substance, Reaction, Element,
  ThermoBatch, Tables, the records (`Model/`), Diagnostics, the method catalog and Evaluate, and what they include
  (`ThermoFun/PublicHeaders.cmake`, checked in CI by `tests/layers/public_headers.py`). Include `ThermoFun/ThermoFun.h`
  (review Q6).
- **`python -m thermofun.convert` writes exact numbers by default**, so a conversion does not change a result (review Q10).
  `--readable` writes short decimals, and the report says how many numbers changed and by how much. `save()` and the
  writers stay readable by default: a short decimal reads back to about 6e-8 relative.
- **Uncertainties of format 2.0** (review Q8): `{half_width: a, distribution: uniform}` gives u = a/√3, `triangular`
  a/√6 (GUM); other words are an error (V2-UNC-005). `degrees_of_freedom` and `standard_state_convention` are not part of
  the format: a warning (V2-UNC-006, V2-KEY-003), and a label with 1atm sets `standard_pressure: 1atm`. The docs say that
  every `err` is a standard uncertainty (k = 1), also for data given at 95 %.
- **Records with the same data are equal**, whatever their files wrote (review Q7): the text of the file, the unit and
  number as written and the method as written are in members that equality ignores (`file`, `written`). In C++ the
  fields have new places (`record.file.text`, `quantity.written.unit`, `application.written.method`); the Python names
  stay.
- **`max_cache_size` is for all the caches together, 100000 by default** (review Q11). It was 1e6 for each of four
  caches: up to about 2.8 GB; now about 70 MB at most.
- **log K from the reference values** (review B2): dG is derived from log K (and log K from dG), and a missing dH or dS
  is filled at Tr, not at the T of the call. Records with log K and dH gave log K -1.75 instead of 2 at 25 degC;
  `LogKConstantCp` with log K, dH and dCp stayed constant; the polynomial from dCp(T) of a legacy record lost its dCp
  terms. PSI/Nagra `PuO2(coll,hyd)` (log K -5.97, dH 0) gave log K 0. `LogKVantHoff` without dH and dS keeps log K
  constant (it gave 0); validate warns (V2-MTH-024) when a method of log K lacks its values. Tests against the closed
  forms at three T: `pytests/test_logk_forms.py`.
- **CpPolynomial with fewer phase transitions than segment boundaries** read past the end of the list (review S5); a
  boundary without a transition has none. V2-MTH-025 warns of a transition that is not at a boundary, and of more
  transitions than boundaries.
- **`Substance::setThermoCalculationType`** writes only the record (review S7): REACDC then DCOMP kept the reaction in
  the record, so the engine and a JSON round trip went on using it. `thermoCalculationType()` answers from the record:
  a legacy record with a reaction and only `mv_constant` now says REACDC (it said DCOMP, but was calculated from the
  reaction). The legacy reader marks a record "from a reaction" only if it has one (H+ was marked); results do not change.
- **Fluids and HP98 minerals are thermodynamically consistent** (review S10: `pytests/test_closed_forms.py` found the
  inconsistencies; all came from GEMS). The implementation `consistent` is the default of `PRSV86`, `CorkHP91`,
  `SoaveRedlichKwong72`, `PengRobinson78`, `SternerPitzer94`, `ChurakovGottschalk03`, `LandauHP98` and `MurnaghanHP98`;
  `gems` keeps the formulas of GEMS (the golden references are reproduced exactly with it). What changes:
  - every fluid adds its residual Cp, which it calculated but did not add: Cp of CH4, CO2 and others changes by up to
    38 J/(mol K) (CO2, 373 K, 500 bar);
  - `PRSV86` below Tc: H took the Peng-Robinson form for a constant k (H2O: 675 J/mol at 298 K, 209 J/mol at 573 K and
    500 bar), and d2a/dT2 had a sign wrong (Cp: 11 J/(mol K)); S and H follow;
  - `CorkHP91` of H2O: da/dT of the supercritical branch had the signs of a4 and a6 wrong; the virial term above p0 was
    not in H and S (also for CO2); in the liquid branch H and S were those of the last part only, without the parts at
    Psat (S off by up to 53 J/(mol K)); the MRK part of H and Cp took the volume with the virial term; the cubic used
    pi with 8 digits; between 673 and 695 K, a(T) was a polynomial in (673 - T), where Holland and Powell (1991, Eq. 6)
    and Perple_X have (T - 673): G of H2O was off there by up to about 140 J/mol and jumped at 695 K (112 J/mol at
    1 kbar). Checked against the paper and Perple_X (`crkh2o`, `crkco2`): the other constants agree; the a(T) of CO2 is
    that of Perple_X ("from Roger" Powell), not the one of Table 1 of 1991, and its virial terms are those of Holland and
    Powell (1998);
  - `ChurakovGottschalk03`: S lacked R ln Z and H had Z RT instead of (Z - 1) RT (S and H at the density, not at P). It
    still has no residual Cp: that needs second derivatives that are not made;
  - `MurnaghanHP98` and `LandauHP98`: V is integrated from the reference pressure, not from P = 0, at every point. G
    was higher by about V x 1 bar except at exactly 298.15 K and 1 bar, where it jumped (Albite 10.0 J/mol, Calcite
    3.7, Quartz 2.3) and V lost the Landau term. The Landau volume term has its Cp, and Tc0 is at Pr.
  G of a fluid does not change (the exact pi of CORK moves it by less than 1e-9), except CORK H2O between 673 and 695 K. G of the
  HP98 minerals and of their reactions changes by 2-10 J/mol. A code that must keep the numbers of GEMS sets
  `method_implementation` to `gems` for these methods.
- **The solvent is chosen once.** If the database has no `H2O@` (for example PSI-TDB, `H2O(l)`), the engine chose the
  solvent at the first aqueous species it calculated, and results before that took the log K data at 1 bar: AgCl(cr) at 573 K and 1 kbar
  was -116386 J/mol first and -117860 J/mol after Cl-. The engine now chooses it when it is made or a setting changes, and
  a calculation no longer changes `solvent_symbol` or `solvent_state` of the preferences.

### Added

- **A calculation under each method example** on the method pages: the code that calculates the record at two points
  and the values it prints (G, H, S, Cp, V, or log K and the reaction values). The generator runs it, so a page cannot
  show a value that the code does not give. Twelve methods got an example for this (log K and volume forms, solutes, a table);
  `CpFeiSaxena86` and `LambdaBerman88` have none, because they have no code yet.
- **The conversion and the record builders in C++** (`Convert.h`, `RecordBuilder.h`), so a C++ program has what Python
  has. `convertFile` (legacy or v2 to v2 JSON, YAML, a dataset folder, or legacy JSON), `conversionReport` (the
  migration report), `shortenedNumbers`, `Database::saveDataset` (a dataset folder), and `RecordBuilder` with
  `RecordValue`. `python -m thermofun.convert` and `thermofun.records` now call them; their Python API is the same.
- **A site of the documentation** built with Sphinx from `docs/` (`docs/conf.py`, `docs/requirements.txt`, workflow
  `docs.yml`, published on GitHub Pages). The tutorials are notebooks (MyST Markdown) that run at each build, and in
  `pytests/test_docs_pages.py`. They are eight now: the five before and the seven notebooks `thermofun-basics` of
  thermofun-jupyter, merged; thermofun-jupyter takes its copies from here. `pandas` and `matplotlib-base` are in
  `environment.devenv.yml` for them.
- `EngineConventions::apparentPropertiesWord()` and `waterPropertiesWord()` in C++: the old words of the conventions. The
  fields `apparent_properties` and `water_properties` are empty after a call of the engine (review Q9).
- `pytests/test_closed_forms.py`: each family of methods against its closed formula in a sweep of T, and every record
  of the bundled databases against dG/dT = -S, dH/dT = Cp, dG/dP = V, log K and dG = dH - T dS (review S10). The pytests
  run on macOS (arm64) too.

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
- The JSON Schema of format 2.0, `ThermoFun/IO/V2/thermohub.schema.json` (installed), made by `python -m thermofun.schema`
  with the type of each key; `docs/reference/format-v2-schema.md` is generated from it. A test checks that its keys are
  those of the reader, and that every v2 file of the repository and the v2 form of every bundled database are valid.
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

- **`BirchMurnaghan47` gave V about V0 too large** away from the reference state: the code of GEMS added `V0 + V(T, P)` to the
  volume before it. The default implementation `consistent` sets V to the volume of the equation (G, H and S were right and
  do not change). The implementation `gems` keeps the old result. Tested against an independent solution of the equation.

- A dataset folder written by `python -m thermofun.convert --split` had a `thermodataset` block in each record file,
  with only the default water convention. Read back, these blocks replaced the dataset of `thermodataset.json`: its
  symbol and name were lost, and a convention other than `HKF_HGK_1974` (for example `NEA_IAPWS`) was replaced by
  `HKF_HGK_1974`. A record file now has no `thermodataset` block (`V2WriterOptions::dataset_block`).
- `ThermoBatch` wrote every volume as 0.000 by default: its 3 decimals were those of J/bar. The default is 8 decimals
  of m3/mol, the same resolution.
- On macOS, errors of a table setup reached Python as RuntimeError, not `TableSetupError`: its type information is now
  in the library only. And without `from_chars` for double (libc++ before 16), a number in a text was read to its last
  point, so "1.5.2 J/mol" was 1.5 J/mol and not an error.
- The library links spdlog itself when it uses the compiled spdlog (and so fmt), not only through ChemicalFun (review N9).
- Derivatives and properties fixed with autodiff (before in `docs/explanation/autodiff.md`):
  - `AkinfievDiamond03`: the reference-state increments carried temperature derivatives.
  - `ln K = -dGr/(R T)` and `dS = (dH - dG)/T` lost the derivative of `1/T`.
  - `LandauHP98`: V is dG/dP of its own G at fixed Q (Holland and Powell 1992, Am. Min. 77:53):
    `V = v_bis (1 + 4P/(1000 kT))^(-1/4) + Vmax (Q^6/3 - Q^2)`. V, U and A change by up to 7e-4 relative (Quartz).
  - `CpPolynomial` with several intervals, `ConstantCp`, `BirchMurnaghan47`: the interval bounds and reference
    temperatures were treated as variables in the derivatives.
  - The pressure of the reaction volume correction was a plain number: dG/dP of a substance from such a reaction was 0.
  - `HKF88` (gems): the exponent of the g function lost its temperature derivative.
  - `WaterZhangDuan05`: the pressure derivatives were scaled twice, and `densityPP` was 1e5 times too large. Its
    derivatives, and those of `SverjenskyEtAl14` and `FernandezEtAl97`, are analytical (they were finite differences).
  - `WaterHGK84` (gems) did not set `densityPP` and `densityTP`.
  - `ChurakovGottschalk03`: Z, U and the residual S are analytical (they were finite differences with a step of 1e-5).
    Values change by about 1e-5: V 1e-5, H 4e-5.
  - LVS critical region of GEMS water: `dalLVS` had misplaced parentheses. At 647.5 K and 22 MPa, dalpha/dT was
    +0.086 instead of -0.172 1/K^2.
  - `DolejsManning10` called the Frantz-Marshall function and read past its 5 coefficients. The units of its first
    coefficient were not checked against the paper.
  - `SoluteHP98`: `T'` was a constant below 500 K, so dG/dT was wrong there. S, Cp and H are the exact derivatives of G
    (implementation `consistent`, the default); the published formulas are the implementation `published`.
  - `ReactionVolumePolynomial` (`Reaction_Vol_fT`) returned empty properties, and mixed Pa and bar. It is implemented:
    `V = Vst (1 + a0 dT + a1 dT^2 + a2 dT^3 + a3 dP + a4 dP^2)`, dG is the integral of V dP.
  - The compressibility and expansivity of a record without them were not initialised; they are 0.
  - U and A of `HKF88` solutes multiplied P V by 41.84 (a conversion from cal of a value in J).
  - U, A and Cv of every substance come from the other properties, with their derivatives and errors: `U = H - P V`,
    `A = U - T S`, `Cv = Cp - T V alpha^2/beta` (not for water). Before, many methods gave Cv = 0 or Cp.
  - A reaction from its reactants had Cv, U and A of its last reactant only; they are the sums over all reactants.
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
