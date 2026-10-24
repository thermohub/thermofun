# 08 — Public API (C++ and Python)

Rules: nothing existing is removed or changed (§05); new functions follow the existing style (camelCase, trailing
return types, same name in Python, §02 §3). Each new function below says the phase in which it appears.

## 1. Loading data

### C++

```cpp
// Model/DataFormat.h  (phase 1)
enum class DataFormat { Auto, Legacy, V2 };

// Database.h — new overloads and members (existing ones unchanged)
struct DatabaseLoadOptions {                                   // phase 3
    DataFormat format = DataFormat::Auto;
    LegacyReadOptions legacy;                                  // §05 §3
    std::string project_root;                                  // directory layout (§04 §5); "" = search
    bool throw_on_error = true;                                // v2: throw if diagnostics contain errors
};

class Database {
public:
    explicit Database(std::string filename);                   // existing: legacy or v2, file/dir/string (phase 3)
    Database(std::string filename, const DatabaseLoadOptions& options);           // phase 3
    auto appendData(std::string filename, const DatabaseLoadOptions& options) -> void;   // phase 3

    // project-scoped records (phase 3)
    auto methods() const -> const std::map<std::string, MethodDefinition>&;
    auto parameterSets() const -> const std::map<std::string, MethodParameterSet>&;
    auto dataSources() const -> const std::map<std::string, DataSource>&;
    auto thermoDataSet() const -> const ThermoDataSetInfo&;

    // checks and export (phase 3)
    auto validate() const -> Diagnostics;                      // all checks of §03 §7 / §04 §7
    auto loadDiagnostics() const -> const Diagnostics&;        // what was found while loading
    auto save(const std::string& path, DataFormat format = DataFormat::V2) const -> void;   // .json; .yaml with TFUN_USE_YAML
    auto toJson(DataFormat format = DataFormat::V2) const -> std::string;

    auto revision() const -> std::uint64_t;                    // phase 2, increments on every change
};
```

### Python

```python
import thermofun as tf

db = tf.Database("aq17-thermofun.json")                 # legacy file: unchanged
db = tf.Database("aq17.yaml")                           # v2 YAML (Python reads YAML, passes JSON to C++)
db = tf.Database("thermohub-data/datasets/aq17/1.0.0")  # dataset directory
db = tf.Database("file.json", tf.DatabaseLoadOptions(format=tf.DataFormat.Legacy))

issues = db.validate()          # tf.Diagnostics; iterate for tf.Diagnostic (severity, code, record, path, message)
print(issues.toString())
db.save("aq17-v2.json")         # v2 JSON
```

Python YAML support: `Database.__init__` and `appendData` are wrapped in `thermofun/__init__.py`: if the argument is
a path ending in `.yaml`/`.yml` (or a directory containing YAML), the Python layer reads the YAML with PyYAML
(`yaml.safe_load`), converts to JSON text and calls the C++ constructor with it (and with
`include`/directory resolution done in `thermofun/io/yaml_loader.py`). PyYAML becomes a dependency of the Python
package.

## 2. Records

```cpp
// Substance.h (phase 1)
auto record() const -> const SubstanceRecord&;
auto setRecord(SubstanceRecord record) -> void;
// Reaction.h, Element.h: the same with ReactionRecord, ElementRecord

// Convenience on Substance (phase 3)
auto standardStateProperty(const std::string& name) const -> std::optional<Quantity>;   // "G", "H", ...
auto methodIds() const -> std::vector<std::string>;                                    // ["HKFReaktoro"]
```

Python: `Quantity`, `Uncertainty`, `ValueMeta`, `MethodApplication`, `SubstanceRecord`, `ReactionRecord`,
`ElementRecord`, `DataSource`, `MethodDefinition`, `MethodParameterSet`, `ThermoDataSetInfo` bound with
`def_readwrite` for fields (pybind11), plus `__repr__` showing symbol and main fields. Records are copied in and
out (value semantics); `substance.record()` returns a copy, `substance.setRecord(r)` applies it.

## 3. Methods

```cpp
// Methods/MethodCatalog.h (phase 2)
MethodCatalog::instance().specs();          // list of MethodSpec
MethodCatalog::instance().find("solute_hkf88_gems")->id;   // "HKFGems"
```

Python: `tf.methodCatalog()` → list of `MethodSpec` (id, aliases, title, tier, slot, parameters with name,
si_unit, description). Used by the generated documentation (§10) and by users to see parameter names.

## 4. Engine

```cpp
// ThermoEngine.h — new (phase 2)
struct DetailedPropertiesSubstance {
    ThermoPropertiesSubstance total;
    std::vector<std::pair<std::string, ThermoPropertiesSubstance>> steps;   // method id → properties after the step
    std::optional<PropertiesSolvent> solvent;                               // water records
    std::optional<ElectroPropertiesSolvent> solvent_electro;
    std::map<std::string, Reaktoro_::ThermoScalar> auxiliary;               // 06 §12, e.g. "epsilon", "g_function"
};
auto thermoPropertiesSubstanceDetailed(double T, double& P, const std::string& symbol) const -> DetailedPropertiesSubstance;

// provenance (phase 3, D33)
struct CalculationSnapshot {
    std::string thermofun_version;
    std::string format_version;       // ThermoHub format version of the loaded data, "legacy" if legacy
    std::string dataset;              // ThermoDataSetInfo.symbol
    std::string dataset_version;
    std::string content_hash;         // SHA-256 of the canonical v2 JSON of all records (§09 §5)
};
auto snapshot() const -> CalculationSnapshot;
```

`EngineConventions` gains (phase 3, 14 §6) `energy_reference`, `standard_pressure`, `concentration_scale` for the
**output** convention (defaults: canonical); the existing `apparent_properties` and `water_properties` strings stay
and map onto them. Input conventions come from the data (methods, records, dataset), never from the engine.

`EnginePreferences` gains (phase 3) `std::string property_source = "record"` with values `record` (today:
REACDC rule and the record's `preferred_property_source`), `direct`, `reaction` (D15 modes, O46). Default keeps
today's results. Per substance or reaction (decision Q32):

```cpp
auto setPropertySource(const std::string& symbol, const std::string& source) -> void;  // "record" | "direct" | "reaction" | "reactants"
auto propertySource(const std::string& symbol) const -> std::string;
```

`EnginePreferences.method_implementation` (`std::map<std::string, std::string>`, id → `"code"` | `"symbolic"`)
selects the implementation of a method (06 §11).

### Consistency report (decision Q32, D17, D37)

```cpp
struct ConsistencyItem {
    std::string symbol;            // over-determined substance or reaction
    std::string kind;              // "substance_vs_reaction" | "reaction_vs_reactants"
    double T, P;                   // K, Pa
    double delta_G;                // J/mol, direct minus derived
    double delta_logK;             // for reactions
    std::string flag;              // "consistent" | "inconsistent" (threshold: ConsistencyOptions)
};
auto consistencyReport(const std::vector<std::pair<double,double>>& TP, const ConsistencyOptions& = {}) const
    -> std::vector<ConsistencyItem>;
```

Over-determined records (a substance with its own data and a defining reaction; a reaction with its own properties
whose species all have data) are evaluated both ways. Python: `python -m thermofun.consistency DB [--TP ...]`
writes a table; CI of data repositories posts it as information, never as a failure (D37).

### Parameter sensitivities (decision Q32, optional)

```cpp
/// d(property)/d(parameter) for the parameters of one method application of a substance, at T (K), P (Pa)
auto parameterSensitivities(double T, double& P, const std::string& symbol, const std::string& method_id) const
    -> std::map<std::string, ThermoPropertiesSubstance>;   // parameter name -> derivatives of G, H, S, Cp, V
```

Built on the existing parameter-derivative passes (`propagate_parameter_errors`, `uncertainCoefficients`), it lets an
optimiser fit the parameters of one contribution (e.g. the Landau term of a mineral) without touching the others.
Fitting itself stays outside ThermoFun.

Python: same names; `ThermoBatch` CSV output gets an optional header line with the snapshot
(`batch.setOutputSnapshot(True)`, default False so files do not change).

## 4a. Building records in Python (decision Q32)

Construction should be as easy as in Reaktoro or Cantera; the builder writes ordinary v2 records:

```python
from thermofun import records as rec

quartz = (rec.substance("Quartz", formula="SiO2", aggregate_state="crystal", class_="component")
             .standard_state(G=-856288, H=-910700, S=41.46, Cp=44.6, V="22.688 cm3/mol")
             .method("CpIntegration", segments=[dict(t_min=298.15, t_max=844, a0=44.6, a1=0.0378, a2=-1.0e6)])
             .method("HP98Min", alpha0=1.24e-5, kappa0="730 kbar"))
db.addSubstance(quartz.to_json())        # or quartz.record() -> SubstanceRecord
```

Values accept numbers, `"number unit"` strings and `{value, uncertainty, unit}` dicts (same rules as files, 04 §3.1).
`rec.reaction(...)`, `rec.element(...)` likewise. Pure Python (`python/src/thermofun/records.py`), no C++ change.

## 5. Converter and validator (phase 3/4, Python only)

```bash
python -m thermofun.convert INPUT OUTPUT [--to v2|legacy] [--format json|yaml] [--split] [--report report.md]
python -m thermofun.validate INPUT [--schema schemas/thermohub/<ver>/thermohub.schema.json]
```

- `convert`: loads INPUT with the C++ readers, writes OUTPUT with the C++ writers; `--split` writes the directory
  layout (§04 §5) with one file per record; `--report` writes the migration report (quirks applied, unread keys,
  unresolved references, legacy uncertainty convention notice, method ids used).
- `validate`: runs `Database.validate()` and, if `--schema` is given or a pinned schema is installed, JSON Schema
  validation of every v2 file (`jsonschema` package, optional dependency). Exit code 1 if any error.

Python API of the same: `thermofun.convert.convert_file(input, output, to="v2", fmt="yaml", split=False,
report=None) -> Diagnostics`, `thermofun.validate.validate_path(path, schema=None) -> Diagnostics`.

## 5a. Tier-1 method authoring tool (phase 5, Python only, needs SymPy)

```bash
python -m thermofun.methods build methods/definitions/MaierKelleyCp.yaml      # writes the generated `expression`
python -m thermofun.methods check methods/definitions/                       # forms agree, tests pass, hashes current
python -m thermofun.methods new --form cp --name MyCp --params a,b,c > MyCp.yaml   # template to fill in
```

Python API: `thermofun.methods.build_method(path_or_dict) -> dict`, `check_method(...) -> Diagnostics`. SymPy is an
optional dependency (`pip install thermofun[authoring]`); running ThermoFun never needs it.

## 6. What does **not** change

`ThermoEngine` property functions and their units, `ThermoBatch`, `ThermoScalar`, `ThermoPropertiesSubstance`
and friends, `EnginePreferences` existing fields, `EngineConventions`, all `Database` add/set/get/contains
functions, all `Substance`/`Reaction`/`Element` getters and setters, the enums in `GlobalVariables.h` and their
Python bindings.
