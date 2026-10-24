# 12 — Open questions (to discuss one by one)

Each question: why it matters, options, **recommendation**, and the step that needs the answer. Until a question
is answered, implement the recommendation. Record the decision here (`Decision:` line) when taken.

Suggested discussion order (blocking first): ~~Q21~~, ~~Q11~~, ~~Q2~~ (decided), Q3, Q7, Q12, Q16, Q10, Q6, Q1, Q4, Q5, Q15, Q13,
Q14, Q9, Q8, Q17–Q20, Q22.

---

### Q1. Python naming: camelCase only, or also snake_case?
The module mirrors C++ names today (`thermoPropertiesSubstance`), but a few names are already snake_case
(`thermofun.with_autodiff`). Python users expect snake_case (PEP 8).
- A: camelCase only (today's rule, one reference for both languages).
- B: add snake_case aliases for everything (two names per function, more docs).
- C: camelCase for C++-mirrored API, snake_case for pure-Python modules (convert, validate, io).
**Recommendation: C** (written into 02 §3). Needed by: S1.11.

### Q2. Several methods in one slot, and their order (feedback to the schema)
Today a record has one method per slot (General → T-correction → P-correction). The design summary allows a list
of method applications whose contributions are summed (D45, D61). Real data needs both chaining (Cp integration,
then a volume EoS acting on its result) and adding (Landau terms).
- A: the Method record declares a `stage`/`slot`; list order is normative within a stage; a new `Additive` stage
  adds contributions.
- B: list order is the only rule; each method transforms the previous result.
- C: keep one-per-slot in v2 too.
**Recommendation: A**, proposed to `thermohub-schema` as a Method field. ThermoFun phase 2 implements C (refuses
two in one slot) and keeps the `slot` field ready. Needed by: after phase 2 (06 §8).

**Decision (2026-10-24): per-property effects** (a refinement of A). Each method declares, for G, H, S, Cp, V,
whether it sets, adds, reads or leaves the property, plus `base`, `transform` and a `rank`. The engine orders
methods as base → transforms → setters → adders (by rank) → engine steps, and rejects conflicts (two setters of
one property). File order does not change results. Phase 2 keeps the slot executor; step S2.9 switches to effects
once every record gives identical results. Proposed to the schema as an `effects` block on Method records.
Cases and the effect table: 06 §8.

### Q3. Unit conveniences in v2 files
Two Cantera-style conveniences: values as strings `"-237.18 kJ/mol"`, and scoped `units:` defaults.
- A: accept both when reading, never write strings; write `units:` per method application when it shortens files.
- B: accept only `{value, unit}` objects (strict, as in the design examples).
**Recommendation: A** for reading; propose both to the schema (they keep files paper-like, e.g. HKF parameters in
cal and bar). Needed by: S3.1.

### Q4. YAML library in C++
- A: yaml-cpp (on conda-forge, used by Cantera, slower — irrelevant for MB-size files).
- B: rapidyaml (much faster per its own benchmarks, less common in packaging).
- C: no YAML in C++ (Python only).
**Recommendation: A, optional (`TFUN_USE_YAML=OFF` by default).** Needed by: S3.6b.

### Q5. Units implementation
- A: keep the existing converter + UCUM front end (07).
- B: adopt LLNL `units` (runtime, UCUM domain), new dependency.
**Recommendation: A**; revisit if UCUM coverage problems appear. Needed by: S1.4.

### Q6. Molar mass in the model: kg/mol (SI) or g/mol?
Chemists write g/mol; SI is kg/mol; the legacy API returns g/mol.
**Recommendation: SI (kg/mol) in records, g/mol in the legacy API and in files by default unit `g/mol`**
(slot default unit = `g/mol` for molar mass only, documented). Needed by: S1.5.

### Q7. `parameters` together with `parameter_set` (O20)
- A: exclusive (design sketch).
- B: allowed; inline parameters override entries of the set (useful to change one water constant).
**Recommendation: B.** Needed by: S5.2 (reader accepts both from S3.2).

### Q8. JSON Schema validation inside the C++ library
**Recommendation: no** (validate in Python/CI); revisit if C++-only users need it (pboettch validator supports
draft-7; check what LinkML emits). Needed by: later.

### Q9. Python packaging and bindings
**Recommendation:** keep pybind11 and the current packaging during the restructuring; consider scikit-build-core
afterwards (separate change); nanobind only if binding size/compile time becomes a problem.

### Q10. What `jsonString()` returns for v2 records
- A: the v2 JSON of the record. B: legacy JSON (via LegacyWriter). C: the original YAML/JSON text.
**Recommendation: A** (C for unedited records read from JSON — same thing). Callers that need legacy use
`Database::toJson(DataFormat::Legacy)`. Needed by: S3.8.

### Q11. Physical meaning of `m_expansivity` / `m_compressibility` (data question)
All bundled data label `m_expansivity` with `kbar` and `m_compressibility` with `1e-05/K`.

**Decision (2026-10-24):** the values match Holland & Powell (1998) under their names (e.g. Corundum α° = 4.19e-5
1/K, κ = 2520 kbar); only the unit labels are swapped, and "compressibility" holds the bulk modulus. In v2 they
are the parameters **`alpha0`** (thermal expansion at 298 K, SI 1/K) and **`kappa0`** (bulk modulus at 298 K, SI Pa,
written e.g. `{value: 2520, unit: kbar}`) **of each HP98 method application** (`HP98Min`, `HP98Landau`), repeated
when both are used; the legacy writer writes them once at record level. Details: 05 §4 note, 06 §5.1.

### Q12. Vocabularies
- Substance classes: keep today's seven (`component`, `gas_fluid`, `aqueous_solute`, `water_solvent`,
  `surface_species`, `site_moiety`, `other`) or the richer list of the design summary (`mineral`, `pure_fluid`,
  `solid_solution`, …)?
- Aggregate state: `crystal` (legacy, Reaktoro) or `crystalline` (design examples)?
**Recommendation:** today's lists (lossless), aliases accepted on reading; richer classes added when the schema
defines them, with a mapping table. Needed by: S1.3.

### Q13. Documentation site
**Recommendation:** Markdown in `docs/` organised by Diátaxis (10 §1), hdoc for C++ API; add MkDocs Material only
if a searchable site is wanted. Needed by: S6.1.

### Q14. v2 copies of the bundled databases: YAML or JSON, one file or directories?
**Recommendation:** one YAML file per database in `Resources/databases/v2/` (readable diffs; Python reads YAML;
C++ users without YAML use the JSON written by `convert --format json` on demand), plus the directory form only in
`thermohub-data`. Needed by: S4.2.

### Q15. Tier-1 expression semantics
**Recommendation:** an expression for ΔG(T, P) relative to the reference state; other properties by symbolic
differentiation at binding (11 S5.3). Needed by: S5.3.

### Q16. Profile choices to confirm with `thermohub-schema`
Top-level `thermohub: <version>` key; sections as maps keyed by symbol (lists accepted); `record_type` on single
records; record-level `applicability`; `data_sources` list on records; `transitions` and `segments` in method
applications; `thermodataset.solvent`. **Recommendation:** propose all to the schema discussion as written in 04.
Needed by: S3.2 (implemented as profile, adapted when the schema is released).

### Q17. `H+` handled by its symbol
The engine returns zeros for the substance whose symbol is exactly `H+` (06 §7.1). **Recommendation:** keep for
legacy; for v2 keep the same rule (a convention flag on the record is a schema question).

### Q18. Choosing the aqueous solvent
Today: `EnginePreferences.solvent_symbol` (default `H2O@`), else the last `water_solvent` substance in map order.
**Recommendation:** keep; v2 `thermodataset.solvent` sets it when present and the user has not changed it.

### Q19. Legacy uncertainty convention
Legacy `errors` have no stated convention. **Recommendation:** read as u (k = 1, today's behaviour), flag
`convention_unknown`, and let a dataset owner set `LegacyReadOptions.error_coverage_factor` when converting.

### Q20. Where the converter lives
D42 puts adapters and migrations in `thermohub-tools`. **Recommendation:** the legacy↔v2 mapping lives in
ThermoFun (it must read legacy files anyway); `thermohub-tools` calls `thermofun.convert` instead of copying the
mapping.

### Q21. Golden reference data in git (~15–25 MB compressed)
- A: commit it (simple, works offline).
- B: generate from a baseline build in CI only (no data in git, slower CI, needs the baseline build).
- C: commit a smaller grid (A with fewer points) + B weekly on a dense grid.
**Recommendation: C.** Needed by: S0.1 (blocking).

**Decision (2026-10-24): C.** Committed subset (~400 substances + ~150 reactions, ≤ 3 MB, 20-point grid, rel. tolerance 1e-10) plus a full two-build comparison against the `golden-baseline` tag (all records, 90-point grid, rel. tolerance 1e-12) **on every pull request**, with the baseline build cached in CI. Details: 09 §1, steps S0.1 and S0.4.

### Q22. Volume of Landau minerals at exactly the reference T and P (possible bug, found while answering Q2)
At T = Tr and P = Pr, `HP98Min` sets V = V° (`SolidMurnaghanHP98.cpp:63`) instead of adding its volume, which drops
the Landau excess volume set before it. V at exactly 298.15 K / 1 bar may differ from V just next to it.
**Recommendation:** confirm with a small calculation (V of Sillimanite at 298.15 K and 298.16 K, 1 bar); if
confirmed, open an issue and fix it in a separate change **after** the refactoring, regenerating the golden
references in that change only. Needed by: not blocking.
