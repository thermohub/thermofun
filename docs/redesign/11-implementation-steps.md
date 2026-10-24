# 11 — Implementation steps

Do the steps in order. Each step is one commit (or a few) and leaves the tree green (§00 rules). Every step has:
**Needs** (earlier steps), **Files**, **Do**, **Test**, **Done when**, **Commit**. Read the referenced sections
before starting a step. When a step says "copy logic", move code without changing what it computes.

Notation: `TF/` = `ThermoFun/`, `PY/` = `python/src/thermofun/`.

---

## Phase 0 — Safety net

### S0.1 Golden reference suite — committed subset

- **Needs**: —
- **Files**: create `pytests/golden/golden_common.py`, `pytests/golden/make_references.py`,
  `pytests/golden/test_golden.py`, `pytests/golden/data/` (generated, incl. `subset.json`),
  `pytests/golden/README.md`.
- **Do** (design in 09 §1):
  1. `golden_common.py` — shared by the committed subset and the two-build comparison; uses only API that exists
     in v0.7.3:
     ```python
     SUBSET_T = [273.15, 298.15, 373.15, 623.15, 1273.15]                      # K
     SUBSET_P = [0.0, 1e5, 1e8, 5e8]                                           # Pa, 0 = saturation
     FULL_T = [273.15, 283.15, 298.15, 323.15, 348.15, 373.15, 423.15, 473.15, 523.15,
               573.15, 623.15, 673.15, 873.15, 1073.15, 1273.15]
     FULL_P = [0.0, 1e5, 1e7, 1e8, 5e8, 1e9]
     SUBST_PROPS = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume",
                    "heat_capacity_cv", "internal_energy", "helmholtz_energy"]
     REAC_PROPS = ["log_equilibrium_constant", "reaction_gibbs_energy", "reaction_enthalpy",
                   "reaction_entropy", "reaction_heat_capacity_cp", "reaction_volume"]
     VARIANTS = ["default", "gas", "errors", "steam", "berman"]
     def databases() -> list[pathlib.Path]: ...      # Resources/databases/*.json + pytests/**/*.json that load + synthetic
     def make_engine(db_path, variant) -> "thermofun.ThermoEngine": ...
     def scalar(x) -> list: ...                      # [val, ddt, ddp, err, status code as int or name]
     def compute(db_path, variant, T, P, symbols=None) -> dict: ...   # symbols=None → all records
     def select_subset(db_path) -> dict: ...         # {"substances": [...], "reactions": [...]} per 09 §1.3
     def compare(expected, got, rel_tol) -> list: ...  # list of differences (09 §1.2)
     ```
     Get record lists with `engine.database().getSubstancesList()` / `getReactionsList()`. Catch exceptions per
     point → `{"error": first line}`. Check how `sta` is exposed in Python and store the status code, not the text.
  2. `make_references.py`: on first run computes `select_subset()` for every database and writes
     `data/subset.json`; afterwards reads it (never re-selects unless `--reselect`). Writes
     `data/<stem>__<variant>.json.gz` (`gzip`, `json.dumps(..., separators=(",", ":"))`) and the commit sha.
     Prints the total size; if above 3 MB apply the reductions of 09 §1.3 rule 5.
  3. `test_golden.py`: parametrised over the data files; recompute the subset; `compare(..., rel_tol=1e-10)`;
     on failure print the report.
  4. Run `python pytests/golden/make_references.py`.
- **Test**: `pytest pytests/golden -q` passes; then change one coefficient in a copy of a database by hand and check
  the test fails (do not commit that).
- **Done when**: suite green; total data ≤ 3 MB; README explains when to regenerate ("only in steps that say so")
  and lists the subset rules.
- **Commit**: `Add golden reference results for a representative subset of the bundled databases`

### S0.2 Legacy key inventory test

- **Needs**: S0.1
- **Files**: `pytests/golden/test_key_inventory.py`
- **Do**: encode the table of 05 §2 as `KNOWN = {"substance": {...}, "tpmethod": {...}, "reaction": {...},
  "rtpmethod": {...}, "element": {...}, "file": {...}}`; walk all databases (same list as golden); fail listing any
  key not in `KNOWN`. Also fail if a `TPMethods.method` id is not in `enum_method_substance` ids
  (0–43 with gaps) or a reaction id not in 0–14.
- **Done when**: green on the current data. **Commit**: `Add inventory test of legacy keys`

### S0.3 Synthetic fixtures for methods not in the bundled data

- **Needs**: S0.1
- **Files**: `pytests/data/legacy-synthetic/*.json` (one per method of 09 §3), `pytests/golden/synthetic_api.py`.
- **Do**: write each fixture in the legacy format (copy the shape of `pytests/Substances/Solute/test-hkf-thermofun.json`;
  include `H2O@` with `water_eos_hgk84_reaktoro` + `water_diel_jnort91_reaktoro` when the method needs a solvent).
  Parameter values from the sources of 09 §3. For HP98Sol/Anderson91 build substances through the API in
  `synthetic_api.py` (as `examples/solute-hp98/main.cpp`), and let `golden_common.databases()` include them.
  Methods that are no-ops today (`impl = no` in 06 §5) still get a fixture: the reference records today's result.
- **Do**: regenerate references **only** for the new fixtures.
- **Done when**: every v2 id in 06 §5 is covered by at least one golden record (add a check in `test_key_inventory.py`
  that lists, from the legacy ids seen, which ids are covered; expected: all).
- **Commit**: `Add synthetic fixtures covering every method`

### S0.4 Two-build comparison and CI job

- **Needs**: S0.1, S0.3; the maintainer has tagged `golden-baseline` (the commit with S0.1–S0.3 merged).
- **Files**: `pytests/golden/build_baseline.sh`, `pytests/golden/compare_builds.py`, `.gitignore`
  (`.golden-baseline/`), `.github/workflows/ci-build.yml` (new job `golden-diff`).
- **Do**: 09 §1.4. The job runs on `pull_request` and on `push` to `master`, Linux only, uses the same conda
  environment as the build job, caches the baseline install with `actions/cache` (key of 09 §1.4).
- **Test**: run locally: build the baseline, compare the working tree with it → no differences; change a coefficient
  in a database copy → reported. Open a draft pull request to check the CI job and the cache.
- **Done when**: `golden-diff` green on a pull request; second run restores the cache.
- **Commit**: `Compare every pull request with the golden baseline build`

### S0.5 Changelog

- **Files**: `CHANGELOG.md` (Keep a Changelog; "Unreleased" section). **Commit**: `Add changelog`

---

## Phase 1 — Internal model and legacy reader (release 0.8.0)

### S1.1 C++ unit-test target and the frozen parser

- **Needs**: S0.1
- **Files**: `tests/model/CMakeLists.txt`, `tests/model/check.h`, `tests/model/main.cpp`,
  `tests/model/dump.h`, `tests/model/frozen/ParseJsonToData_v073.cpp`, `tests/model/frozen/ParseJsonToData_v073.h`,
  `tests/model/test_legacy_equivalence.cpp`; modify root `CMakeLists.txt` (`add_subdirectory(tests/model)` inside
  `if(TFUN_BUILD_TESTS)`).
- **Do**:
  1. `check.h`: the `check()` macro and failure counter of `tests/memoization/src/main.cpp`, as a header.
  2. `main.cpp`: calls `runAll()` functions of each `test_*.cpp`, returns failures count. One executable
     `tfun-unit` linked to `ThermoFun::ThermoFun`, `add_test(NAME tfun-unit COMMAND tfun-unit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})`.
     It needs nlohmann_json (already found by `ThermoFunFindDeps`).
  3. `dump.h`: `dumpSubstance(const Substance&) -> std::string` printing **every** legacy field via public getters,
     including all fields of `ThermoParametersSubstance` (iterate each vector, `coefficient_errors` map),
     reference properties (`val, ddt, ddp, err, sta`), enums, limits, reference T/P, class, state, charge,
     molar mass, reaction symbol, thermo calculation type. Same for Reaction and Element. Use `%.17g`.
  4. Frozen parser: copy `TF/Common/ParseJsonToData.cpp` as it is now; put its content in
     `namespace frozen_v073 { ... }`; keep includes; rename nothing else. Add the matching header.
  5. `test_legacy_equivalence.cpp`: for each JSON file in the list of S0.1 (hard-code the paths relative to the
     source dir), for each record text (dump each element of `j["substances"]` etc.), compare
     `dumpSubstance(frozen_v073::parseSubstance(text))` with `dumpSubstance(ThermoFun::parseSubstance(text))`.
- **Done when**: `ctest -R tfun-unit` passes (trivially, both parsers are the same code now).
- **Commit**: `Add C++ unit test target and frozen copy of the legacy parser`

### S1.2 Diagnostics

- **Files**: `TF/Common/Diagnostics.h/.cpp`, `tests/model/test_diagnostics.cpp`.
- **Do**: as 02 §4.1. `toString()` format: `"<severity> <code> <file>:<record>:<path>: <message>"`, empty parts
  omitted. Add `diagnosticCatalog()` returning `std::vector<{code, severity, meaning}>` with all codes of 04 §7
  (meaning = the table text).
- **Commit**: `Add Diagnostics for collected reading and validation problems`

### S1.3 Quantity and vocabularies

- **Files**: `TF/Model/Quantity.h/.cpp`, `TF/Model/Vocabulary.h/.cpp`, `tests/model/test_vocabulary.cpp`.
- **Do**: types of 03 §1–§2. `Vocabulary.cpp` holds static tables: v2 string ↔ enum ↔ legacy code for
  SubstanceClass, AggregateState, ElementClass (verify element codes 1–3 against ChemicalFun's `ElementKey`/`Element`
  sources before writing them; if unsure, map only 0 and 4 and treat others as `other` keeping the code in
  `extra_json`). Functions: `substanceClassFromLegacy(int) -> std::string`, `legacySubstanceClass(const std::string&)
  -> SubstanceClass::type`, the same for aggregate state and element class; `propertyNameFromString`, `toString`.
  Each vocabulary table has an alias column (start with `crystalline` → `crystal`); readers map aliases to the
  canonical value, writers emit canonical values. Keep all tables in this one file (Q12: provisional values).
- **Test**: every value round-trips; unknown string → `other`.
- **Commit**: `Add Quantity, Uncertainty and vocabularies of the record model`

### S1.4 UCUM front end

- **Files**: `TF/Common/Ucum.h/.cpp`, `tests/model/test_ucum.cpp`; modify `TF/Common/Units.cpp` to add `cal_IT`
  (4.1868 J) and `cal_th` (= cal) atoms.
- **Do**: 07 §3. Implement `fromUcum` as a tokenizer: split into atoms, operators `.` `/` `(` `)`, and exponents;
  output converter syntax. `dimension()`: convert 1 unit to a set of base units — implement by asking the converter
  for the factor to each base combination is hard; instead implement a small table: dimension of each atom known to
  the converter (copy the atom list from `Units.cpp`: mass, length, time, temperature, amount, energy = kg m² s⁻²,
  pressure = kg m⁻¹ s⁻², …) and combine exponents while parsing.
- **Test**: 07 §3 table; all unit strings of the bundled data (collect with a one-off script, paste the list);
  `convertDifference(1, "degC", "K") == 1`.
- **Commit**: `Add UCUM unit-string front end and dimension check`

### S1.5 Record types

- **Files**: `TF/Model/Applicability.h`, `MethodApplication.h/.cpp`, `MethodDefinition.h`, `MethodParameterSet.h`,
  `DataSource.h`, `ThermoDataSetInfo.h`, `ElementRecord.h`, `SubstanceRecord.h`, `ReactionRecord.h`, `Records.h`,
  `TF/IO/Format.h`; modify `TF/ThermoFun.h` to include `Model/Records.h` and `IO/Format.h`.
- **Do**: declarations of 03 exactly (field names matter: they are the v2 keys). `Quantity::isBare()`.
  Equality operators (`= default` with C++20) for all records — used by tests.
- **Test**: `tests/installed_headers` passes (the new headers must not include nlohmann).
- **Commit**: `Add format-neutral record types`

### S1.6 Method catalog (specs only)

- **Files**: `TF/Methods/MethodSpec.h`, `TF/Methods/MethodCatalog.h/.cpp`, `tests/model/test_catalog.cpp`.
- **Do**: 06 §1–§2 (catalog part only, no registry yet). Fill the table from 06 §5: id, aliases (the legacy
  `tp_method_name` and the Thrift enum name, e.g. `"solute_hkf88_gems"`, `"CTPM_HKF"`), slot, role, needs_solvent,
  piecewise, implemented, `effects` (table 06 §8.3), parameters with `si_unit`, `model_unit` (copy the `units_to` lists from
  `ParseJsonToData.cpp` exactly), `legacy_key`, `legacy_index`, and `legacy` aliases (id, name, thrift code from
  `new_old_s_methodtype`/`new_old_r_methodtype`, or 122/123 for HP98/AN91, 99 for CON).
  For parameter names marked "to be determined", use `c0…cN` and leave a `// TODO(S2.3)` comment.
- **Test**: 09 §5 `test_catalog.cpp` list.
- **Commit**: `Add catalog of built-in method specifications`

### S1.7 Format detection

- **Files**: `TF/IO/FormatDetect.h/.cpp`, `tests/model/test_format_detect.cpp`.
- **Do**: 05 §1. The header declares only string-based functions (`detectDocumentFormat(const std::string& text)`)
  so nlohmann stays in the `.cpp`; an internal header `TF/IO/detail/JsonDetect.h` (not installed: add
  `PATTERN "detail" EXCLUDE` to the header `install(DIRECTORY ...)` in `TF/CMakeLists.txt`) offers the `json`
  overloads for other `.cpp` files.
- **Commit**: `Add detection of legacy and v2 data`

### S1.8 Legacy reader → records

- **Files**: `TF/IO/Legacy/LegacyKeys.h`, `LegacyQuirks.h/.cpp`, `LegacyReader.h/.cpp`,
  `tests/model/test_legacy_reader.cpp`.
- **Do**:
  1. `LegacyKeys.h`: `inline constexpr const char* SmGibbsEnergy = "sm_gibbs_energy";` etc. for every key of 05 §2.
  2. `LegacyReader.cpp`: functions of 05 §3. Port the value-reading helpers from `ParseJsonToData.cpp`
     (`readValueErrorUnit`, `read_values_units`, `read_value_unit`, `convert_values_units`) **unchanged** into
     `namespace ThermoFun::detail` in this file (they become the single implementation; `ParseJsonToData.cpp` will
     call them in S1.9). Then fill records: values in SI (`units::convertAny(value, unit, si)`), quirks through
     `LegacyQuirks` functions that also add `LEG-QRK-*` info diagnostics.
     Coefficient arrays → named parameters with the catalog's `ParameterSpec`s: for method spec M and its
     `legacy_key`, `parameters[spec.name] = Quantity{convert(values[idx], unit_i, spec.si_unit)}` where `unit_i`
     follows the same rule as `convert_value_units` (missing/empty → the model unit). Extra values → `extra_json`
     `_legacy_extra_values` (LEG-QRK-010). Coefficient keys that belong to another method → 05 §3.3.
  3. `original_text` = the input text; `source_format = Legacy`.
- **Test**: small hand-written records: HKF species, a Cp mineral with 2 intervals and a transition, H2O@, a gas
  with PRSV, a reaction with logk_ft_coeffs; check record fields and SI values (e.g. a1 in J/(mol.Pa) =
  0.1839 cal/(mol*bar) × 4.184 / 1e5).
- **Commit**: `Read legacy records into the format-neutral model`

### S1.9 Legacy view: records → legacy fields; switch the parser

- **Needs**: S1.8
- **Files**: `TF/IO/Legacy/LegacyView.h/.cpp`; modify `TF/Common/ParseJsonToData.cpp`, `TF/Substance.h/.cpp`,
  `TF/Reaction.h/.cpp`, `TF/Element.h/.cpp`.
- **Do**:
  1. `applyLegacyView(const SubstanceRecord&, Substance&)`: set every legacy field from the record so that the
     result equals the old parser's. Write it by following the old `parseSubstance` line by line and replacing
     each JSON read by the record field, converting SI back to the units the old code produced (`units_to`), and
     rebuilding `ThermoParametersSubstance` with the catalog's `legacy_index` order, `coefficient_errors` with the
     parameter uncertainties, `temperature_intervals` from segments (only for `CpIntegration`, as the old
     `prop_name == "cp_ft_equation"` rule), `phase_transition_prop` from transitions, method enums through
     `setTPMethods_old` with the spec's legacy id, and `REACDC` rule.
  2. `Substance` (pimpl): add `SubstanceRecord record; bool record_stale = false;`, `record()`, `setRecord()`
     (08 §2). In every existing setter add `pimpl->record_stale = true;`.
  3. `ParseJsonToData.cpp`: `parseSubstance(text)` becomes `Substance s; s.setRecord(readLegacySubstance(text));
     return s;` (with `setRecord` calling `applyLegacyView`). Same for reactions and elements. Keep the old
     helper functions only if still used; otherwise delete them (the frozen copy keeps the old behaviour for the
     test).
  4. JSON exceptions: keep the old `try/catch` with `thfun_logger->error` in `parse*` so behaviour on bad input
     is unchanged.
- **Test**: `tfun-unit` (equivalence for all bundled records), golden suite, all pytests.
- **Done when**: all green with no tolerance change.
- **Commit**: `Load legacy records through the record model`

### S1.10 Legacy fields → record (for edits through the old API)

- **Files**: `TF/IO/Legacy/LegacyView.h/.cpp` (add `legacyViewToRecord`), `Substance.cpp` (`record()` rebuilds when
  stale), tests.
- **Do**: inverse of S1.9. Start from the previous record (keeps metadata and `extra_json`), overwrite what the
  legacy fields define. `original_text` is cleared when stale (then `jsonString()` keeps returning the old stored
  string as today — `jsonString()` is not changed in this step).
- **Test**: for every bundled record r: `legacyViewToRecord(apply(r), r) == r`; editing `setMethod_P` then reading
  `record().methods` shows the new method.
- **Commit**: `Rebuild records after edits through the legacy API`

### S1.11 Database metadata, Python bindings for records, release 0.8.0

- **Files**: `TF/Database.h/.cpp`; `PY/pyQuantity.cpp`, `PY/pyRecords.cpp`, `PY/pyDiagnostics.cpp`, register them in
  `PY/PyThermoFun.cpp`; `pytests/io/test_records.py`.
- **Do**: `Database::Impl` gains `dataset_info`, the three project maps (empty for legacy), `revision` (increment in
  every add/set/append). Read `thermodataset`, `datasources`, `date`, `conventions` into `dataset_info` in
  `fromFile`. Accessors of 08 §1 (`thermoDataSet()`, `methods()`, `parameterSets()`, `dataSources()`,
  `revision()`). Bindings of 08 §2.
- **Test**: Python can read `db.getSubstance("H2O@").record().standard_state_properties`.
- **Release**: update `CHANGELOG.md`; ask the maintainer to bump `project(ThermoFun VERSION 0.8.0)`.
- **Commit**: `Expose records and dataset metadata (C++ and Python)`

---

## Phase 2 — Registry, binding, executor (release 0.9.0)

### S2.1 Move the dispatch code out of ThermoEngine.cpp unchanged

- **Files**: `TF/Engine/WorkContext.h`, `TF/Engine/SubstanceExecutor.h/.cpp`, `ReactionExecutor.h/.cpp`,
  `SolventExecutor.h/.cpp`; modify `TF/ThermoEngine.cpp`.
- **Do**: move, **without editing logic**, these `ThermoEngine::Impl` members into free functions or classes in
  `Engine/` that take a `WorkContext` (references to the database, preferences, conventions, the memoized
  `*_fn` callables, the water triple data): `getThermoPreferencesSubstance`, `thermoPropertiesSubstance`,
  `fallbackThermoPropertiesSubstance`, `reacDCthermoProperties`, `propertiesSolvent`, `electroPropertiesSolvent`,
  `getThermoPreferencesReaction`, `thermoPropertiesReaction`, `thermoPropertiesReactionFromReactants`,
  `warnIfLogKFunctionDisagrees`, `completeThermoProperties`, `toSteamTables`, `toBermanBrown`.
  `ThermoEngine::Impl` keeps the memoization and calls them. Keep `WorkPreferences` (move it to `WorkContext.h`).
- **Test**: golden, pytests, ctest — identical.
- **Commit**: `Move substance, reaction and solvent evaluation into Engine/`

### S2.2 Registry and binding

- **Files**: `TF/Methods/MethodRegistry.h/.cpp`, `Binding.h/.cpp`, `BuiltinSubstanceMethods.cpp`,
  `BuiltinReactionMethods.cpp`, `BuiltinSolventMethods.cpp`, `tests/model/test_binding.cpp`.
- **Do**: 06 §2 and §6. In this phase factories only resolve the spec (the executor still calls the classes);
  `bindSubstance` returns the slot specs and a `Substance` whose legacy fields are packed from the record
  (`packLegacy`). For legacy-loaded records the legacy fields already exist; `packLegacy` must still be
  implemented from named parameters, because v2 records need it.
- **Test**: for every bundled record, `dumpSubstance(packLegacy(record))` equals `dumpSubstance(substance)` —
  this checks every parameter name, index and unit of the catalog **before any v2 file exists**.
- **Commit**: `Add method registry and binding of records to model inputs`

### S2.3 Complete the parameter names

- **Files**: `TF/Methods/MethodCatalog.cpp`, `docs/redesign/06-methods-and-executor.md`.
- **Do**: for each `TODO(S2.3)`: read the model code and name the parameters:
  - gas `eos_gas_crit_props` index 6: `TF/Substances/Gases/s_solmod2_.cpp`, the `*CalcFugPure` functions and
    where `Eosparm[i][6]` / `Coeff[6]` is used;
  - CGF `eos_churakov_gottschalk_coeffs`: `GasCGF.cpp`, `CGFpure.hpp`;
  - Birch–Murnaghan: `SolidBMGottschalk.cpp` `BirchMurnaghan()` (BMConst[0..7]; units from how they are used with
    `Vst*10.` and P in bar).
  Update the table in 06 §5.
- **Commit**: `Name the remaining method parameters`

### S2.4 Dispatch on bound specs

- **Files**: `TF/Engine/*Executor.cpp`, `TF/ThermoEngine.cpp`.
- **Do**: replace `pref.method_genEOS/method_T/method_P` (read from the Substance's legacy enums) by the codes of
  the bound specs (`spec->legacy[0].thrift_code`), obtained from `bindSubstance(record)`. Keep each `switch` as is
  (now on the bound code) — this step changes only where the codes come from. Then evaluate the model classes with
  `bound.legacy_substance` instead of the database's `Substance`.
- **Test**: golden identical.
- **Commit**: `Evaluate substances and reactions from bound methods`

### S2.5 Binding cache

- **Files**: `TF/ThermoEngine.cpp`, `pytests/test_binding_cache.py`.
- **Do**: 06 §6 cache keyed by symbol, cleared when `database.revision()` changes (check at each top-level
  property call; cheap integer compare). Thread safety: same mutex pattern as the memoization caches in
  `OptimizationUtils.h`.
- **Test**: compute, `db.setSubstance(modified)`, compute again → new value (engine built from the same database
  object; check how `ThermoEngine` holds the database — copy or reference — and test accordingly).
- **Commit**: `Cache bound records in the engine`

### S2.6 Executor rule tests

- **Files**: `tests/model/test_executor_rules.cpp`.
- **Do**: one test per rule of 06 §7 with expected values computed by the current build (they equal v0.7.3 by the
  golden suite); comment each with the rule it checks.
- **Commit**: `Add tests of the evaluation rules`

### S2.7 Detailed results

- **Files**: `TF/Engine/DetailedResult.h`, executors, `TF/ThermoEngine.h/.cpp`, `PY/pyThermoEngine.cpp`,
  `PY/pyDetailedResult.cpp`, `pytests/test_detailed.py`.
- **Do**: 08 §4. The executor takes an optional `std::vector<std::pair<std::string, ThermoPropertiesSubstance>>*`
  and pushes after each slot step; `thermoPropertiesSubstanceDetailed` passes it; the normal path passes `nullptr`
  (no cost). Not memoized.
- **Test**: `total` equals `thermoPropertiesSubstance`; last step equals total before `complete/fallback`.
- **Commit**: `Add per-method breakdown of substance properties`

### S2.8a Confirm the declared effects

- **Files**: `TF/Methods/MethodCatalog.cpp`, `docs/redesign/06-methods-and-executor.md` §8.3.
- **Do**: for every method, read its `thermoProperties` and confirm each property is set, added, read or untouched
  (look for `tps.X =`, `tps.X +=`, uses of the incoming `tps`). Fix the table and the catalog where the code
  disagrees. In particular: does `EmpiricalCpIntegration` set V; does `ConMolVol` read the incoming V or V°;
  `IdealGasLawVol`; the gas EoS H and S terms.
- **Commit**: `Confirm the declared effects of every method`

### S2.8 Python catalog and generated method reference

- **Files**: `PY/pyMethods.cpp` (`methodCatalog()`), `PY/docs.py`, `docs/reference/methods/*` (generated),
  CI step `python -m thermofun.docs generate --check docs/reference`.
- **Do**: 10 §2. **Release 0.9.0** (changelog, version bump by maintainer).
- **Commit**: `Generate the method reference from the catalog`

### S2.9 Effect-based composition (after S2.8a; may be done after phase 3)

- **Files**: `TF/Engine/Composition.h/.cpp`, `TF/Engine/SubstanceExecutor.cpp`, `TF/Engine/ReactionExecutor.cpp`,
  `TF/IO/Validate.cpp`, `tests/model/test_composition.cpp`, `pytests/test_method_order.py`.
- **Do**: 06 §8.2 order (base → transforms → setters → adders by rank → engine steps) and validation codes
  012–015. The legacy-specific rules of §7.2 that are not expressible as effects stay as explicit rules
  (H+, H2O vapor/solvent classification, CST used as base only without CPT, the steam RT ln P correction, reacDC).
  The slot-based path is removed only when the effect path gives identical results for every record.
- **Test**: golden subset and two-build comparison identical; for every bundled record with ≥ 2 methods,
  shuffling the order of `methods` in its v2 form gives bit-identical results; invalid combinations give their codes.
- **Commit**: `Compose methods by their declared effects`

---

## Phase 3 — v2 reader and writer (release 0.10.0)

### S3.1 Normalisation of quantities and units

- **Files**: `TF/IO/V2/V2Keys.h` (every v2 key as `inline constexpr const char*`; no string literal for a key anywhere
  else in the reader/writer — Q16), `TF/IO/V2/Normalize.h/.cpp`, `tests/model/test_normalize.cpp`.
- **Do**: 04 §3.1 rules 1–7 and §3.3 scoped units:
  ```cpp
  struct UnitScope { std::map<std::string, std::string> defaults; const UnitScope* parent = nullptr; };
  auto readQuantity(const json& node, const std::string& slot_si_unit, const UnitScope& scope,
                    Diagnostics& diag, const std::string& path) -> std::optional<Quantity>;
  auto writeQuantity(const Quantity& q, const std::string& slot_si_unit) -> json;
  auto scopedUnit(const std::string& si_unit, const UnitScope& scope) -> std::string;   // J/(mol.K) + {energy: cal} → cal/(mol.K)
  ```
- **Test**: every row of the 04 §3.1 table, every error code, `writeQuantity(readQuantity(x)) == x` for the
  shortest forms.
- **Commit**: `Read and write v2 quantities with units and uncertainties`

### S3.2 v2 reader for elements, substances, reactions + resolution

- **Files**: `TF/IO/V2/V2Reader.h/.cpp`, `TF/IO/Resolve.h/.cpp`, modify `TF/Database.cpp` (`fromFile`, `addRecord`,
  `fromJSONs`: route by `detectDocumentFormat`/`detectRecordFormat`), fixtures `pytests/data/v2/`:
  `minimal.json` (as `examples/v2/minimal.yaml` of 10 §5, in JSON), `hkf.json`, `minerals.json`,
  `gases.json`, `reactions.json`, `invalid/*.json` (one per error code).
- **Do**: 04 §2–§6. Map/list forms, `symbol` from the key, record fields of 04 §3.4–§3.6, method applications with
  `MethodCatalog::canonicalId`, parameters validated against the spec, segments/transitions, reference conditions,
  `thermodataset` defaults; then `Resolve`. Throw (with `Diagnostics::toString()`) if errors and
  `throw_on_error`.
- **Test**: fixtures load; computed properties of `minimal.json` equal those of the same species in the legacy
  databases (write the fixture by converting by hand from `aq17`/`slop98` records, so results must match).
- **Commit**: `Read v2 substances, reactions and elements`

### S3.3 v2 project-scoped records

- **Files**: V2Reader, `pytests/data/v2/methods.json`.
- **Do**: `methods`, `parameter_sets`, `data_sources`, `thermodataset` sections; check MethodDefinition against
  specs (`V2-MTH-010/011`); `thermodataset.solvent` sets the engine's default solvent when the engine is created
  from that database (only if the user did not set one — check `EnginePreferences.solvent_symbol` default
  `"H2O@"`; document the rule).
- **Commit**: `Read v2 methods, parameter sets, data sources and dataset metadata`

### S3.4 v2 writer and the migration round trip

- **Files**: `TF/IO/V2/V2Writer.h/.cpp`, `Database::toJson/save`, `pytests/io/test_roundtrip_legacy_v2.py`,
  `pytests/io/test_v2_writer_idempotent.py`.
- **Do**: 04 §8, §3.1 writer rules. For legacy-loaded records, write `unit` for parameters as the legacy unit when
  it was given (so HKF parameters stay in cal-based units in the file and stay readable), using a method-level
  `units:` mapping when all parameters of the application share the same energy/pressure units (shorter files).
- **Test**: 09 §4 — **the key acceptance test of the whole plan**: every golden database loaded as legacy,
  written as v2, re-loaded, gives the golden numbers.
- **Commit**: `Write v2 JSON; legacy to v2 round trip reproduces all reference results`

### S3.5 Validation

- **Files**: `TF/IO/Validate.h/.cpp`, `Database::validate()/loadDiagnostics()`, `PY` bindings,
  `pytests/io/test_validate.py`.
- **Do**: 03 §7 checks. Element/charge balance through ChemicalFun (`Database::parseSubstanceFormula`).
- **Test**: each invalid fixture yields exactly its code; bundled databases: list the warnings they produce and
  keep the list as expected output (they are data issues, not code issues).
- **Commit**: `Validate databases and report all problems`

### S3.6 YAML

- **S3.6a (Python)** Files: `PY/io/__init__.py`, `PY/io/yaml_loader.py`, `PY/__init__.py` wrappers, add `pyyaml`
  to `python/pyproject.toml.in` dependencies and `environment.devenv.yml`. Do: 08 §1 Python YAML. Test:
  `test_yaml_json_equivalence.py`. Commit: `Load v2 YAML files in Python`.
- **S3.6b (C++, optional)** Files: `TF/IO/Yaml.h/.cpp`, `cmake/modules/ThermoFunFindDeps.cmake`
  (`find_package(yaml-cpp)` if `TFUN_USE_YAML`), `ThermoFunConfig.cmake.in` (find it for users if ON).
  Do: YAML text → `nlohmann::ordered_json` (scalars: try int, then double, then bool/null, else string; keep
  quoted strings as strings). Test: same equivalence test in C++. Commit: `Optional YAML reading in C++`.

### S3.7 Dataset directory loader and include

- **Files**: `TF/IO/DatasetLoader.h/.cpp`, `examples/v2/dataset/...`, `pytests/io/test_dataset_dir.py`.
- **Do**: 04 §5. In C++ only JSON files are read unless YAML is enabled; the Python wrapper handles YAML
  directories by loading each file itself and passing JSON.
- **Commit**: `Load ThermoHub dataset directories`

### S3.8 Legacy writer

- **Files**: `TF/IO/Legacy/LegacyWriter.h/.cpp`, `Database::save(path, Legacy)`, `pytests/io/test_legacy_writer.py`.
- **Do**: 05 §6. Also: `Substance::jsonString()` for v2-loaded records returns the v2 JSON of the record (05 §5.2,
  Q10, decided).
- **Test**: 09 §4; `tests/gems_export` still passes.
- **Commit**: `Write records in the legacy format`

### S3.9 Snapshot, property-source preference

- **Files**: `TF/Engine/Snapshot.h/.cpp`, `ThermoEngine`, `EnginePreferences`, bindings, tests.
- **Do**: 08 §4; SHA-256: add a small public-domain implementation in `TF/Common/Sha256.cpp` (no new dependency).
- **Commit**: `Add calculation snapshot and property-source preference`

### S3.10 Pinned schema and JSON Schema validation

- **Needs**: a `thermohub-schema` release. If none exists yet, skip and note it in 12-open-questions.md.
- **Files**: `schemas/thermohub/<version>/` (JSON Schema + examples, copied from the release with its licence),
  `PY/validate.py`, CI step.
- **Do**: 08 §5 validate; make all `pytests/data/v2` fixtures and writer outputs pass the schema; differences
  between the schema and the ThermoFun profile become issues for the schema or reader changes (one commit each).
- **Release 0.10.0**.
- **Commit**: `Validate v2 files against the ThermoHub JSON Schema`

---

## Phase 4 — Conversion of data

### S4.1 Converter CLI

- **Files**: `PY/convert.py`, `pytests/io/test_convert.py`.
- **Do**: 08 §5: `--split` writes the directory layout; `--report` Markdown with sections: Summary (counts),
  Quirks applied (code, count, records), Unread legacy keys, Unresolved references, Methods used, Uncertainty
  convention notice.
- **Commit**: `Add python -m thermofun.convert`

### S4.2 v2 copies of the bundled databases

- **Files**: `Resources/databases/v2/<name>.yaml` (or `.json` if YAML in git is not wanted — §12 Q14).
- **Do**: convert each; add golden variants loading the v2 copies (no new reference files: they must match the
  legacy references).
- **Commit**: `Add v2 versions of the bundled databases`

### S4.3 Downstream CI

- **Do**: 09 §6 item 5. **Commit**: `Build Reaktoro's ThermoFun extension in CI`

---

## Phase 5 — Shared constants and Tier-1 methods (release 0.11.0)

### S5.1 Constants of reference methods into parameter structs (one step per method family)

- **Files**: per family, e.g. `TF/Substances/Solvent/WaterHGKreaktoro.*` and the Reaktoro water headers; new
  `TF/Methods/ParameterSets/WaterHGKConstants.h/.cpp`.
- **Do**: move the numeric constants into a `struct XConstants` with a `static const XConstants& defaults()`;
  the model takes a `const XConstants&` (default argument = defaults). Export defaults as a `MethodParameterSet`
  record (`<Id>_constants_v1`) via `MethodCatalog::builtinParameterSets()`. **No value may change** (copy
  literally; golden suite identical).
  Order: JN91 (Reaktoro, GEMS), Sverjensky14, Fernandez97, HGK (Reaktoro, GEMS), WP95, ZhangDuan05, HKF universal
  constants (η, θ, ψ in the HKF classes).
- **Commit**: `Move <family> constants into a parameter set` (one per family)

### S5.2 Using parameter sets from data

- **Do**: binding builds the constants from the set named by `parameter_set` (or the built-in default set), then
  applies inline `parameters` of the same names as overrides (Q7, decided) and reports each as `V2-MTH-016`.
- **Test**: a fixture with a modified constant gives a different, expected value; default gives golden.
- **Commit**: `Use method parameter sets from data`

### S5.3 Optional Tier-1 symbolic methods

- **Files**: `TF/Methods/Symbolic/SymbolicExpression.h/.cpp`, `Evaluator.h`, CMake option `TFUN_USE_SYMENGINE`.
- **Do**: parse `MethodDefinition::expression` with SymEngine at binding; evaluate by walking the expression tree
  with a function templated on the scalar type (`real` or `PlainReal`) supporting `+ - * / pow exp log sqrt`;
  variables `T`, `P` and parameter names. Run `tests` reference cases (D63) in `validate()`.
  What the expression defines is decided in §12 Q15 before starting. Recommended: the Method record gives one
  expression for the **Gibbs energy increment** ΔG(T, P) relative to the reference state; at binding, SymEngine
  differentiates it symbolically to get S = −∂G/∂T, Cp = −T ∂²G/∂T², V = ∂G/∂P, H = G + TS, and the evaluator
  computes all five (with autodiff for ddt/ddp). This avoids symbolic integration (limited in SymEngine) and
  numerical integration.
- **Commit**: `Optional symbolic (Tier-1) methods`

---

## Phase 6 — v2 by default (release 1.0.0)

### S6.1 Documentation

- **Do**: 10 §1 structure, tutorials and how-tos with tested code, `examples/v2/`. Update `README.md` examples to v2.
- **Commit**: several, one per page group.

### S6.2 Deprecations

- **Do**: 05 §8 list: `[[deprecated("use Substance::record().methods")]]` etc.; Python `warnings.warn(...,
  DeprecationWarning, stacklevel=2)` in thin Python wrappers (pybind11 cannot mark deprecated directly; wrap in
  `PY/__init__.py`). Internal uses of deprecated functions get `#pragma` suppression or are switched to records.
- **Commit**: `Deprecate the enum and positional-parameter API`

### S6.3 Cleanup and release

- **Do**: delete `tests/model/frozen/`; keep the golden suite; release notes; version 1.0.0.
- **Commit**: `Release 1.0.0`

### Optional later steps

- Packaging with scikit-build-core (§12 Q9).
- C++ JSON Schema validation behind an option (§12 Q8).
