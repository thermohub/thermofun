# 09 — Testing

The refactoring is safe only if every step proves that results did not change. Five kinds of tests do that.

| # | Test | Guards | Where |
|---|---|---|---|
| 1 | Golden references: committed subset + two-build comparison | numbers (values, derivatives, errors, statuses) | `pytests/golden/`, CI job `golden-diff` |
| 2 | Legacy reader equivalence | the new reader fills exactly what the old parser filled | `tests/model/test_legacy_equivalence.cpp` |
| 3 | Round trips | legacy → v2 → engine = golden; writers idempotent | `pytests/io/` |
| 4 | Unit tests | Ucum, Normalize, Vocabulary, Diagnostics, MethodCatalog, FormatDetect, executor rules | `tests/model/`, `pytests/io/` |
| 5 | Downstream | Reaktoro's ThermoFun extension still builds and passes | CI job |

## 1. Golden references (Phase 0, decision Q21)

Two mechanisms, sharing one computation module (`pytests/golden/golden_common.py`):

| | **Committed subset** (§1.3) | **Two-build comparison** (§1.4) |
|---|---|---|
| Records | ~400 substances + ~150 reactions selected to cover every method, rule and database | every record of every database |
| Grid | 5 T × 4 P = 20 points | 15 T × 6 P = 90 points |
| Reference | `pytests/golden/data/*.json.gz` in git (target ≤ 3 MB total) | a build of the tag `golden-baseline`, computed on the fly |
| Tolerance | relative 1e-10 (survives compiler updates) | relative 1e-12 (same compiler, same machine) |
| Runs | every `pytest` run, locally and in CI | CI on every pull request; locally on demand |

Both compare values, `ddt`, `ddp`, `err` and the status code. The baseline is the last commit before the
refactoring, tagged `golden-baseline` by the maintainer after S0.1–S0.3 are merged.

### 1.1 What is computed (both mechanisms)

- **Databases**: every file of `Resources/databases/*.json`, plus `pytests/**/*.json` that load as a database,
  plus `tests/reaktoro_models/reaktoro_models_db.json` (the `new_models` methods, decision Q29), plus the synthetic
  fixtures of §3. Each database is loaded alone.
- **Engines** (preference variants): `default` (all defaults); `gas` (`apply_pressure_correction_to_gas_props =
  True`, `cancel_fugacity_coefficient_in_gas_props = False`); `errors` (`propagate_parameter_errors = True`);
  `steam` (`EngineConventions.water_properties = "STEAM_TABLES"`); `berman`
  (`apparent_properties = "BERMAN_BROWN"`). In the committed subset all variants run on the subset; in the
  two-build comparison `errors`, `steam` and `berman` run on `aq17`, `mines16`, `test-hkf-thermofun.json` and the
  synthetic fixtures (to bound CI time), `default` and `gas` on everything.
- **Grids**:
  - subset: T = 273.15, 298.15, 373.15, 623.15, 1273.15 K × P = 0 (saturation), 1e5, 1e8, 5e8 Pa;
  - full: T = 273.15, 283.15, 298.15, 323.15, 348.15, 373.15, 423.15, 473.15, 523.15, 573.15, 623.15, 673.15,
    873.15, 1073.15, 1273.15 K × P = 0, 1e5, 1e7, 1e8, 5e8, 1e9 Pa.
- **Quantities**:
  - substances: `gibbs_energy, enthalpy, entropy, heat_capacity_cp, volume, heat_capacity_cv, internal_energy,
    helmholtz_energy` — each `val, ddt, ddp, err, sta`;
  - reactions: `log_equilibrium_constant, reaction_gibbs_energy, reaction_enthalpy, reaction_entropy,
    reaction_heat_capacity_cp, reaction_volume`;
  - solvent records (class water_solvent): `propertiesSolvent` (density, Alpha, Beta, dAldT and their derivatives)
    and `electroPropertiesSolvent` (epsilon, bornZ, bornY, bornQ, bornX and derivatives);
  - also the returned `P` (for P = 0 the engine returns the saturation pressure), if the binding returns it.
- **Exceptions**: if a call raises, store `{"error": "<first line of message>"}` for that point; compare the message.

### 1.2 Comparison rule (shared function `compare(expected, got, rel_tol)`)

- `val`, `ddt`, `ddp`, `err`: `math.isclose(got, exp, rel_tol=rel_tol, abs_tol=rel_tol * max(abs(exp), 1.0))`;
  NaN equals NaN; ±inf must match exactly.
- `sta`: exact equality of the status code (not of the message).
- errors: same first line.
- Report: the first 20 differences as a table (database, variant, record, property, field, T, P, expected, got,
  relative difference) and the total count.
- A step that legitimately changes floating-point order (none is planned) must say so; the tolerance is never
  loosened globally.

### 1.3 Committed subset

**Selection** (`golden_common.select_subset()`, deterministic, run once in S0.1; the result is stored in
`pytests/golden/data/subset.json` so later code changes cannot change the selection):
1. For each database, group substances by signature `(class, aggregate_state, sorted legacy method ids,
   has defining reaction, has m_phase_trans_props, number of Cp intervals)`; take the first 3 symbols (sorted) of
   each group.
2. For each database, group reactions by `(sorted legacy method ids, has logKr, has drsm_volume, has drsm_heat_capacity_p)`;
   take the first 3 of each group.
3. Always include: every water_solvent record; `H+`; every substance defined by a reaction together with its
   reaction and the reaction's other species (so the subset database can compute them); every record of the
   synthetic fixtures.
4. Each subset is evaluated **inside its full database** (load the full database, compute only the selected
   symbols), so references between records stay valid.
5. If the gzipped total exceeds 3 MB, lower "first 3" to "first 2", then drop the `steam`/`berman` variants for the
   large databases; record what was dropped in `pytests/golden/README.md`.

**Files**: one gzip JSON per database and variant, `pytests/golden/data/<db>__<variant>.json.gz`:

```json
{"thermofun_commit": "<sha>", "grid": {"T": [...], "P": [...]},
 "substances": {"H2O@": {"gibbs_energy": [[val, ddt, ddp, err, "sta"], ... one per grid point]}, ...},
 "reactions": {...}, "solvent": {...}}
```

Numbers are written with `repr(float)` (exact). No Git LFS. The data is regenerated only in a step that says so,
which also updates `thermofun_commit`.

**Test**: `pytests/golden/test_golden.py`, parametrised over the data files, `rel_tol = 1e-10`.

### 1.4 Two-build comparison

**Scripts**:
- `pytests/golden/build_baseline.sh [TAG] [PREFIX]` — `git worktree add` the tag (default `golden-baseline`) into
  `.golden-baseline/src`, configure and build it with the same options as CI, install the library and the Python
  package into `PREFIX` (default `.golden-baseline/install`). `.golden-baseline/` is in `.gitignore`.
- `pytests/golden/compare_builds.py --base PREFIX --new PREFIX [--variant ...] [--db ...] [--max-report 20]`
  — runs `golden_common.compute(...)` in two subprocesses, one per build (`PYTHONPATH` set to each build's
  site-packages, `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH` to its `lib`), writes the results to temp files, compares with
  `rel_tol = 1e-12`, exits 1 on any difference. `--new` defaults to the active environment.
- The computation module itself is passed to both subprocesses from the **working tree**, so both builds are asked
  exactly the same questions. It must only use API that exists in the baseline (v0.7.3 API).

**CI job** `golden-diff` (Linux, every pull request and push to master):
1. restore the baseline install from the Actions cache, key = `golden-baseline-<tag commit sha>-<hash of
   environment.devenv.yml>`; on a miss, run `build_baseline.sh` and save the cache;
2. build and install the pull request (the existing job's steps);
3. run `compare_builds.py --base <cache> --new <env>`; upload the report as an artifact on failure.
First run: one extra build (~ as long as the normal build); later runs: cache restore only.

**Locally**: `bash pytests/golden/build_baseline.sh` once, then `python pytests/golden/compare_builds.py
--base .golden-baseline/install` after building the working tree.

## 2. Legacy reader equivalence (Phase 1)

Phase 1 replaces the parser. To prove it fills `Substance`/`Reaction`/`Element` exactly as before:

1. Copy today's `ThermoFun/Common/ParseJsonToData.cpp` to `tests/model/frozen/ParseJsonToData_v073.cpp`, wrap it in
   `namespace frozen_v073`, rename the free functions (`parseSubstance` → `frozen_v073::parseSubstance` etc.).
   It must compile against the current headers (it does at the start of phase 1).
2. `tests/model/test_legacy_equivalence.cpp`: for every record of every bundled file, compare
   `operator<<(stream, frozen_v073::parseSubstance(text))` with `operator<<(stream, Substance(text))` (the
   `operator<<` of `Substance` prints all fields including `thermoParameters`; extend it first, step S1.1, if it
   misses a field: `coefficient_errors`, `temperature_intervals`, `phase_transition_prop`,
   `m_landau_phase_trans_props`, `isothermal_compresibility`, `isobaric_expansivity`, method enums, limits,
   reference T/P, class, aggregate state, charge, reaction symbol, thermo calculation type). Same for reactions
   and elements.
3. Registered as ctest `legacy-equivalence`; it reads the database files from the source tree.

The frozen copy is deleted in Phase 6 (the golden suite remains).

## 3. Synthetic fixtures for methods not in the bundled data

`pytests/data/legacy-synthetic/` — one small legacy JSON per method that no bundled database uses (§05 §2):
Saxena Cp (expect: no-op), Berman88Vol, Dorogokupets88Vol, BirchMurnaghanGott97, CGF, SRK, PR78,
SternerPitzer, WaterIAPWS95Gems (no-op), WaterZhangDuan05, Sverjensky14Diel, Fernandez97Diel, Berman88Landau
(no-op), reactions FrantzMarshall, DolejsManning10, DrVolumeFT, LogKExtrap1T1, LogKExtrap2T, LogKLagrange (no-op).
Each file holds the record(s) needed (e.g. the solvent `H2O@` for solute or density models). Parameter values:
copy from the C++ tests (`tests/unit0*/src/main.cpp`, `tests/autodiff/src/*.cpp`) or examples where they exist;
otherwise use published values and cite them in a `"_comment"` key (kept by the reader as an unknown key).

HP98Sol and Anderson91 cannot be loaded from legacy files (§06 §5.1); the golden script builds them through the
API as in `examples/solute-hp98/main.cpp` and `examples/solute-anderson91/main.cpp` and stores them as
`synthetic-api__default.json.gz`.

## 4. Round-trip tests (Phase 3/4)

- `test_roundtrip_legacy_v2.py`: for each golden database: load legacy → `toJson(V2)` → load that → engine →
  compare with the golden file (same tolerance). **This proves the migration is numerically lossless.**
- `test_v2_writer_idempotent.py`: v2 → write → read → write: the two texts are equal.
- `test_legacy_writer.py`: legacy → record → legacy JSON → record: records equal (field by field, incl.
  uncertainties and `extra_json`); for unedited records the written text equals `original_text`.
- `test_yaml_json_equivalence.py`: the v2 fixtures written as YAML and as JSON load to equal records.

## 5. Unit tests

C++ (`tests/model`, one executable `tfun-unit`, the same minimal `check()` style as `tests/memoization`; no new
test framework):
- `test_ucum.cpp`: the translation table (§07 §3), every unit string in the bundled data, every catalog unit;
  `convertDifference` for °C; dimension checks.
- `test_normalize.cpp`: each row of §04 §3.1 table, each diagnostic code it can emit.
- `test_vocabulary.cpp`: every enum value round-trips string ↔ enum ↔ legacy code.
- `test_catalog.cpp`: every legacy substance id 0–43 and reaction id 0–14 in `enum_method_substance` /
  `enum_method_reaction` resolves; for every spec with `legacy_key`, the number of parameters equals the length of
  today's `units_to` list for that key in `ParseJsonToData.cpp`; ids unique; aliases unique.
- `test_format_detect.cpp`: documents and records of both formats, edge cases.
- `test_composition.cpp` (S2.10): order derived from effects; validation codes V2-MTH-012…015; shuffled method
  lists give bit-identical results.
- `test_executor_rules.cpp` (phase 2): one case per rule of §06 §7 (H+, H2O vapor, CON short-circuit, CST without
  CPT, steam RT ln P, REA default LGK, MRB early return, reacDC gas volume…), each compared with a value computed by
  the v0.7.3 engine (stored in the test).

Python (`pytests/io`): reader/writer/validate/convert behaviour, diagnostic codes, the YAML wrapper.

### Hashing for snapshots

`content_hash` (§08 §4) = SHA-256 of the v2 JSON of all records, written with sorted keys, no whitespace, numbers
in shortest round-trip form. Test: loading the same data from legacy and from its v2 conversion gives the same
hash.

## 6. CI changes (`.github/workflows/ci-build.yml`)

1. Existing jobs unchanged; `pytest` picks up `pytests/golden` and `pytests/io` automatically (`pytest.ini`).
2. Add to the Linux job: `cmake -DTFUN_USE_YAML=ON` (needs `yaml-cpp` in `environment.devenv.yml`, phase 3).
3. Add a Linux job with `-DTFUN_USE_AUTODIFF=OFF`; it runs ctest and the golden suite in "values only" mode
   (derivatives are 0 without autodiff; the test skips `ddt`/`ddp` when the existing `thermofun.with_autodiff` is false).
4. Add the job `golden-diff` (§1.4), on every pull request and push to master.
5. Add a job (allowed to fail at first): clone Reaktoro, build its ThermoFun extension against the installed
   ThermoFun, run its ThermoFun tests.
6. Phase 3: validate `pytests/data/v2/**` and converted databases against the pinned JSON Schema
   (`pip install jsonschema`).

## 7. What a step must run before committing

```bash
cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build && pytest -ra .
python pytests/golden/compare_builds.py --base .golden-baseline/install   # once the baseline exists
```
All green. If the golden suite fails, the step is wrong: fix the code, never the references (rule 1 of §00).
