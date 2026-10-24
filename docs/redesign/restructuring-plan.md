# ThermoFun restructuring plan — ready for the ThermoHub v2 data format, compatible with the current one

Status: proposal / working plan. Built from the ThermoHub redesign summary (decisions D1–D65, open points O1–O84)
and its examples (E1–E24), and from a survey of the ThermoFun code at v0.7.3.

Decision numbers (`Dnn`), open points (`Onn`) and examples (`Enn`) refer to `thermohub-redesign-summary.md` and
`thermohub-redesign-examples.md`.

---

## 0. Summary

ThermoFun today has **one data format baked into its core**: the ThermoHub/Thrift JSON (`TPMethods`,
`sm_gibbs_energy: {values, errors, units}`, enum-keyed method objects `{"29": "water_eos_hgk84_lvs83_gems"}`),
parsed in one file (`Common/ParseJsonToData.cpp`) straight into positional coefficient vectors
(`ThermoParametersSubstance::HKF_parameters[0..6]`, `Cp_coeff[interval][0..10]`, …). Method choice is three
enum slots per record (`methodGenEoS` / `method_T` / `method_P`) dispatched by hard-coded `switch` blocks in
`ThermoEngine.cpp`.

The plan puts a **format-neutral internal data model** between the files and the engine, following the v2
concepts (Quantity, MethodApplication, Method registry, MethodParameterSet, DataSource, ThermoDataSet). Both
formats become **readers into that model**:

```
 legacy ThermoHub JSON ──► LegacyReader ─┐
                                         ├──► Model (records, Quantities, MethodApplications) ──► Binding ──► Engine
 ThermoHub v2 YAML/JSON ─► V2Reader ─────┘                    ▲                                (compiled per substance)
                                                              │
                                        V2Writer / LegacyWriter (export, round-trip, migration)
```

Rules that hold throughout:

1. **No change in numbers.** Each phase must reproduce, to a tight tolerance, the results the current code gives
   on every database in `Resources/databases` and `pytests/`. A golden-reference suite is built first (Phase 0).
2. **Legacy stays first-class until 2.0.** Old files, old C++ / Python API calls, and `jsonString()` keep working.
   The old API is a thin view over the new model, marked deprecated only after v2 is the default.
3. **One mapping, used twice.** The legacy→model mapping in C++ is the *only* implementation of the field
   mapping. The bulk migration tool (D34, in `thermohub-tools`) calls it through the Python bindings
   (legacy → model → V2Writer) instead of re-implementing it.
4. **JSON is the contract** (D62). ThermoFun hand-writes its `from_json`/`to_json` against the JSON Schema
   released by `thermohub-schema`; generated C++ (quicktype) is optional and never authoritative.
5. **Discipline over scope.** Composite-method machinery (D7) is parsed and stored, not executed (D61). Tier‑1
   symbolic methods are an optional build feature. Nothing is built for one case only.

---

## 1. Where the current code stands (what has to change)

| Area | Today | Problem for v2 |
|---|---|---|
| Parsing | `Common/ParseJsonToData.cpp` reads legacy keys directly into `Substance`/`Reaction`/`Element`; unit fix-ups for bad data inline (`eos_hkf_coeffs` `(cal*K)/mol`, `m_expansivity` in `kbar`) | Format and model are fused; no place to add a second reader |
| Record model | `Substance` = pimpl with symbol, formula, `ThermoPropertiesSubstance` reference props, `ThermoParametersSubstance` (positional vectors), 3 method enums, `lowerT/upperT/...`, `referenceT/P` | No named parameters, no per-value metadata, one applicability range per record, no multiple methods per slot |
| Method selection | `MethodGenEoS_Thrift` / `MethodCorrT_Thrift` / `MethodCorrP_Thrift` enums; legacy method ids (0–43, reaction 0–14) mapped to them by `new_old_s_methodtype`, `setTPMethods_old` | Closed set, numeric ids, can't name a method by string (D22), can't add a method without editing three places |
| Dispatch | `switch` blocks in `ThermoEngine.cpp` (`thermoPropertiesSubstance`, `electroPropertiesSolvent`, `propertiesSolvent`, `thermoPropertiesReaction`) | No registry; ordering and special cases (HKF + gas, CON short-circuit) implicit in code |
| Parameters | `ThermoParametersSubstance` / `…Reaction`: `vd`/`vvd` fields + `coefficient_errors` map keyed by legacy JSON keys | Meaning of index *i* lives only in the model `.cpp` files |
| Units | `Common/Units.*` converter, `*` multiplication syntax (`J/(mol*K)`), `cal` = 4.184 J | UCUM uses `.` (`J/(mol.K)`); needs a UCUM front end |
| Uncertainty | `ThermoScalar.err` with first-order propagation; parameter errors via `uncertainCoefficients()` | Convention (u vs U vs CI) undocumented; no k, no kind (primary/derived) |
| Reactions | `reactants` list (sign: reactants −, products +, already as D18), `equation` string parser, reaction symbol = defined substance symbol, `reactionSymbol` on Substance | Fits D15/D18 well; needs `defining_reaction`, `preferred_property_source` (D16), `level` (D27) |
| Identity | maps keyed by `symbol`, last write wins with a warning | Fits D47 (symbol unique per ThermoDataSet); needs `class`, `sourcetdb` kept for composite identity (D24) |
| Project-scoped records | none (datasources is a list of strings; no Method or ParameterSet records) | D64 needs Method, MethodParameterSet, DataSource collections in `Database` |
| Hard-coded constants | water EoS, dielectric models, HKF universal constants, JN g-function in C++ | O84: should be overridable from `MethodParameterSet` records |
| Results | one `ThermoPropertiesSubstance` total | D65 asks for total + per-method split + auxiliary outputs |

What already fits and must be **kept**: autodiff-templated models (`real`, D56/D57), status per property, linear
error propagation (D10/D58), memoization, `ThermoBatch`, ChemicalFun formula parsing (O47), the D18 sign convention.

---

## 2. Target architecture

### 2.1 Source tree

Reorganise by concern. Old headers stay at their old paths as forwarding headers (`#include` of the new one) so
installed-header users and `tests/installed_headers` keep compiling.

```
ThermoFun/
  ThermoFun.h                      umbrella header (unchanged name)
  Model/                           format-neutral records (new)
    Quantity.h                     Quantity, Uncertainty, ValueMeta (quality, origin, source)
    Vocabulary.h                   PropertyName, SubstanceClass, AggregateState, Quality, Origin, ... (string <-> enum)
    StandardStateProperties.h      map<PropertyName, Quantity> + reference_conditions + convention
    MethodApplication.h            method id, named parameters, parameter_set ref, applicability, segments
    MethodDefinition.h             Method record (tier, kind, operates_on, applies_to_classes, contributes_to,
                                   parameter_definitions, implementation_id | expression, tests, citation)
    MethodParameterSet.h
    DataSource.h  ThermoDataSet.h
    ElementRecord.h  SubstanceRecord.h  ReactionRecord.h
  IO/
    FormatDetect.h/.cpp            sniff legacy vs v2 per file and per record
    Legacy/LegacyReader.*          today's ParseJsonToData.cpp, moved and re-targeted to Model
    Legacy/LegacyMethodMap.*       legacy method ids/keys -> v2 method ids + parameter names (table, §4)
    Legacy/LegacyWriter.*          Model -> legacy JSON (for GEMS export and old tools)
    V2/V2Reader.*  V2/V2Writer.*   v2 JSON (and YAML, optional) <-> Model
    V2/Normalize.*                 value-or-object union, shorthand expansion, unit normalisation (O39/O41)
    DatasetLoader.*                thermohub-data directory layout (D64): methods/, bibliography/, datasets/
  Units/                           Units.* moved here + Ucum.* front end
  Methods/
    Registry.h/.cpp                MethodRegistry: id -> {definition, factory}
    Binding.h/.cpp                 MethodApplication -> bound evaluator (parameters resolved, units converted)
    Substance/Solids, Solute, Gases, Solvent, ...   current model classes (moved, unchanged maths)
    Reaction/...                   current reaction models
    Symbolic/                      Tier-1 evaluator (optional, SymEngine)
  Engine/
    ThermoEngine.*                 public API unchanged
    Executor.*                     runs the method list of one record; replaces the switch blocks
    Solvent.*                      solvent resolution for HKF/AD/HP98 solutes
  Database.*  Element.*  Substance.*  Reaction.*   public classes, now views over Model records
  Batch/  Common/
```

### 2.2 The internal model (C++)

Kept small and close to the JSON shape (D44 flat document, D45 inline-vs-reference).

```cpp
struct Uncertainty {                      // D9, D11
    std::optional<double> standard;       // u, k = 1, SI
    double coverage_factor = 1.0;         // as read; stored value is always converted to k = 1
    enum class Kind { primary, derived } kind = Kind::primary;
    std::string distribution;             // optional (D9-D)
};

struct Quantity {                         // D9, D13, D14, D17
    double value;                         // SI (D14: convert at the boundary)
    std::string unit_as_read;             // UCUM, kept for round-trip export
    std::optional<Uncertainty> uncertainty;   // missing != zero (D9)
    std::string quality, origin, calculation_path, source;   // D17, D26; empty = inherit (D48)
};

struct Segment { double t_min, t_max; std::map<std::string, Quantity> parameters; };   // see O-new-2

struct MethodApplication {                // D13, D45
    std::string method;                   // "HKFGems", "Berman88", ... (D22)
    std::map<std::string, Quantity> parameters;   // symbol-as-key
    std::string parameter_set;            // Tier 3 ref (O20: exclusive with inline parameters)
    Applicability applicability;          // t_min, t_max, p_min, p_max (+ isHard)
    std::vector<Segment> segments;        // piecewise-in-T methods (Cp intervals, phase transitions)
};

struct SubstanceRecord {
    std::string symbol, name, formula, class_, aggregate_state, sourcetdb;
    int charge; std::optional<double> molar_mass;
    ReferenceConditions reference_conditions;            // T, P (D8)
    std::string standard_state_convention;               // D8
    std::map<std::string, Quantity> standard_state_properties;   // G, H, S, Cp, V, ... (D13)
    std::vector<MethodApplication> methods;               // ordered (D13)
    std::string defining_reaction, preferred_property_source;   // D15, D16
    std::string data_source; bool deprecated = false;     // D48, D39
    nlohmann::json extra;                                 // unknown fields preserved for round-trip
    SourceFormat source_format;                           // legacy | v2 (for jsonString()/export)
};
```

`ReactionRecord` is parallel: `stoichiometry` (map symbol → ν, D18), `equation`, `reaction_properties` (logK, ΔG, ΔH,
ΔS, ΔCp, ΔV), `methods`, `level` (D27), `preferred_property_source`. `ElementRecord`: symbol, class, isotope mass,
number, valence, molar mass, standard entropy as `Quantity`, `reference_form` (O36).

`Database` gains project-scoped collections (D64): `methods()`, `parameterSets()`, `dataSources()`, plus
`thermoDataSet()` metadata (name, version, status, sourcetdb list, conventions). Today's `DatabaseConventions`
(`water_triple_point_reference`) moves into ThermoDataSet metadata; the old accessor stays.

### 2.3 Method registry and binding (replaces enums + switches)

Each method implementation registers once:

```cpp
registry.add({
  .id = "HKFGems", .tier = 2, .kind = "reference", .operates_on = "substance",
  .applies_to_classes = {"aqueous_solute"}, .contributes_to = {"G","H","S","Cp","V"},
  .stage = Stage::EoS,                 // ordering, see 2.4
  .needs_solvent = true,
  .parameters = { {"a1","cal/(mol.bar)"}, {"a2","cal/mol"}, {"a3","(cal.K)/(mol.bar)"}, {"a4","(cal.K)/mol"},
                  {"c1","cal/(mol.K)"}, {"c2","(cal.K)/mol"}, {"omega","cal/mol"} },
  .legacy = { .tp_method_ids = {2}, .thrift = MethodGenEoS_Thrift::CTPM_HKF, .coeff_key = "eos_hkf_coeffs" },
  .factory = makeHKFgems });
```

* The **parameter list in the registry** is the single statement of what index *i* means. It drives the
  legacy reader (positional → named), the v2 reader (validation, defaults, units), the writers and the docs.
* **Binding** happens once per substance when the engine is built (not per T,P call): parameters are looked up by
  name, converted from SI to the units the model's maths expects, and packed into the existing
  `ThermoParametersSubstance` vectors. So **model classes do not change in the first phases** — the maths stays
  byte-identical; only how its inputs are filled changes.
* The registry also exports itself as `Method` records (JSON) so `thermohub-data/methods/definitions/` can be
  generated from the code for Tier 2/3, and CI can check that data and code agree (D36 Tier C).
* Unknown method id at load → error with the list of known ids (today: silently ignored in several places).

### 2.4 Executor: how a method list is evaluated

The design summary says the calculator *sums* contributions of several MethodApplications (D45, D61). The current
engine does not only sum: it **chains** — a generic EoS/Cp integration produces properties at (T, Pr), then a
T-correction and a P-correction (volume EoS, gas EoS, Landau) take those properties and return modified ones
(`thermoProperties(T, P, tps)` signatures). Both have to be supported, so each registered method declares a
**stage** and a **mode**:

| Stage | Examples | Mode |
|---|---|---|
| `ReferenceTP` | ConstantProperties | sets G,H,S,Cp,V at Tr,Pr |
| `EoS` / `TIntegration` | CpIntegration (Maier-Kelley/Berman), HKFGems, HKFReaktoro, HP98Sol, Anderson91, AD03, Water EoS | produces properties at T,P (or T,Pr) |
| `PCorrection` | ConMolVol, Berman88 V, Murnaghan HP98, Birch-Murnaghan, Tait HP11, gas EoS (PRSV, CORK, SRK, PR78, STP, CGF), IdealGasVol | transforms |
| `Additive` | HPLandau, BermanLandau, future lattice/electronic splits | adds a contribution |
| `Solvent` | water dielectric (JN91, Sverjensky14, Fernandez97) | auxiliary outputs only |

The executor sorts a record's methods by stage (stable, preserving file order within a stage), applies them, and
records each step's contribution (`after − before`) for the D65 breakdown. The special cases now hidden in the
switches (HKF species with a gas P-model, `CTPM_CON` short-circuit, CST without CPT, water steam convention,
Berman-Brown conversion, fallback to reference properties) become explicit, tested rules in `Executor.cpp`.

This is a **gap in the design documents** to feed back to the schema discussion (new open point, §9):
MethodApplication order/stage semantics must be in the schema (or the Method record), because "sum of
contributions" alone does not describe Cp-integration → volume-EoS chaining.

### 2.5 Engine API

* `ThermoEngine` public signatures unchanged.
* New: `thermoPropertiesSubstanceDetailed(T, P, symbol)` → `{total, by_method, auxiliary}` (D65); Python mirrors it.
* New: `engine.snapshot()` → `{schema_version, dataset, dataset_version, content_hash}` emitted by
  `ThermoBatch`/CSV output headers (D33). Hash = SHA-256 of the canonical (sorted-keys, SI-normalised) JSON of
  the records used (O67).
* Property-source policy (D15 modes 1–5, D16, O46): `EnginePreferences.property_source = {record_default,
  prefer_direct, prefer_reaction}`; today's behaviour (`reacDCthermoProperties` when the substance is defined by a
  reaction) is `record_default`.
* Bound violations keep today's behaviour (warning, `isHard` → error), per O32.

---

## 3. Formats and the compatibility layer

### 3.1 Detection

Per file: v2 if it has a `schema_version`/`thermodataset` record in v2 form or any record with
`standard_state_properties` / `methods` / `stoichiometry`; legacy if it has `TPMethods`, `sm_*`, `_label`,
`properties` wrappers or `thermodataset` as a string. Per record (for `appendData(vector<string>)` and
`addSubstance(json)`), the same test. Mixed databases (legacy file + v2 file appended) are allowed, since both end
in the same model. `Database(path, Format::auto | legacy | v2)` lets a user force it.

A directory argument (`Database("thermohub-data/datasets/cemdata18/v2.1.0")`) goes through `DatasetLoader`, which
also loads the project-scoped `methods/`, `parameter_sets/` and `bibliography/` (D64) found up the tree or given
explicitly.

### 3.2 v2 reading details

* **JSON in the core build** (nlohmann, already a dependency). **YAML optional** (`TFUN_USE_YAML`, yaml-cpp or
  rapidyaml): the YAML is converted to a `nlohmann::json` tree and parsed by the same code, so there is one parser.
  The Python package always reads YAML (PyYAML) and passes JSON strings to C++ — so Python users get YAML
  without the C++ dependency. WASM builds stay JSON-only.
* `Normalize` implements the value-or-object union once (O39, O41): bare number → `{value}`; `uncertainty: x` →
  `{standard: x, k: 1, kind: primary}`; expanded U with k → u = U/k; `unit` → SI via UCUM; °C/°F → K (affine,
  D14); `cal` = thermochemical (D14). Round-trip: `V2Writer` emits the shortest form that normalises back to the
  same model (bare number when there is no metadata and the unit is the default).
* Fallback chain (D48): `data_source`, `quality`, `origin`, uncertainty defaults resolved per value → record →
  ThermoDataSet at load; the model keeps both the as-written and the effective value (the writer emits as-written).
* Symbol resolution (D47): per loaded ThermoDataSet, every reference (`method`, `parameter_set`,
  `defining_reaction`, stoichiometry keys, element symbols in formulas, `data_source`) is checked after load;
  `Database::validate()` returns a list of issues instead of throwing on the first one. Cycle detection for
  reaction-defined substances (D15).
* Composite methods (`kind: composite`, ports, bindings — D7) are parsed and stored; using one in a calculation
  raises "composite methods are not executed in this version" (D61 gating).

### 3.3 Legacy reading details

`LegacyReader` is today's parser moved and re-targeted:

* `sm_gibbs_energy {values, errors, units}` → `standard_state_properties.G` Quantity (unit from `units[0]`, SI).
  Legacy `errors` are read as standard uncertainty **with `kind: primary` and a `convention: legacy_unspecified`
  note** (D9-B is exactly this ambiguity); a `LegacyOptions.error_coverage_factor` (default 1, i.e. today's
  behaviour) lets a dataset owner state otherwise.
* `TPMethods[i].method {"<id>": "<name>"}` + coefficient key → `MethodApplication` via `LegacyMethodMap` (§4),
  positional values → named parameters, each with its own legacy unit.
* Several `cp_ft_equation` entries (one per T interval with `limitsTP`) → one `CpIntegration` application with
  `segments`; `m_phase_trans_props` rows → `segments`/transition parameters of that application.
* `m_expansivity`, `m_compressibility` (record level) → parameters of the `ConMolVol`/volume application.
* The existing data fix-ups (HKF `(cal*K)/mol` units, `kbar` expansivity, `1e-05/K` compressibility) stay, but
  move into a named, documented table `LegacyQuirks` so the migration tool reports where they fired.
* `class_ {"3":"SC_AQSOLVENT"}`, `aggregate_state {"4":"AS_AQUEOUS"}` → v2 vocabulary strings (O30/O43:
  `water_solvent`, `aqueous_solute`, `gas`, `mineral`, …; table in `Vocabulary.h`).
* `references` / `datasources` strings → `data_source`; strings already in NEA form (`[2017MIR/WAG]`) are
  normalised to `2017MIR/WAG` (D26); others kept verbatim in `legacy_reference` for the migration report.
* `_key/_id/_rev/_label/properties` wrapper (ArangoDB export) → dropped from the model (D28/D35), `_label` used
  only to pick the record type.
* `jsonString()` keeps returning the original record text (byte-for-byte, as today).

### 3.4 Writers

* `V2Writer` (JSON; YAML when built with YAML) — used by the migration tool, by `Database::save(path, Format::v2)`,
  and by round-trip tests.
* `LegacyWriter` — so GEMS export (`tests/gems_export`) and older tools can still get legacy JSON from a v2
  database. Only what the legacy format can express is written; the rest is reported.

### 3.5 Old C++/Python API as a view

| Old call | Becomes |
|---|---|
| `Substance::methodGenEOS()/method_T()/method_P()` | derived from `methods` via registry `legacy.thrift` |
| `Substance::thermoParameters()` | the bound/packed vectors (same as today) |
| `Substance::thermoReferenceProperties()` | built from `standard_state_properties` |
| `lowerT()/upperT()/lowerP()/upperP()` | envelope of method applicabilities |
| `referenceT()/referenceP()` | `reference_conditions` |
| `reactionSymbol()` | `defining_reaction` |
| setters (`setMethodGenEoS`, `setThermoParameters`, …) | translate into the method list (kept working for users who build records in code) |
| `Reaction::reactants()` | `stoichiometry` |

All of these remain undeprecated until v2 is the default format (Phase 6), then `[[deprecated]]` with a pointer
to the new accessor; removal not before 2.0.

---

## 4. Method id mapping (legacy → v2)

Following D22 (family-first PascalCase, class suffix only when needed) and D20 tiers. Proposed ids; to be confirmed
when the Method records are written in `thermohub-data/methods/definitions/`.

Substances (`enum_method_substance` id → v2 id, tier, stage):

| Legacy id / name | Thrift slot | v2 id | Tier | Stage |
|---|---|---|---|---|
| 0 cp_ft_equation | CTPM_CPT | `CpIntegration` (Maier-Kelley/Berman/HP Cp polynomial) | 1 | TIntegration |
| 1 cp_ft_equation_saxena86 | CTM_FEI | `CpSaxena86` | 1 | TIntegration |
| 43 standard_entropy_cp_integration | CTM_CST | `EntropyCpIntegration` | 1 | TIntegration |
| 2 solute_hkf88_gems | CTPM_HKF | `HKFGems` | 2 | EoS |
| 3 solute_hkf88_reaktoro | CTPM_HKFR | `HKFReaktoro` | 2 | EoS |
| 4 solute_aknifiev_diamond03 | CPM_AKI | `AkinfievDiamond03` | 2 | EoS |
| 41 solute_holland_powell98 | CTPM_HP98 | `HP98Sol` | 2 | EoS |
| 42 solute_anderson91 | CTPM_AN91 | `Anderson91` | 1 | EoS |
| 5 landau_holland_powell98 | CTM_CHP | `HP98Landau` | 2 | Additive |
| 6 landau_berman88 | CTM_BER | `Berman88Landau` | 2 | Additive |
| 34 mv_constant | CPM_CON | `ConMolVol` | 1 | PCorrection |
| 35 mv_equation_dorogokupets88 | CPM_VKE | `Dorogokupets88Vol` | 1 | PCorrection |
| 36 mv_equation_berman88 | CPM_VBE | `Berman88Vol` | 1 | PCorrection |
| 37 mv_eos_birch_murnaghan_gott97 | CPM_VBM | `BirchMurnaghanGott97` | 2 | PCorrection |
| 38 mv_eos_murnaghan_hp98 | CPM_CEH | `HP98Min` (Murnaghan) | 2 | PCorrection |
| 39 mv_eos_tait_hp11 | CPM_VTA | `HP11Tait` | 2 | PCorrection |
| 40 mv_pvnrt | CPM_OFF/IdealGas | `IdealGasVol` | 1 | PCorrection |
| 8 fug_critical_param | CPM_GAS | `GasCritical` | 2 | PCorrection |
| 9 fluid_prsv | CPM_PRSV | `PRSV` | 2 | PCorrection |
| 10 fluid_churakov_gottschalk | CPM_EMP | `CGF` | 2 | PCorrection |
| 11 fluid_soave_redlich_kwong | CPM_SRK | `SRK` | 2 | PCorrection |
| 12 fluid_sterner_pitzer | CPM_STP | `SternerPitzer` | 2 | PCorrection |
| 13 fluid_peng_robinson78 | CPM_PR78 | `PR78` | 2 | PCorrection |
| 14 fluid_comp_redlich_kwong_hp91 | CPM_CORK | `CORK` | 2 | PCorrection |
| 29 water_eos_hgk84_lvs83_gems | CTM_WAT | `WaterHGKGems` | 3 | EoS |
| 30 water_eos_iapws95_gems | CTM_WAS | `WaterIAPWS95Gems` | 3 | EoS |
| 31 water_eos_hgk84_reaktoro | CTM_WAR | `WaterHGK` | 3 | EoS |
| 32 water_eos_iapws95_reaktoro | CTM_WWP | `WaterWP95` | 3 | EoS |
| 33 water_pvt_zhang_duan05 | CTM_WZD | `WaterZhangDuan05` | 3 | EoS |
| 25 water_diel_jnort91_reaktoro | CTPM_WJNR | `JN91Reaktoro` | 3 | Solvent |
| 26 water_diel_jnort91_gems | CTPM_WJNG | `JN91Gems` | 3 | Solvent |
| 27 water_diel_sverj14 | CTPM_WSV14 | `Sverjensky14Diel` | 3 | Solvent |
| 28 water_diel_fern97 | CTPM_WF97 | `Fernandez97Diel` | 3 | Solvent |
| (implicit) ideal-gas water | — | `WaterIdealGasWoolley` | 3 | EoS |
| (no method, props only) | CTPM_CON | `ConstantProperties` | 1 | ReferenceTP |

Reactions (`enum_method_reaction`):

| Legacy id / name | v2 id | Tier |
|---|---|---|
| 0 logk_fpt_function | `LogKAnalytic` (a0…a6) | 1 |
| 3 logk_nordstrom_munoz88 | `LogKNordstromMunoz88` | 1 |
| 4–7 logk_1/2/3_term_extrap* | `LogKExtrap1T0`, `LogKExtrap1T1`, `LogKExtrap2T`, `LogKExtrap3T` (or `VantHoff`/`ConstLogK` aliases) | 1 |
| 8 logk_lagrange_interp | `LogKLagrange` (stored but not implemented today — keep error) | 2 |
| 9 logk_marshall_frank78 | `FrantzMarshall` | 2 |
| 10 solute_eos_ryzhenko_gems | `RyzhenkoBryzgalin` | 2 |
| 11 dr_heat_capacity_ft | `DrCpIntegration` | 1 |
| 12 dr_volume_fpt | `DrVolumeFT` | 1 |
| 13 dr_volume_constant | `DrVolumeConst` | 1 |
| 14 logk_dolejs_manning10 | `DolejsManning10` | 2 |

Positional → named parameters (examples; the full list is produced from the registry in Phase 2):

| Legacy key | Units (today's `units_to`) | v2 parameter names |
|---|---|---|
| `eos_hkf_coeffs[0..6]` | cal/(mol·bar), cal/mol, cal·K/(mol·bar), cal·K/mol, cal/(mol·K), cal·K/mol, cal/mol | `a1 a2 a3 a4 c1 c2 omega` |
| `m_heat_capacity_ft_coeffs[0..10]` | J/(mol·K), J/(mol·K²), J·K/mol, J/(mol·K^0.5), … | `a0 … a10` per segment |
| `m_phase_trans_props[0..4]` | K, J/(mol·K), J/mol, J/bar, K/bar | `T_tr dS_tr dH_tr dV_tr dTdP_tr` per transition |
| `m_landau_phase_trans_props[0..2]` | °C, J/(mol·K), J/bar | `Tc_ref Smax Vmax` |
| `eos_akinfiev_diamond_coeffs[0..2]` | 1, cm³/g, cm³·K^0.5/g | `xi a b` |
| `eos_gas_crit_props[0..3]` | K, bar, 1, 1 | `Tc Pc omega_acentric k1` |
| `m_volume_fpt_coeffs[0..4]` | 1/K, 1/K², 1/K³, 1/bar, 1/bar² | `a1 a2 a3 b1 b2` |
| `logk_ft_coeffs[0..6]` | 1, 1/K, K, 1, K², 1/K², K^0.5 | `a0 … a6` |
| `dr_dolejs_manning10_coeffs[0..4]` | kJ/mol, J/(mol·K), …, | `dH dS dCp b dV` (names to confirm against the paper) |

Parameter names must be confirmed against each model's `.cpp` and the original papers in Phase 2 (each method
revisited individually, D20); the table above is the starting point.

---

## 5. Phased plan

Each phase ends with a release that passes the golden suite. Rough effort is for one person familiar with the code.

### Phase 0 — Safety net (≈1–2 weeks)

* **Golden references.** A script (`tests/golden/make_refs.py`) loads every database in `Resources/databases/*`
  and every `pytests/*.json`, evaluates every substance and reaction on a T–P grid (e.g. 273–1273 K × 1 bar,
  saturation, 1–5 kbar) for all properties, values **and** `ddt`, `ddp`, `err`, `sta`, and stores compressed CSV
  under `tests/golden/`. A pytest compares the current build against them (rel. 1e-10, abs 1e-9 for values near
  zero; status must match exactly).
* Inventory test: list every legacy JSON key present in the resource databases vs. keys the parser reads; any key
  never read is reported (these are what migration must not silently drop).
* CI matrix stays as today (Linux/macOS/Windows, autodiff ON/OFF, WASM).

Acceptance: golden suite green on `master`; key inventory committed.

### Phase 1 — Internal model + legacy reader (≈3–4 weeks) → release 0.8

* Add `Model/` types, `Vocabulary`, `Units/Ucum` front end (accept UCUM `.`, `/`, `^`/exponent digits, `[degF]`,
  `Cel`, `cal_IT`; map to the existing converter).
* Move `ParseJsonToData.cpp` → `IO/Legacy/LegacyReader.cpp`; it now fills `SubstanceRecord` etc.
* Add `LegacyMethodMap` (from the tables in §4) and a temporary binder that packs named parameters back into
  `ThermoParametersSubstance`/`…Reaction` vectors and sets the three Thrift enums — so the engine runs unchanged.
* `Substance`/`Reaction`/`Element` hold a record + the bound legacy view; old getters/setters unchanged.
* Forwarding headers for moved files; `ThermoFun.pri` and CMake source lists updated.

Acceptance: golden suite identical; `installed_headers` test green; no public API change (diff of headers is
additions only).

### Phase 2 — Method registry, binding, executor (≈4–6 weeks) → release 0.9

* `MethodRegistry` with all methods of §4, each declaring parameters (names, units), stage, classes, legacy ids.
* `Binding` replaces the temporary packer: SI → model units, defaults, missing-parameter errors with names.
* `Executor` replaces the switch blocks in `ThermoEngine.cpp`; special cases moved into explicit rules with their
  own tests (HKF + gas P-model, CON short-circuit, CST without CPT, steam convention, Berman–Brown, fallback).
* Multiple applications per record allowed (D45/D61): e.g. `HP11Tait` + `HP98Landau` + `CpIntegration`.
* `thermoPropertiesSubstanceDetailed` (D65) built from the executor's per-step record.
* Export the registry as v2 `Method` JSON (`thermofun --dump-methods`) for `thermohub-data/methods/definitions/`.

Acceptance: golden suite identical; `ThermoEngine.cpp` free of method `switch`es; adding a method = one
registration + one class.

### Phase 3 — v2 reader/writer and dataset loader (≈4–5 weeks) → release 0.10

Can start in parallel with Phase 2 once Phase 1 is in (it only needs the model).

* Vendor a pinned `thermohub-schema` release (`schemas/thermohub/<version>/` with the generated JSON Schema and
  example records, via CMake FetchContent or a git submodule — O81) and record the version in the build.
* `V2Reader` (JSON core; YAML optional), `Normalize` (O39/O41), fallback chain (D48), symbol resolution and
  `Database::validate()` (D47), reaction cycle detection.
* `V2Writer` (shortest round-trippable form) and `LegacyWriter`.
* `DatasetLoader` for the D64 layout; ThermoDataSet metadata (`version`, `status`, `sourcetdb` list, conventions).
* Uncertainty: k-conversion at import, `kind` kept; derived uncertainties are only produced at calculation time.
* Per-value metadata (quality, origin, source) kept in the model and exposed read-only in Python
  (`substance.standardStateProperty("G").source`).
* Tests:
  - **Round-trip equivalence:** for each legacy resource database: legacy → model → v2 JSON → model → engine must
    equal the golden results. This proves the migration mapping numerically, not just structurally.
  - v2 → v2 round-trip is textually stable (writer idempotent).
  - All `thermohub-schema/examples/*` load; invalid examples give the expected `validate()` issues.
  - v2 output of the converter validates against the vendored JSON Schema (python `jsonschema` in CI).

Acceptance: ThermoFun can compute from v2 files with results identical to the legacy files they came from.

### Phase 4 — Migration tooling handed to `thermohub-tools` (≈1–2 weeks, overlaps Phase 3)

* Python entry point in the `thermofun` package: `python -m thermofun.convert in.json out_dir/ --to v2
  [--split-records] [--yaml]`, implemented as legacy reader → V2Writer (rule 3). Writes the D64 layout
  (`substances/<symbol>.yaml`, `reactions/...`, `thermodataset.yaml`) plus a **migration report**: quirks applied,
  unresolved references, unmapped keys, legacy error convention flags.
* `thermohub-tools` wraps this for the one-time bulk ArangoDB → text migration (D34-1). Per-source adapters
  (PHREEQC, SUPCRT, ThermoML) stay in `thermohub-tools` and target the v2 JSON, not ThermoFun.
* Convert all files in `Resources/databases` and `pytests` and commit the v2 versions next to the legacy ones.

### Phase 5 — Shared constants and Tier 1 (≈4–8 weeks, can be split)

* **Constants into MethodParameterSets (O84).** For water EoS (HGK, WP95, ZhangDuan), dielectric models (JN91,
  Sverjensky14, Fernandez97), JN g-function and HKF universal constants (η, Θ, Ψ, …): move the C++ constants into
  a struct filled from a `MethodParameterSet`; the current values are shipped as built-in defaults
  (`<id>_constants_v1`) and exported as records. Behaviour unchanged unless a dataset points to another set.
* **Tier 1 symbolic (D4, D56, D59, D63)** behind `TFUN_USE_SYMENGINE` (OFF by default at first; conda-forge has
  `symengine`; not in WASM builds):
  - parse the Method `expression` with SymEngine once at binding;
  - evaluate with a small tree-walking evaluator templated on the scalar type, so it works with `autodiff::real`
    (D57: runtime goes through autodiff) and with `PlainReal` when autodiff is OFF;
  - run each Method record's reference test cases (D63) in `Database::validate()` and in CI;
  - first targets: `CpIntegration`, `LogKAnalytic`, `ConMolVol`, `DrVolumeFT` written both as code (existing) and
    as expressions — the golden suite must agree within 1e-12 before the expression form is allowed to replace
    the coded one;
  - without SymEngine, a Tier-1 method that has a coded implementation runs the code; one that exists only as an
    expression raises a clear error.

### Phase 6 — v2 by default, documentation, deprecations (≈2 weeks) → release 1.0

* Examples, README, `docs/GUIDE.md`, `docs/python.md` written against v2 files; a "migrating from the legacy
  format" page with the field table of §3 and §4.
* `[[deprecated]]` on the Thrift-enum and positional-vector API; Python `DeprecationWarning`s.
* Legacy reader remains supported through 1.x (bug fixes, no new features); removal discussed for 2.0 only.

### Later / gated (not scheduled)

* Composite method executor (D7) — only after two production refactors need it (D61).
* Monte-Carlo propagation (D10) and covariance (D9-E) — when data with covariance exists.
* C++ JSON-Schema validation in-library (e.g. `json-schema-validator`) — only if Python/CI validation proves
  insufficient.

---

## 6. Python package

* Bindings follow the C++ changes; new: `Database(path_or_dir, format="auto")`, `db.validate()`,
  `db.save(path, format="v2")`, `engine.thermoPropertiesSubstanceDetailed(...)`, `engine.snapshot()`, read-only
  access to Quantities and their metadata.
* Pure-Python helpers in `thermofun/io/`: YAML loading (PyYAML) → JSON strings for C++; `convert` CLI.
* Optional: depend on the Pydantic models generated by `thermohub-schema` (`pip install thermohub-schema`) for
  users who want to build records in Python (O33 builder API); ThermoFun itself does not require them.

---

## 7. Testing strategy (summary)

| Test | Guards |
|---|---|
| Golden T–P grid (values, ddt, ddp, err, sta) for all resource DBs | numbers never change during refactoring |
| Legacy → v2 → engine equals golden | migration mapping is numerically correct |
| Writer idempotence, v2 ↔ v2 | stable files, clean diffs in `thermohub-data` |
| Schema examples load / invalid ones are reported | conformance with `thermohub-schema` |
| Executor special-case unit tests | rules moved out of the switches |
| Registry ↔ Method records consistency | code and data agree (D36 Tier C) |
| Tier‑1 reference cases (D63) | symbolic methods' maths |
| Existing pytests, gtests, autodiff, memoization, gems_export, installed_headers | no regression elsewhere |
| Benchmarks (engine build time, per-call time) | binding at load keeps the hot loop free of string lookups |

---

## 8. Dependencies and build

| Dependency | Use | Required? |
|---|---|---|
| nlohmann_json, ChemicalFun, spdlog, autodiff | as today | as today |
| yaml-cpp or rapidyaml | YAML in C++ | optional (`TFUN_USE_YAML`) |
| SymEngine | Tier‑1 expressions | optional (`TFUN_USE_SYMENGINE`) |
| PyYAML | YAML in Python | Python package only |
| `thermohub-schema` release | JSON Schema + examples for tests | test-time only, pinned |

No UCUM library is added (O44): the existing converter plus a UCUM syntax front end covers the units used in
thermodynamic data; revisit if unit coverage becomes a problem.

---

## 9. Points to feed back into the schema discussion

ThermoFun's code shows a few things the design documents do not settle yet:

1. **Method order / stage semantics (new).** The executor chains T-integration → P-correction → additive terms;
   it is not a plain sum (§2.4). The Method record should declare its `stage` (or the MethodApplication list order
   should be normative). Without it, a v2 file is ambiguous for minerals with Cp + volume EoS + Landau.
2. **Piecewise methods (new).** Several Cp intervals and phase transitions on one substance are *segments* of one
   method, not additive applications. Proposal: `segments` inside a MethodApplication (each with `t_min/t_max`
   and its parameters).
3. **Solvent coupling.** HKF/AD/HP98 solutes need the solvent's properties; until D7 ports are executed, the
   Method record could declare `requires_solvent: true` and the ThermoDataSet name the solvent symbol (today
   `ThermoEngine::setSolventSymbol`).
4. **Legacy error convention.** Migrated uncertainties should carry a flag that the original convention is
   unknown, so they are not mistaken for checked k=1 values (D9).
5. **Record-level parameters** (`m_expansivity`, `m_compressibility`) belong to a method's parameters in v2.
6. **Internal vs SI units.** v2 normalises to SI at the boundary (D14); models compute in their own units (bar,
   J/bar, cal). Binding converts SI → model units, so the models do not need to change; the SI rule is about the
   data model, not the maths.
7. O20 (parameters vs parameter_set exclusive): ThermoFun would prefer "inline parameters override entries of the
   referenced set" — useful for water models with one modified constant. Worth deciding before Phase 5.

---

## 10. Risks

| Risk | Mitigation |
|---|---|
| Hidden behaviour in the switches lost in the executor | Phase 0 golden suite with derivatives and statuses; special-case tests written before moving code |
| Parameter names wrong for a method | names come from the registry, checked by legacy → v2 → engine equality on all resource DBs |
| Schema still moving while ThermoFun implements v2 | v2 reader isolated in `IO/V2`; pin a schema tag; tolerate unknown fields (kept in `extra`) |
| Performance regression from string maps | bind once per substance; executor uses pre-resolved function pointers and vectors |
| Optional dependencies fragment builds (YAML, SymEngine, WASM) | JSON + coded methods are the complete feature set; options only add; CI covers ON/OFF |
| Two migration implementations drifting | rule 3: one mapping in C++, used by the converter through Python |
| Users relying on enum API | kept as a view through 1.x; deprecations only at 1.0 |

---

## 11. First concrete steps

1. Phase 0 golden-reference script and test (no library change).
2. `Model/Quantity.h`, `Vocabulary.h`, UCUM front end with unit tests.
3. Move the parser to `IO/Legacy/`, fill records, temporary packer; run golden suite.
4. Write `LegacyMethodMap` from §4 and check every method id/key in the resource databases is covered.
5. Open the feedback issues of §9 in the schema discussion.
