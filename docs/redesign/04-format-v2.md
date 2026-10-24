# 04 — The v2 format as ThermoFun reads and writes it

ThermoHub v2 is being designed in `thermohub-schema` (LinkML → JSON Schema). Until that schema is released,
ThermoFun implements the **"ThermoFun profile"** described here: the subset of v2 that ThermoFun needs, written
from the design decisions (D8–D18, D24–D27, D35, D44–D48, D60, D64) and examples (E7, E11–E13, E21). When the
schema is released, this file is updated to point to it and the differences become reader changes plus tests.
Choices the design documents leave open are marked *(provisional)*. They are implemented as written, every key name
is a constant in `IO/V2/V2Keys.h`, and they will be revised with the ThermoHub v2 schema; renamed keys stay readable
as aliases for one minor version (decision Q16).

## 1. Files and versions

- **JSON** is always supported. **YAML** is supported in Python always, in C++ with `TFUN_USE_YAML=ON` (§02 §5).
- A v2 file **must** have a top-level `thermohub` key with the format version, e.g. `thermohub: "2.0.0-draft.1"`
  *(provisional)*. This is what format detection keys on, and what version checks use:
  - same major, any minor/patch → read; a newer minor than ThermoFun knows → warning `V2-VER-002` once;
  - different major → error `V2-VER-001` ("this file uses ThermoHub format 3.x; this ThermoFun reads 2.x").
- Files written by ThermoFun carry `thermohub: "<the version ThermoFun implements>"` and
  `generator: "ThermoFun <version>"`.

## 2. Single-file bundle (the common case for ThermoFun users)

Like today's single legacy file, one v2 file can hold a whole dataset. Top-level keys:

| Key | Type | Required | Content |
|---|---|---|---|
| `thermohub` | string | yes | format version |
| `generator` | string | no | tool that wrote the file |
| `thermodataset` | mapping | no | ThermoDataSet metadata (§3.7) |
| `units` | mapping | no | default units for the file (§3.3) |
| `include` | list of strings | no | other files to load first, paths relative to this file (§5) *(provisional)* |
| `elements` | map symbol → Element, or list | no | §3.4 |
| `substances` | map symbol → Substance, or list | no | §3.5 |
| `reactions` | map symbol → Reaction, or list | no | §3.6 |
| `methods` | map id → Method, or list | no | §3.8 (project scope, D64) |
| `parameter_sets` | map id → MethodParameterSet, or list | no | §3.9 |
| `data_sources` | map symbol → DataSource, or list | no | §3.10 |

**Map or list.** Authored YAML uses maps keyed by symbol (E7, E21): readable and unique by construction. A list of
records each with `symbol` is also accepted (JSON tools often produce lists). In a map, a `symbol` field inside the
record is optional and, if present, must equal the key (`V2-ID-002` otherwise). Writers emit maps.

Unknown top-level keys: keys starting with `_` are kept silently; others give warning `V2-KEY-001` and are kept in
`ThermoDataSetInfo::extra_json`.

## 3. Records

### 3.1 Quantities (value-or-object union, D9, D13, D17, O39, O41)

Every numeric slot (standard-state properties, method parameters, applicability bounds, molar mass, …) accepts:

| Written | Meaning |
|---|---|
| `-237183` | value in the slot's default unit; no uncertainty; no metadata |
| `{value: -237183}` | same |
| `{value: -237.183, unit: kJ/mol}` | value with explicit unit |
| `{value: -237183, uncertainty: 100}` | u = 100 (k = 1, kind primary) in the value's unit |
| `{value: -237183, uncertainty: {standard: 100}}` | same |
| `{value: -237183, uncertainty: {expanded: 200, coverage_factor: 2}}` | u = 100 |
| `{value: …, uncertainty: {standard: …, distribution: normal, degrees_of_freedom: 12, kind: primary}}` | full form |
| `{value: …, quality: core, origin: calculated, calculation_path: "…", source: 2018LOT/MAT}` | metadata (D17) |
| `"-237.183 kJ/mol"` | string form "number unit" (Cantera style) — accepted when reading, never written (decision Q3) |

Rules (the implementation is `IO/V2/Normalize.cpp`):
1. A JSON number → `Quantity{value}`; `unit_as_written = ""`.
2. A string → split at the first space; the left part must parse as a number (`V2-QTY-001` otherwise), the right
   part is the unit.
3. An object → `value` is required (`V2-QTY-002`); allowed keys: `value, unit, uncertainty, quality, origin,
   calculation_path, source`; other keys → warning `V2-QTY-003`, kept nowhere (Quantities have no `extra_json`).
4. Unit resolution: explicit `unit` > the nearest enclosing `units:` default for the slot's dimension (§3.3) >
   the slot's default unit (SI, or the parameter's declared unit for method parameters, §06).
5. Conversion: `value_SI = convert(value, unit, slot_SI_unit)`; uncertainty is converted as a **difference**
   (scale only, no offset; for °C → K the uncertainty is unchanged).
6. `expanded` without `coverage_factor` → error `V2-UNC-001`. Both `standard` and `expanded` → error `V2-UNC-002`.
   Negative uncertainty → error `V2-UNC-003`.
7. A bare `uncertainty: 0` is kept as zero (a defined zero, D9), not as "missing".

Writer (shortest form that reads back the same):
- no uncertainty, no metadata, default unit → bare number;
- otherwise an object with only the keys that are set; `unit` only if `unit_as_written` was set, and then the
  value is converted back to that unit; `uncertainty` as a bare number when only `standard` is set and
  `coverage_factor_as_written == 1` and kind is primary and no distribution.

### 3.2 Number formatting

Numbers are written with the shortest representation that reads back to the same `double` (nlohmann's default).
No rounding on write.

### 3.3 Scoped default units (decision Q3)

A `units:` mapping may appear at file level, in a record, or in a method application. It sets the default unit per
dimension for all values below it, inner mappings override outer ones (Cantera's rule):

```yaml
units: {energy: cal, pressure: bar, temperature: K, length: cm, quantity: mol}
```

Dimensions and the SI they replace: `energy` (J), `pressure` (Pa), `temperature` (K), `length` (m), `mass` (kg),
`quantity` (mol). A slot's SI unit is rewritten by substituting the defaults, e.g. `J/(mol.K)` with
`{energy: cal}` → `cal/(mol.K)`. This is implemented once in `Normalize` and is optional for authors. The writer emits a `units:` mapping on a
method application only when all its parameters share the same non-SI energy/pressure units and the file gets
shorter; otherwise it writes explicit `unit` keys or SI values.

### 3.4 Element

```yaml
elements:
  O:
    class: element            # §03 §2.4, default "element"
    number: 8
    molar_mass: {value: 15.9994, unit: g/mol}
    standard_entropy: {value: 102.57, unit: J/(mol.K)}   # legacy "entropy" (S0i)
    reference_form: O2(g)     # optional (O36)
    data_sources: [1998SHO/EA]
  Zz:
    class: charge
    standard_entropy: -65.34
```

Fields: `symbol` (key), `name`, `class`, `number`, `isotope_mass`, `valence`, `molar_mass`, `standard_entropy`,
`heat_capacity`, `volume`, `reference_form`, `data_sources`, `sourcetdb`.

### 3.5 Substance

```yaml
substances:
  NaCl@:
    name: NaCl aqueous complex
    formula: NaCl@
    charge: 0
    class: aqueous_solute
    aggregate_state: aqueous
    sourcetdb: slop98
    reference_conditions: {T: 298.15, P: 1.0e5}     # optional; default from thermodataset
    standard_state_properties:
      G: {value: -388735.44, uncertainty: 800}
      H: -96910
      S: {value: 117.152, unit: J/(mol.K)}
      Cp: 30.1248
      V: {value: 24.0, unit: cm3/mol}
    methods:
      - method: HKFReaktoro
        units: {energy: cal, pressure: bar}         # the HKF parameters as published
        parameters: {a1: 0.6997e-1, a2: 0.5462e3, a3: 0.5156e1, a4: -0.2857e5, c1: 0.1153e2, c2: -0.2415e5, omega: -0.0050e5}   # illustrative values
        applicability: {t_max: 1273.15, p_max: 5.0e8}
    data_source: 1997SVE/SHO
```

Fields:

| Field | Type | Notes |
|---|---|---|
| `symbol` | string | key |
| `name`, `formula` | string | `formula` uses ChemicalFun syntax (as today) |
| `charge` | integer | optional; if absent ThermoFun computes it from the formula (as `setSubstancesChargeFromFromula`) |
| `class` | §03 §2.2 | default `other` |
| `aggregate_state` | §03 §2.3 | default `other` |
| `sourcetdb` | string | identity (D24) |
| `molar_mass` | Quantity (kg/mol in the model; bare numbers in g/mol, decision Q6) | optional |
| `standard_state_convention` | string | default from `thermodataset` |
| `reference_conditions` | `{T, P}` Quantities | default from `thermodataset`, else 298.15 K, 1e5 Pa |
| `standard_state_properties` | map `G,H,S,Cp,V` → Quantity | D13 |
| `methods` | list of method applications (§3.5.1) | ordered |
| `defining_reaction` | reaction symbol | D15 |
| `preferred_property_source` | `direct` \| `reaction` | D16 |
| `applicability` | `{t_min, t_max, p_min, p_max}` | record-level range, used when a method application has none *(provisional)* |
| `data_source` | DataSource symbol | record default (D48) |
| `data_sources` | list of symbols | all citations of the record, as legacy `datasources` *(provisional)* |
| `deprecated`, `deprecated_in_favour_of` | bool, symbol | D39 |

#### 3.5.1 Method application

```yaml
# methods can be listed in any order: the engine orders them by their declared effects (06 §8)
- method: CpIntegration           # method id or alias (§06)
  parameters: {a0: 44.6, a1: 0.0378}      # for single-piece methods
  parameter_set: IAPWS95_constants_v1     # Tier 3; inline `parameters` override entries of the set (decision Q7)
  segments:                                # for piecewise methods instead of `parameters`
    - {t_min: 298.15, t_max: 846, parameters: {a0: 44.603, a1: 0.03778, a2: -1.0018e6}}
    - {t_min: 846, t_max: 2000, parameters: {a0: 58.928, a1: 0.01003}}
  transitions:                             # phase transitions inside the range
    - {T: 846, dS: 0.7, dH: 600, dV: 0.0}
  applicability: {t_min: 298.15, t_max: 2000, p_min: 0, p_max: 1.0e9, hard: false}
  units: {...}                             # optional scoped defaults
```

`parameters` and `segments` are exclusive (`V2-MTH-003`). Parameter names must be declared by the method
(`V2-MTH-002` unknown name: error; with a suggestion of the closest declared name).

### 3.6 Reaction

```yaml
reactions:
  NaHCO3@:
    equation: "Na+ + HCO3- = NaHCO3@"
    stoichiometry: {NaHCO3@: 1, Na+: -1, HCO3-: -1}   # D18; optional if equation given
    reference_conditions: {T: 298.15, P: 1.0e5}
    reaction_properties:
      logK: {value: -0.25, uncertainty: 0.05}
      dH: 15000
    methods:
      - method: LogKAnalytic
        parameters: {a0: 1.8528, a2: -606.24}
        applicability: {t_min: 273.15, t_max: 573.15}
      - method: DrVolumeConst
    level: 0
    data_source: 2017MIR/WAG
```

- If both `equation` and `stoichiometry` are present they must agree (`V2-RXN-001`); if only `equation`, it is
  parsed with the existing `Reaction::fromEquation` parser (O47).
- Map form keeps the written order (nlohmann `ordered_json` is used for reading YAML/JSON maps that need order).

### 3.7 ThermoDataSet

```yaml
thermodataset:
  symbol: aq17
  name: Aqueous 2017
  version: 1.0.0
  status: published
  standard_state_convention: IUPAC_1bar
  reference_conditions: {T: 298.15, P: 1.0e5}
  sourcetdb: [slop98, aq17]
  solvent: H2O@                         # (provisional) replaces the search for a SC_AQSOLVENT substance
  data_source: 2017MIR/WAG              # D48 dataset default
  defaults: {quality: recommended}
  conventions:
    water_triple_point_reference: HKF_HGK_1974    # today's DatabaseConventions
```

### 3.8 Method (project scope)

ThermoFun accepts v2 Method records (E10, E11) and checks them against its built-in `MethodSpec` (§06 §6):
for a built-in method, the record's `parameter_definitions` must match names and dimensions (`V2-MTH-010`),
`operates_on`, `contributes_to` and `effects` (06 §8.6) must match (`V2-MTH-011`). A Method that ThermoFun does not implement is kept;
using it is an error at calculation time unless it is a Tier-1 expression and SymEngine is enabled (phase 5).
Tier-1 (symbolic) Method records carry a `source` block written by the author and a generated `expression` block
(06 §10, examples there); `python -m thermofun.methods build` writes the second from the first.
Composite methods (D7) are kept, never executed (D61): `V2-MTH-020` at use.

### 3.9 MethodParameterSet and 3.10 DataSource

As E11 and D26: `symbol`, `method`, `parameters`, `auxiliary`, `reference`; DataSource `symbol` (NEA pattern
`^[0-9]{4}[A-Z]{3}(/[A-Z]{3})?$` checked as a warning, `V2-SRC-001`), `title`, `authors`, `year`, `journal`,
`doi`, `bibtex`.

## 4. Single records (API calls)

`Database::addSubstance(json)`, `appendData(vector<string>, label)` accept v2 records as JSON objects:
the record with `symbol` inside, and either the label argument or a `record_type` key
(`element | substance | reaction | method | parameter_set | data_source`) *(provisional)*. A record is v2 if it has
any of `standard_state_properties`, `methods`, `stoichiometry`, `reaction_properties`, `record_type`, or
`thermohub`; otherwise legacy (§05 §1).

## 5. Directory layout (D64) and `include`

`Database("path/to/thermohub-data/datasets/cemdata18/v2.1.0")` (a directory) loads:

1. the dataset directory: `thermodataset.yaml|json` (required), then every `*.yaml|*.yml|*.json` under
   `substances/`, `reactions/`, `elements/` (each file is a bundle with the matching section, or a single
   record with `record_type`), in sorted path order;
2. project-scoped records: searching upward from the dataset directory for a directory that has `methods/` or
   `bibliography/` (the `thermohub-data` root), loading `methods/definitions/*`, `methods/parameter_sets/*`,
   `bibliography/*`. `DatasetLoaderOptions::project_root` overrides the search.

`include:` in a bundle loads the listed files first (relative to the including file, glob patterns allowed); a file
is loaded once even if included several times; include cycles → `V2-INC-001`.

## 6. Resolution (after all files are read)

Implemented in `IO/Resolve.cpp`, in this order:
1. **Aliases**: `methods[*].method` → canonical id via the registry (§06); unknown → `V2-REF-003`.
2. **Defaults (D48)**: for each Quantity, empty `meta.source/quality/origin` are filled from the record
   (`data_source`), then from `thermodataset` (`data_source`, `defaults`). The as-written state is kept for the
   writer (the writer emits only what was written).
3. **Reference conditions**: record value, else dataset, else 298.15 K / 1e5 Pa.
4. **References**: `defining_reaction` → reaction (`V2-REF-001`), stoichiometry keys → substances (`V2-REF-002`,
   warning), `parameter_set` → set of the same method (`V2-REF-004`), `data_source` → DataSource (`V2-REF-005`,
   warning only, since citations may be kept elsewhere).
5. **Cycles** through defining reactions → `V2-REF-010` with the cycle printed.

## 7. Diagnostic codes

Stable codes so tests and users can refer to them. `E` = error, `W` = warning.

| Code | Sev | Meaning |
|---|---|---|
| V2-VER-001 | E | unsupported major format version |
| V2-VER-002 | W | newer minor version than known |
| V2-KEY-001 | W | unknown key (kept) |
| V2-ID-001 | E | missing or empty symbol |
| V2-ID-002 | E | symbol inside record differs from map key |
| V2-ID-003 | W | duplicate symbol, later record replaces earlier (today's behaviour) |
| V2-QTY-001 | E | string quantity is not "number unit" |
| V2-QTY-002 | E | quantity object without value |
| V2-QTY-003 | W | unknown key in quantity |
| V2-UNIT-001 | E | unit string cannot be parsed |
| V2-UNIT-002 | E | unit has the wrong dimension for the slot |
| V2-UNC-001 | E | `expanded` uncertainty without `coverage_factor` |
| V2-UNC-002 | E | both `standard` and `expanded` given |
| V2-UNC-003 | E | negative uncertainty |
| V2-VOC-001 | W | unknown vocabulary value (class, aggregate_state, …), `other` used |
| V2-MTH-001 | E | unknown method id |
| V2-MTH-002 | E | unknown parameter name for the method |
| V2-MTH-003 | E | both `parameters` and `segments` |
| V2-MTH-004 | E | required parameter missing |
| V2-MTH-005 | E | method not applicable to the record's class (D23) |
| V2-MTH-006 | W | segments overlap or leave gaps |
| V2-MTH-007 | E | two methods in the same slot (phase 2 only; replaced by 012–015 from step S2.10) |
| V2-MTH-008 | E | method known but not implemented (06 §5 `impl = no`) |
| V2-MTH-012 | E | not exactly one base method (06 §8.2) |
| V2-MTH-013 | E | two methods set the same property |
| V2-MTH-014 | W | a method adds to V but no earlier step defines V |
| V2-MTH-015 | W | a transformation is listed after setters/adders (it runs before them) |
| V2-MTH-016 | I | an inline parameter overrides an entry of the referenced parameter set |
| V2-MTH-010 | E | Method record's parameter definitions disagree with the built-in spec |
| V2-MTH-011 | E | Method record's `operates_on`/`contributes_to` disagree with the built-in spec |
| V2-MTH-020 | E | composite method used in a calculation |
| V2-RXN-001 | E | equation and stoichiometry disagree |
| V2-RXN-002 | W | reaction not balanced in elements or charge |
| V2-REF-001 | E | `defining_reaction` not found |
| V2-REF-002 | W | stoichiometry names a substance that is not in the database |
| V2-REF-003 | E | unknown method id or alias |
| V2-REF-004 | E | `parameter_set` not found or belongs to another method |
| V2-REF-005 | W | `data_source` not found |
| V2-REF-010 | E | cycle of defining reactions |
| V2-INC-001 | E | include cycle |
| V2-SYM-001 | E | unknown variable or function in a symbolic expression |
| V2-SYM-002 | E | symbolic expression does not parse |
| V2-SYM-003 | W | `source` changed after `expression` was generated (regenerate) |
| V2-SYM-004 | E | a reference test case of the method fails |
| V2-SYM-005 | E | the expression does not have the dimension of energy per mole |
| V2-SRC-001 | W | DataSource symbol not in NEA pattern |
| LEG-QRK-001…011 | I | a legacy quirk was applied (list in 05 §4) |
| LEG-MTH-001 | E | legacy method id unknown |
| LEG-MTH-002 | W | legacy method has no implementation or is ignored today (ids 1, 6, 30, 39, 41, 42; reaction 8) |
| LEG-KEY-002 | W | coefficient key does not belong to the entry's method (kept, 05 §3.3) |
| LEG-WRT-001 | W | part of a record cannot be written in the legacy format (05 §6) |
| LEG-KEY-001 | W | legacy key not read by ThermoFun (inventory, §05 §2) |

## 8. Canonical writer output

`V2Writer` writes, in this key order: `thermohub`, `generator`, `thermodataset`, `elements`, `substances`,
`reactions`, `methods`, `parameter_sets`, `data_sources`. Records in each section sorted by symbol unless
`WriterOptions::keep_order` (default true: keep load order, so a converted file diffs cleanly against its source).
Inside a record the field order of §3.4–§3.6 tables. Writing JSON is always available; YAML output is produced by
the Python converter (PyYAML, `sort_keys=False`, `allow_unicode=True`) or by C++ with `TFUN_USE_YAML`.
