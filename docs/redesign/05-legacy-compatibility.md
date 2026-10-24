# 05 — Compatibility with the legacy format and the existing API

Three kinds of compatibility are kept:

1. **Files**: every legacy JSON file that loads today loads the same way, with the same results.
2. **API**: every public C++ function and Python binding keeps its signature and behaviour (until 1.0 marks some
   deprecated; nothing is removed before 2.0).
3. **Downstream code**: Reaktoro's `ThermoFunDatabase`, GEMS export, notebooks — see §7.

## 1. Detecting the format

`IO/FormatDetect.cpp`:

```cpp
enum class DataFormat { Auto, Legacy, V2 };

/// Format of a whole parsed document (top-level object or array).
auto detectDocumentFormat(const nlohmann::json& j) -> DataFormat;   // in .cpp only
/// Format of one record object.
auto detectRecordFormat(const nlohmann::json& j) -> DataFormat;
```

Document: `V2` if the top level is an object with key `thermohub`; otherwise `Legacy` (today a top-level object
with `elements/substances/reactions` sections, or an array of records with `_label`).

Record: `V2` if it has any of `standard_state_properties`, `methods`, `stoichiometry`, `reaction_properties`,
`record_type`; `Legacy` if it has any of `TPMethods`, `sm_gibbs_energy`, `sm_enthalpy`, `sm_entropy_abs`,
`sm_heat_capacity_p`, `sm_volume`, `logKr`, `drsm_gibbs_energy`, `reactants`, `_label`, `properties`, `class_`,
`Tst`; if neither (a record with only `symbol`/`formula`), `Legacy` (today's behaviour).

Input string handling stays as in `Database::Impl::fromFile`: the argument is first tried as JSON text, then as a
file name. New: if the file name ends in `.yaml`/`.yml` (or JSON parsing fails and YAML is enabled), it is read as
YAML. A directory → `DatasetLoader` (§04 §5). `DataFormat` can be forced:
`Database(path, DataFormat::Legacy)` (§08).

Error behaviour stays: for legacy input, JSON exceptions are **logged and the load continues** (today's
`catch (json::exception&)` blocks); for v2 input, problems become `Diagnostics`, and the constructor throws only
if there are errors, with all of them in the message (new behaviour for a new format, nothing changes for old
files).

## 2. Inventory of legacy keys in the bundled data

Counted over `Resources/databases/*.json` and `pytests/**/*.json` (5533 substances, 799 reactions, 504 elements).
"Read" = read by today's parser. A step in Phase 0 (S0.2) turns this table into a test that fails when a key
appears that the reader does not handle.

| Record | Key | Count | Read | Maps to (model) |
|---|---|---|---|---|
| substance | `symbol` | 5533 | yes | `symbol` |
| | `name` | 5532 | yes | `name` |
| | `formula` | 5533 | yes | `formula` |
| | `formula_charge` | 5364 | yes | `charge` |
| | `aggregate_state` `{"<code>": "AS_…"}` | 5533 | yes, **code** | `aggregate_state` (§03 §2.3) |
| | `class_` `{"<code>": "SC_…"}` | 5533 | yes, **code** | `class_` (§03 §2.2) |
| | `Tst`, `Pst` | 5532, 5533 | yes | `reference_conditions` |
| | `sm_gibbs_energy`, `sm_enthalpy`, `sm_entropy_abs`, `sm_heat_capacity_p`, `sm_volume` `{values, errors, units}` | ~5400 each | yes | `standard_state_properties` G, H, S, Cp, V |
| | `TPMethods` (list) | 4914 | yes | `methods` (§3) |
| | `m_expansivity`, `m_compressibility` | 389, 393 | yes | parameters `alpha0`, `kappa0` of HP98 methods (§3.3) |
| | `mass_per_mole` | 168 | yes | `molar_mass` |
| | `reaction` | 793 | yes | `defining_reaction` |
| | `datasources` | 5014 | no | `data_sources` |
| | `references` | 11 | no | `data_sources` |
| | `_label` | 3 | yes (type) | — |
| | `limitsTP` (record level) | 0 | yes | `applicability` |
| TPMethods entry | `method` `{"<id>": "<name>"}` | 7533 | yes, **id** | `MethodApplication.method` via §06 table |
| | `eos_hkf_coeffs` | 2879 | yes | HKF parameters |
| | `m_heat_capacity_ft_coeffs` | 2461 | yes | Cp segment parameters |
| | `limitsTP` | 2461 | yes (cp_ft only) | segment `t_min`/`t_max` |
| | `m_phase_trans_props` | 561 | yes | `transitions` |
| | `m_landau_phase_trans_props` | 60 | yes | Landau parameters |
| | `eos_gas_crit_props` | 43 | yes | gas EoS parameters |
| | `eos_akinfiev_diamond_coeffs` | 9 | yes | AD parameters |
| | `eos_birch_murnaghan_coeffs`, `eos_churakov_gottschalk_coeffs`, `m_volume_fpt_coeffs`, `solute_holland_powell98_coeff` | 0 | yes | §06 tables |
| reaction | `symbol`, `equation`, `reactants` `[{symbol, coefficient}]` | 799 | yes | `symbol`, `equation`, `stoichiometry` |
| | `Tst`, `Pst` | 633 | yes | `reference_conditions` |
| | `logKr`, `drsm_*` | 660–790 | yes | `reaction_properties` |
| | `limitsTP` (record level) | 703 | yes | `applicability` |
| | `TPMethods` | 797 | yes | `methods` |
| | `datasources`, `references` | 736, 8 | no | `data_sources` |
| reaction TPMethods entry | `method` | 1801 | yes, **id** | §06 reaction table |
| | `logk_ft_coeffs` | 790 | yes | LogK parameters |
| | `dr_heat_capacity_ft_coeffs` | 54 | yes | ΔCp parameters |
| | `dr_ryzhenko_coeffs` | 6 | yes | RB parameters |
| | `limitsTP` | 501 | **no** | kept in `MethodApplication.applicability` (not used by the engine, as today) |
| element | `symbol`, `class_`, `entropy {values, name:"S0i"}`, `atomic_mass` | 504 | yes | §03 §5.1 |
| | `datasources`, `references` | 496, 8 | no | `data_sources` |
| file | `thermodataset` (string), `datasources` (list), `date` | — | no | `ThermoDataSetInfo.symbol`, `legacy_datasources`, `legacy_date` |
| | `conventions` | — | yes | `ThermoDataSetInfo.water_triple_point_reference` |

Methods used in the bundled data (by legacy id): 3 `solute_hkf88_reaktoro` 2866, 0 `cp_ft_equation` 2461,
34 `mv_constant` 1461, 38 `mv_eos_murnaghan_hp98` 381, 40 `mv_pvnrt` 114, 43 `standard_entropy_cp_integration`
86, 5 `landau_holland_powell98` 60, 9 `fluid_prsv` 31, 25 `water_diel_jnort91_reaktoro` 16,
32 `water_eos_iapws95_reaktoro` 13, 14 `fluid_comp_redlich_kwong_hp91` 12, 2 `solute_hkf88_gems` 10,
4 `solute_aknifiev_diamond03` 9, 29 `water_eos_hgk84_lvs83_gems` 5, 26 `water_diel_jnort91_gems` 3,
31 `water_eos_hgk84_reaktoro` 2, 8 `fug_critical_param` 1. Reactions: 7 `logk_3_term_extrap` 734,
13 `dr_volume_constant` 503, 0 `logk_fpt_function` 502, 3 `logk_nordstrom_munoz88` 54,
10 `solute_eos_ryzhenko_gems` 6, 4 `logk_1_term_extrap0` 2.

**All other methods** (Saxena Cp, Berman/Dorogokupets volume, Birch–Murnaghan, Tait, CGF, SRK, PR78,
Sterner–Pitzer, HP98 solute, Anderson91, IAPWS95-gems, Zhang–Duan, Sverjensky14, Fernandez97, Berman Landau;
reactions Frantz–Marshall, Dolejs–Manning, ΔV(T), 1-term-extrap1, 2-term, Lagrange) do **not** occur in the
bundled data: the golden suite needs synthetic fixtures for them (§09 §3).

## 3. Legacy → model mapping (LegacyReader)

`IO/Legacy/LegacyReader.cpp` reproduces `ParseJsonToData.cpp` behaviour exactly, but fills records.

```cpp
struct LegacyReadOptions {
    double error_coverage_factor = 1.0;   // legacy "errors" are read as U with this k; 1 = today (u = error)
};
auto readLegacyElement  (const std::string& text, const LegacyReadOptions& = {}, Diagnostics* = nullptr) -> ElementRecord;
auto readLegacySubstance(const std::string& text, const LegacyReadOptions& = {}, Diagnostics* = nullptr) -> SubstanceRecord;
auto readLegacyReaction (const std::string& text, const LegacyReadOptions& = {}, Diagnostics* = nullptr) -> ReactionRecord;
auto readLegacyDocumentInfo(const std::string& text) -> ThermoDataSetInfo;   // top-level keys
```

### 3.1 Values `{values, errors, units}`

`readValueErrorUnit` / `read_values_units` logic is kept: unit = `units[0]` if present and not null, else the
slot's legacy unit (table §03 §2.1); value = `values[0]`; error = `errors[0]` if present. Then:
- `Quantity.value = convert(value, unit, SI)`; `unit_as_written = units[0]` or "" if absent;
- if an error is present: `Uncertainty{standard = convertDifference(error)/error_coverage_factor,
  coverage_factor_as_written = error_coverage_factor, convention_unknown = true}`; an error of exactly 0 → an
  uncertainty of 0 (as today: `err = 0`), not "missing".
- `values: [null]` → property missing (today: status `notdefined`).

### 3.2 Standard-state properties

`sm_gibbs_energy→G`, `sm_enthalpy→H`, `sm_entropy_abs→S`, `sm_heat_capacity_p→Cp`, `sm_volume→V` (J/bar in the
legacy file unless `units` says otherwise; m³/mol in the model).

### 3.3 TPMethods → method applications

For each entry with a non-null `method` object: `id = stoi(first key)`, `name = first value` (trimmed). The **id
decides** the method, as today (`setTPMethods_old(SubstanceTPMethodType(key))`). If `name.trim()` is not the name
of that id in `enum_method_substance`, emit `LEG-QRK-001` (info) — this happens for two bundled records
(`{"3": "solute_hkf88_gems"}` is read as HKF Reaktoro, which is today's result).

The legacy id is turned into a method id with the table in §06 §5 (`MethodCatalog::fromLegacySubstanceId(id)`).
Then the coefficient key of that method is read into named parameters using the method's `ParameterSpec` list
(position i → parameter i, unit i = `units[i]` or the spec's legacy unit). Rules per method family:

| Legacy | Model |
|---|---|
| `cp_ft_equation` (id 0) entries | **one** `CpIntegration` application; each entry adds a `Segment{t_min = limitsTP.lowerT, t_max = limitsTP.upperT, parameters a0…a10}`; each entry's `m_phase_trans_props` adds one `transitions` row `{T, dS, dH, dV, dTdP}` |
| `eos_hkf_coeffs` (ids 2, 3) | `a1 a2 a3 a4 c1 c2 omega` |
| `m_landau_phase_trans_props` (id 5, 6) | `Tc0` (written in °C → K in the model), `Smax`, `Vmax` |
| `m_expansivity`, `m_compressibility` (record level) | parameters `alpha0` (thermal expansion at 298 K, 1/K) and `kappa0` (bulk modulus at 298 K, kbar → Pa) added to **every** application whose spec declares them (`HP98Min`, `HP98Landau`); the legacy writer writes them once at record level |
| `eos_gas_crit_props` (ids 8–14) | `Tc Pc omega k1 k2 k3 k4` (index 6 to be named from `s_solmod2_.cpp`, §06) |
| `eos_akinfiev_diamond_coeffs` (id 4) | `xi a b` |
| others | §06 §5 tables |

`thermoParamSubst` reads **all** coefficient keys of an entry regardless of its method (e.g. an HKF entry with a
stray `m_heat_capacity_ft_coeffs`). The reader keeps this: keys that do not belong to the entry's method are put
into the application of the method they belong to if the record has one, otherwise kept in the entry's
`extra_json` and reported `LEG-KEY-002` (warning). The legacy view (§5) reproduces today's vectors exactly from
the record, which the Phase-1 equivalence test (§09 §2) proves.

Reactions: same, with `enum_method_reaction` ids, `reactants` → `stoichiometry` (order kept), `logKr`/`drsm_*` →
`reaction_properties`, record `limitsTP` → `applicability`.

Substance with `reaction` and **no** `TPMethods` → `defining_reaction` set and the legacy view's
`thermoCalculationType = REACDC` (today's rule). With `TPMethods` and `reaction` → both set (D15 over-determined).

### 3.4 Elements

`entropy.values[0]` → `standard_entropy` (J/(mol·K), no units key in the data), `atomic_mass` → `molar_mass`
(g/mol → kg/mol), `class_` code → class, `isotope_mass`, `number`, `datasources` → `data_sources`.

## 4. Quirks (`IO/Legacy/LegacyQuirks.cpp`)

Every fix-up of bad legacy data is a named function, applied in one place, logged as an info diagnostic
`LEG-QRK-nnn`, and listed in the migration report (phase 4).

| Code | Condition | Fix (today's behaviour) |
|---|---|---|
| LEG-QRK-001 | method id and name disagree | id wins |
| LEG-QRK-002 | method name has leading/trailing spaces (13 bundled records) | trimmed |
| LEG-QRK-003 | `eos_hkf_coeffs.units[2] == "(cal*K)/mol"` | units replaced by the expected list |
| LEG-QRK-004 | `m_expansivity.units[0] == "kbar"` | unit `1/K`, value × 1e5 |
| LEG-QRK-005 | `m_compressibility.units[0] == "1e-05/K"` | unit `kbar` |
| LEG-QRK-006 | `aggregate_state` missing or empty | `other` (not `gas`) |
| LEG-QRK-007 | record wrapped in `properties` (ArangoDB export) | unwrapped |
| LEG-QRK-008 | `values: [null]` | property missing |
| LEG-QRK-009 | more than 1000 coefficients | error (today's limit) |
| LEG-QRK-010 | coefficient array longer than the method uses (2866 `eos_hkf_coeffs` have 8 values, 7 used; `eos_akinfiev_diamond_coeffs` 7 values, 3 used) | extra values kept in the application's `extra_json` as `_legacy_extra_values`, written back by the legacy writer |
| LEG-QRK-011 | `units` list shorter than `values` (HKF: 6 units for 8 values) or empty strings in `units` | missing/empty units = the method's legacy unit (today's `convert_value_units`) |

**Unit labels of `m_expansivity` / `m_compressibility` (decision Q11).** In all bundled data, `m_expansivity`
carries the unit `kbar` (389 records) and `m_compressibility` the unit `1e-05/K` (393 records). The **values are
under the right names, only the unit labels are swapped**, and `m_compressibility` holds the **bulk modulus**
(inverse of compressibility). Example, Corundum in `aq17`: `m_expansivity` 4.19e-5 → α° = 4.19e-5 1/K;
`m_compressibility` 2520 → κ = 2520 kbar; both are Holland & Powell (1998) values. Quirks 004/005 plus the model
code (`SolidMurnaghanHP98.cpp:18-19`, `SolidHPLandau.cpp:31-32`: ×1e5 on reading, ×1e-5 in the model) already give
the model these physical values. The reader stores them as `alpha0` = `m_expansivity.values[0]` in 1/K and
`kappa0` = `m_compressibility.values[0]` kbar → Pa, as parameters of every `HP98Min` and `HP98Landau` application of
the record; binding converts back to what the model receives (`isobaric_expansivity = alpha0·1e5`,
`isothermal_compresibility = kappa0` in kbar). The golden suite checks that the numbers are unchanged.

## 5. The old API as a view over the record

### 5.1 Approach (phase 1 and 2)

`Substance::Impl` keeps **all its current fields** (the "legacy fields": enums, `ThermoParametersSubstance`,
reference properties, limits) and gains:

```cpp
SubstanceRecord record;          // what the file said
bool record_stale = false;       // a legacy setter changed the legacy fields after loading
```

- Loading: reader → `record` → `LegacyView::apply(record, impl)` fills the legacy fields exactly as the old parser
  did. (Phase 1 proves equality with the old parser for every bundled record.)
- Legacy getters return legacy fields (unchanged code).
- Legacy setters change legacy fields (unchanged code) and set `record_stale = true`.
- New `Substance::record() const -> const SubstanceRecord&` rebuilds `record` from the legacy fields with
  `LegacyView::toRecord(impl, record)` when stale (keeping unknown fields and metadata it cannot rebuild).
- New `Substance::setRecord(SubstanceRecord)` sets the record and re-applies the view.

From phase 2 the engine evaluates the **record** (through binding); the legacy fields are then only used by the
legacy getters and by `LegacyView::toRecord`. `LegacyView` lives in `IO/Legacy/LegacyView.{h,cpp}` and has two
functions, tested to be inverse of each other on all bundled records:

```cpp
auto applyLegacyView(const SubstanceRecord& r, Substance& s) -> void;          // record → legacy fields
auto legacyViewToRecord(const Substance& s, SubstanceRecord base) -> SubstanceRecord; // legacy fields → record
```

Same for `Reaction` and `Element`.

### 5.2 Old calls and where their answer comes from

| Old call | Answer |
|---|---|
| `Substance::methodGenEOS()/method_T()/method_P()` | legacy fields; for v2 records computed from `methods` with the spec's `legacy.thrift_slot` (§06) |
| `Substance::thermoParameters()` | legacy fields (packed vectors); for v2 records packed by `Binding::packLegacy` |
| `Substance::thermoReferenceProperties()` | built from `standard_state_properties` (V back to J/bar) |
| `referenceT()/referenceP()` | `reference_conditions` |
| `lowerT()/upperT()/lowerP()/upperP()` | record `applicability`, else as today |
| `reactionSymbol()` / `thermoCalculationType()` | `defining_reaction` / REACDC rule |
| `substanceClass()`, `aggregateState()` | vocabulary string → legacy enum (§03) |
| `jsonString()` | `original_text` if not stale and legacy; legacy JSON written by `LegacyWriter` otherwise; for v2 records the v2 JSON of the record (decision Q10) |
| `Reaction::reactants()` | `stoichiometry` as map |
| `Element::molarMass()` (g/mol), `entropy()` | from the record, converted to the legacy units |

## 6. Legacy writer (phase 3)

`IO/Legacy/LegacyWriter.{h,cpp}` writes records as legacy JSON: `sm_*` with `{values:[v], errors:[u], units:[legacy
unit]}`, methods as `TPMethods` with `{"<id>": "<name>"}` and the coefficient arrays in the legacy units; segments
back to one `cp_ft_equation` entry per segment; `alpha0`/`kappa0` back to `m_expansivity`/`m_compressibility`. Records
that came from a legacy file and were not edited are written from `original_text`. A v2 record that the legacy
format cannot express (an unknown method, a Tier-1 expression, metadata) is written without that part and a
`LEG-WRT-001` warning names it. Used by `Database::save(path, DataFormat::Legacy)` and by GEMS export.

## 7. Downstream users and the API they need

| User | Uses | Must keep |
|---|---|---|
| Reaktoro `ThermoFunDatabase` (`Reaktoro/Extensions/ThermoFun/ThermoFunDatabase.cpp`) | `ThermoFun::Database(file)`, `mapSubstances()`, `Substance::symbol()/name()/formula()/aggregateState()`, `ThermoFun::AggregateState::*` enum values, `Element` API, `ThermoEngine::thermoPropertiesSubstance` | all of these, same types |
| GEMS3K export (`tests/gems_export`) | `Database`, `Substance`, thermo parameters | as today |
| Python notebooks (thermofun-jupyter) | `thermofun.Database`, `ThermoEngine`, `ThermoBatch`, `appendData`, `getSubstance`, `jsonString` | all Python names |
| ThermoHub client / ThermoMatch | legacy JSON records via `appendData(vector<string>, label)` and `jsonString()` | record round-trip |

Before each release, build Reaktoro's ThermoFun extension against the new ThermoFun (CI job, §09 §6).

## 8. Deprecation timeline (NumPy NEP 23 style)

| Release | Change |
|---|---|
| 0.8–0.11 | everything kept; new API added |
| 1.0 | v2 is the documented format; `[[deprecated]]` (C++) and `DeprecationWarning` (Python) on: `setMethodGenEoS/setMethod_T/setMethod_P`, `methodGenEOS/method_T/method_P`, `setThermoParameters`, `thermoParameters` (positional vectors). Legacy files load without warning |
| 1.x (≥ 2 minor releases later) | loading a legacy file logs one info message pointing to `python -m thermofun.convert` |
| 2.0 earliest | removal of the deprecated calls may be discussed; the legacy **reader** stays as long as legacy files exist in use |
