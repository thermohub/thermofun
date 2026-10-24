# 12 — Decision log

Every design question raised while writing this plan, the decision, and where it is applied. Other files refer to
these as "decision Qn". **Final** decisions were discussed and agreed; **provisional** ones were taken at the plan's
recommendation and can be revisited before the step that needs them. Vocabulary and v2-format details are expected to
change with the ThermoHub v2 schema work (Q12, Q16); the plan keeps them in one place each so changes stay small.

To add a decision: append an entry with the next number, the date, and the files/steps it changes.

## Summary

| Q | Topic | Decision | Status | Applied in |
|---|---|---|---|---|
| Q1 | Python naming | C++-mirrored API in camelCase; pure-Python modules in snake_case | final | 02 §3, 08 |
| Q2 | Combining several methods on one record | per-property **effects** (set/add/read), engine derives the order | final | 06 §8, S2.8, S2.10 |
| Q3 | Unit conveniences in v2 files | read `"number unit"` strings and scoped `units:`; never write strings | final | 04 §3.1, §3.3, S3.1 |
| Q4 | YAML in C++ | yaml-cpp, optional (`TFUN_USE_YAML=OFF`); Python always reads YAML | final | 01 §9, 02 §5, S3.6 |
| Q5 | Units implementation | existing converter + UCUM front end; LLNL units only if needed | final | 07, S1.4 |
| Q6 | Molar mass | kg/mol in records; g/mol in files (bare numbers) and in the legacy API | final | 03 §5, 04 §3.5 |
| Q7 | Parameters with a parameter set | allowed; inline values override set entries, reported as `V2-MTH-016` | final | 04 §3.5.1, 06 §6, S5.2 |
| Q8 | JSON Schema validation in C++ | no; Python/CI only | final | 01 §6, 09 §6 |
| Q9 | Bindings and packaging | keep pybind11 and current packaging; scikit-build-core later, optional | final | 01 §10, 11 (optional steps) |
| Q10 | `jsonString()` of v2 records | the record's v2 JSON | final | 05 §5.2, S3.8 |
| Q11 | `m_expansivity` / `m_compressibility` | values correct, unit labels swapped; v2 parameters `alpha0` (1/K), `kappa0` (bulk modulus, Pa) on each HP98 method | final | 05 §4, 06 §5.1, S1.8 |
| Q12 | Vocabularies | keep today's 7 substance classes and `crystal`; alias tables; provisional values | final | 03 §2, S1.3 |
| Q13 | Documentation site | Markdown in `docs/` by Diátaxis; hdoc for C++ API | final | 10 |
| Q14 | v2 copies of bundled databases | one YAML file per database in `Resources/databases/v2/` | final | S4.2 |
| Q15 | Tier-1 (symbolic) methods | authors write source forms (Cp, ΔCp, V, log K, ΔrCp, ΔrV, G); a SymPy tool generates G/ΔrG; engine differentiates symbolically; substances and reactions | final | 06 §10, S5.3–S5.5 |
| Q16 | v2 format details chosen by the plan | implement as written, provisional, all keys in `IO/V2/V2Keys.h` | final | 04, S3.1 |
| Q17 | `H+` special case | keep (all properties zero for symbol `H+`) | final | 06 §7.1 |
| Q18 | Choice of aqueous solvent | keep today's rule; `thermodataset.solvent` sets it when given | final | 06 §7.1, S3.3 |
| Q19 | Legacy uncertainty convention | read as u (k = 1), flag `convention_unknown`; configurable coverage factor | final | 05 §3.1, 07 §5 |
| Q20 | Where the legacy↔v2 converter lives | in ThermoFun; `thermohub-tools` calls it | final | 08 §5, S4.1 |
| Q21 | Golden references | committed subset (≤ 3 MB, tol. 1e-10) + two-build comparison on every PR (tol. 1e-12) | final | 09 §1, S0.1, S0.4 |
| Q22 | Landau mineral volume at exactly Tr, Pr | confirm by calculation; fix separately after the refactoring | final | 06 §8.3 |
| Q23 | Order of external sources | PHREEQC-format + tabulated log K → HP2011 → Shomate/NASA/SGTE → conventions → DFT → DEW | provisional | 13 §7, Phase 7 |
| Q24 | Energy reference conventions | `energy_reference` field (apparent formation, Berman–Brown, SER, assigned enthalpy, DFT 0 K); convert at load | provisional | 13 §5.1, S7.5 |
| Q25 | Tabulated methods | implement `TabulatedLogK` (legacy id 8) and `TabulatedProperties` (spline on Cp, integrate) | provisional | 13 §6, S7.2 |
| Q26 | Standard pressure 1 atm vs 1 bar | `standard_pressure` convention field; gas conversion at load | provisional | 13 §5.2, S7.5 |
| Q27 | DFT-derived data | separate `sourcetdb`, quality `supplemental`, origin `calculated`; no G(T) model by default, Bartel optional | provisional | 13 §4, §5.3, S7.6 |
| Q28 | Activity models (Debye–Hückel, SIT, Pitzer) | out of ThermoFun's scope (solver's job); kept as unknown fields by adapters | provisional | 13 §2.1 |

## Details

### Q1. Python naming — 2026-10-24, final
The Python module mirrors C++ names (one reference for both languages). New pure-Python modules
(`thermofun.convert`, `thermofun.validate`, `thermofun.io`, `thermofun.docs`, `thermofun.methods`,
`thermofun.sources`) use snake_case. Existing snake_case names (`with_autodiff`) stay.

### Q2. Combining methods — 2026-10-24, final
Each method declares for G, H, S, Cp, V whether it sets, adds, reads or leaves the property, plus `base`, `transform`
and `rank`. Order: base → transforms → setters → adders (by rank) → engine steps (U, A, Cv; conventions; fallback).
Two setters of one property are an error. File order does not change results. Reason: in real data the volume is
replaced by some methods and added by others (Cp + Landau + Murnaghan, 60 minerals), and Akinfiev–Diamond reads the
previous result; list order alone would let a swapped entry silently give wrong volumes. Phase 2 keeps today's slot
executor; S2.10 switches to effects once all records give identical results. Proposed to the schema as an
`effects` block on Method records.

### Q3. Unit conveniences — 2026-10-24, final
The reader accepts `"-237.18 kJ/mol"` strings and scoped `units:` mappings (file, record, method application). The
writer never writes strings; it writes `units:` on a method application when all its parameters share non-SI
energy/pressure units and the file gets shorter. Proposed to the schema.

### Q4–Q5, Q8–Q9, Q13–Q14, Q17–Q20 — 2026-10-24, final
Taken as recommended; see the summary table and the files listed there.

### Q6. Molar mass — 2026-10-24, final
SI (kg/mol) in records; default file unit g/mol; legacy API returns g/mol.

### Q7. Parameter sets — 2026-10-24, final
`parameter_set` and inline `parameters` may be combined; inline values override; each override reported as info
`V2-MTH-016` and visible in detailed results.

### Q10. `jsonString()` — 2026-10-24, final
For v2-loaded records: the v2 JSON of the record (the original text if unedited and read from JSON). Legacy records
keep their original legacy text. Legacy JSON of any record via `Database::toJson(DataFormat::Legacy)`.

### Q11. Expansivity and compressibility — 2026-10-24, final
Example Corundum (aq17): `m_expansivity` 4.19e-5 → α° = 4.19e-5 1/K; `m_compressibility` 2520 → κ = 2520 kbar,
both Holland & Powell (1998) values. Only the unit labels are swapped, and "compressibility" holds the bulk modulus.
v2: `alpha0` (1/K) and `kappa0` (Pa, written e.g. `{value: 2520, unit: kbar}`) as parameters of every `HP98Min` and
`HP98Landau` application; the legacy writer writes them once at record level.

### Q12. Vocabularies — 2026-10-24, final
Seven substance classes (`component`, `gas_fluid`, `aqueous_solute`, `water_solvent`, `surface_species`,
`site_moiety`, `other`) and aggregate state `crystal`. Every vocabulary has an alias table (start: `crystalline` →
`crystal`); readers accept aliases, writers write canonical values. Values are provisional and live in
`Model/Vocabulary.cpp` only.

### Q15. Tier-1 methods — 2026-10-24, final
Option D: a Method record has a `source` block written by the author (forms `cp`, `cp_increment`, `volume`,
`gibbs`, `gibbs_increment`, `logk`, `drcp`, `drv`, `drgibbs`, `drgibbs_increment`; Phase 7 adds `shomate`, `nasa7`,
`nasa9`, `sgte`) and a generated `expression` block (G or ΔrG, per segment, with continuity constants) written by
`python -m thermofun.methods build` (SymPy). The engine parses only the generated potential and derives the other
properties by symbolic differentiation (SymEngine, optional build feature). Both forms are checked against the
record's test cases (D63). Covers substances and reactions. Goal: adding a closed-form model is a data pull request,
easy for users and contributors.

### Q16. Provisional v2 format details — 2026-10-24, final
Top-level `thermohub` version key; sections as maps keyed by symbol (lists accepted); `record_type` on single
records; record-level `applicability`; `data_sources` list; `segments` and `transitions`; `thermodataset.solvent`;
`include`. Implemented as written, marked provisional in 04, all keys in `IO/V2/V2Keys.h`; renamed keys stay
readable as aliases for one minor version after the schema is released.

### Q21. Golden references — 2026-10-24, final
Committed subset (~400 substances + ~150 reactions selected to cover every method, rule and database; ≤ 3 MB;
20-point grid; relative tolerance 1e-10) plus a full two-build comparison against the `golden-baseline` tag on every
pull request (all records; 90-point grid; tolerance 1e-12; baseline build cached in CI).

### Q22. Landau mineral volume at Tr, Pr — 2026-10-24, final
At exactly T = Tr and P = Pr, `HP98Min` sets V = V° (`SolidMurnaghanHP98.cpp:63`) instead of adding, dropping a
Landau excess volume. Confirm with V of Sillimanite at 298.15 K and 298.16 K, 1 bar; if confirmed, fix in a separate
change after the refactoring and regenerate the golden references in that change only.

### Q23–Q28. External data sources — 2026-10-24, provisional
Taken at the recommendations of [13-data-sources.md](13-data-sources.md) §5–§8; to be confirmed before Phase 7.
