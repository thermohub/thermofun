# 09 — Testing

The refactoring is safe only if every step proves that results did not change. Five kinds of tests do that.

| # | Test | Guards | Where |
|---|---|---|---|
| 1 | Golden references | numbers (values, derivatives, errors, statuses) | `pytests/golden/` |
| 2 | Legacy reader equivalence | the new reader fills exactly what the old parser filled | `tests/model/test_legacy_equivalence.cpp` |
| 3 | Round trips | legacy → v2 → engine = golden; writers idempotent | `pytests/io/` |
| 4 | Unit tests | Ucum, Normalize, Vocabulary, Diagnostics, MethodCatalog, FormatDetect, executor rules | `tests/model/`, `pytests/io/` |
| 5 | Downstream | Reaktoro's ThermoFun extension still builds and passes | CI job |

## 1. Golden references (Phase 0)

### 1.1 What is computed

`pytests/golden/make_references.py` (run by hand, only in steps that say so):

- **Databases**: every file of `Resources/databases/*.json`, plus `pytests/**/*.json` that load as a database,
  plus the synthetic fixtures of §3. Each database is loaded alone.
- **Engines** (preference variants): `default` (all defaults); `gas` (`apply_pressure_correction_to_gas_props =
  True`, `cancel_fugacity_coefficient_in_gas_props = False`); `errors` (`propagate_parameter_errors = True`, only on
  the synthetic fixtures and `test-hkf-thermofun.json`, to keep it fast); `steam` (`EngineConventions
  water_properties = STEAM_TABLES`) and `berman` (`apparent_properties = BERMAN_BROWN`) on `aq17` and `mines16`.
- **Grid**: T = 273.15, 298.15, 373.15, 623.15, 1273.15 K × P = 0 (saturation), 1e5, 1e8, 5e8 Pa → 20 points.
  Use the same grid for reactions.
- **Quantities**:
  - substances: `gibbs_energy, enthalpy, entropy, heat_capacity_cp, volume` (and `heat_capacity_cv,
    internal_energy, helmholtz_energy` for the `default` engine only) — each `val, ddt, ddp, err, sta`;
  - reactions: `log_equilibrium_constant, reaction_gibbs_energy, reaction_enthalpy, reaction_entropy,
    reaction_heat_capacity_cp, reaction_volume`;
  - solvent records (class water_solvent): `propertiesSolvent` (density, Alpha, Beta, dAldT and their derivatives)
    and `electroPropertiesSolvent` (epsilon, bornZ, bornY, bornQ, bornX and derivatives);
  - also the returned `P` (for P = 0 the engine returns the saturation pressure).
- **Exceptions**: if a call raises, store `{"error": "<first line of message>"}` for that point; the test compares
  the message.

### 1.2 Storage

One gzip JSON per database and engine variant: `pytests/golden/data/<db>__<variant>.json.gz`:

```json
{"thermofun_commit": "<sha>", "grid": {"T": [...], "P": [...]},
 "substances": {"H2O@": {"gibbs_energy": [[val, ddt, ddp, err, "sta"], ...20 points]}, ...},
 "reactions": {...}, "solvent": {...}}
```

Numbers are written with `repr(float)` (exact). Expected size: ~5500 substances × 20 points × 5–8 properties
× 4 numbers ≈ 3–4.5 M numbers ≈ 15–25 MB gzipped for all databases. If the total exceeds 25 MB, drop the
`steam`/`berman` variants for the large databases and keep them for one small one. Keep the files in git (they
are the baseline); do not use Git LFS (contributors may not have it).

### 1.3 Comparison (`pytests/golden/test_golden.py`)

- Parametrised over `(db, variant)`; recomputes and compares.
- `val`, `ddt`, `ddp`: `math.isclose(a, b, rel_tol=1e-12, abs_tol=1e-12 * scale)` where `scale` = max(|b|, 1)
  for energies; exact equality of NaN-ness. Rationale: the refactoring must not change floating-point
  operations; a looser tolerance hides reordering mistakes. A step that legitimately changes floating-point
  order (none is planned) must say so and loosen the tolerance for that step only.
- `err`: same tolerance. `sta`: exact string equality of the status code (not of the message).
- Errors: same first line.
- On failure, print the first 20 differences as a table: db, record, property, T, P, expected, got.

### 1.4 Differential mode (optional, step S0.4)

`pytests/golden/compare_builds.py BASE_PREFIX NEW_PREFIX` runs two installed builds (e.g. the `golden-baseline`
tag and the working tree) on a denser grid (15 T × 6 P) in two subprocesses and compares. Used in CI weekly, not
on every push.

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
Saxena Cp (expect: no-op), Berman88Vol, Dorogokupets88Vol, BirchMurnaghanGott97, HP11Tait (no-op), CGF, SRK, PR78,
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
4. Add a weekly job: differential mode (§1.4).
5. Add a job (allowed to fail at first): clone Reaktoro, build its ThermoFun extension against the installed
   ThermoFun, run its ThermoFun tests.
6. Phase 3: validate `pytests/data/v2/**` and converted databases against the pinned JSON Schema
   (`pip install jsonschema`).

## 7. What a step must run before committing

```bash
cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build && pytest -ra .
```
All green. If the golden suite fails, the step is wrong: fix the code, never the references (rule 1 of §00).
