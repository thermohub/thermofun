# 02 — Architecture, folders, files and conventions

## 1. Layers

```
                ┌──────────────────────────────────────────────────────────────┐
  user code ──► │ Public API: Database, Substance, Reaction, Element,          │
  (C++/Python)  │ ThermoEngine, ThermoBatch  (signatures kept, §08)            │
                └───────────────┬──────────────────────────────┬───────────────┘
                                │                              │
                 ┌──────────────▼─────────────┐   ┌────────────▼──────────────┐
                 │ IO                          │   │ Engine                     │
                 │  FormatDetect               │   │  SubstanceExecutor          │
                 │  Legacy/  (reader, writer)  │   │  ReactionExecutor           │
                 │  V2/      (reader, writer,  │   │  SolventExecutor            │
                 │            normalize)       │   │  DetailedResult             │
                 │  DatasetLoader, Validate    │   └────────────┬──────────────┘
                 └──────────────┬─────────────┘                │
                                │                 ┌────────────▼──────────────┐
                                │                 │ Methods                    │
                                │                 │  MethodCatalog (specs:     │
                                ├────────────────►│   ids, params, units,      │
                                │   (specs only)  │   effects, legacy aliases) │
                                │                 │  MethodRegistry (factories)│
                                │                 │  Binding                   │
                                │                 └────────────┬──────────────┘
                 ┌──────────────▼─────────────────────────────▼──────────────┐
                 │ Model: Quantity, Uncertainty, SubstanceRecord, ReactionRecord, │
                 │ ElementRecord, MethodApplication, MethodDefinition,         │
                 │ MethodParameterSet, DataSource, ThermoDataSetInfo, Vocabulary│
                 └──────────────┬─────────────────────────────────────────────┘
                 ┌──────────────▼─────────────────────────────────────────────┐
                 │ Common: Units (+ Ucum), Exception, Diagnostics, ThermoScalar,│
                 │ Real, GlobalVariables                                        │
                 └──────────────────────────────────────────────────────────────┘
  Model implementations (unchanged maths): Substances/…, Reactions/…, ThermoModels*.cpp
  are called only from Methods/ (through factories) and, until phase 2 ends, from ThermoEngine.cpp.
```

### Dependency rules (checked in review)

| Layer | May include | Must not include |
|---|---|---|
| `Common/` | standard library, autodiff, spdlog | anything else in ThermoFun |
| `Model/` | `Common/` | nlohmann/json, IO, Methods, Engine, public classes |
| `Methods/MethodCatalog` | `Model/`, `Common/` | model implementation classes, Engine |
| `Methods/MethodRegistry`, `Binding` | catalog, `Model/`, model implementation classes, public `Substance`/`Reaction` | IO, Engine |
| `IO/` | `Model/`, `Methods/MethodCatalog`, `Common/`, nlohmann/json (only in `.cpp` files) | Engine, MethodRegistry (factories) |
| `Engine/` | everything above | IO |
| Public `Database` | IO, Model, Methods | — |
| Public `ThermoEngine` | Engine, Database | — |

**nlohmann/json appears only in `.cpp` files** (as today: `ParseJsonToData.h` does not include it). The
installed CMake config does not `find_package(nlohmann_json)`, so a public header that includes it would break
users. Unknown fields kept for round-trip are stored as a JSON **string** (`std::string extra_json`).

## 2. Folder and file layout

Existing files are **not moved** (Reaktoro, GEMS and other users include them by path; `ThermoFun/CMakeLists.txt`
globs all sources, so new folders are picked up automatically). New code goes into new subfolders of
`ThermoFun/`. `ParseJsonToData.*` stays as a thin forwarding layer to `IO/Legacy/` (§05).

```
ThermoFun/                                   (library sources and installed headers, as today)
│  ThermoFun.h                               umbrella header — add the new public headers to it
│  Database.h/.cpp  Substance.h/.cpp  Reaction.h/.cpp  Element.h/.cpp      public classes (kept; now views, §05)
│  ThermoEngine.h/.cpp                       public engine (kept; dispatch moves to Engine/ in phase 2)
│  ThermoParameters.h ThermoProperties.h GlobalVariables.h …               kept
│
├─ Common/                                   kept; additions:
│    Ucum.h / Ucum.cpp                       NEW  UCUM syntax → existing converter syntax (§07)
│    Diagnostics.h / Diagnostics.cpp         NEW  Diagnostic, Diagnostics (collect-all reporting)
│    ParseJsonToData.h/.cpp                  KEPT forwards to IO/Legacy (phase 1)
│
├─ Model/                                    NEW  format-neutral records (§03)
│    Quantity.h / Quantity.cpp               Uncertainty, ValueMeta, Quantity
│    Vocabulary.h / Vocabulary.cpp           PropertyName, SubstanceClass, AggregateState strings ↔ enums
│    Applicability.h                         Applicability, Segment
│    MethodApplication.h / .cpp              MethodApplication
│    MethodDefinition.h                      MethodDefinition, ParameterDefinition, ReferenceTestCase
│    MethodParameterSet.h                    MethodParameterSet
│    DataSource.h                            DataSource
│    ThermoDataSetInfo.h                     ThermoDataSetInfo (metadata, defaults, conventions)
│    ElementRecord.h  SubstanceRecord.h  ReactionRecord.h
│    Records.h                               includes all of the above (convenience)
│
├─ Methods/                                  NEW  (§06)
│    MethodSpec.h                            MethodSpec, ParameterSpec, Stage, MethodScope, LegacyAlias
│    MethodCatalog.h / .cpp                  the table of all built-in MethodSpecs (data only)
│    MethodRegistry.h / .cpp                 id/alias → spec + factory; registerBuiltinMethods()
│    Binding.h / .cpp                        MethodApplication → BoundMethod (SI → model units, packing)
│    BuiltinSubstanceMethods.cpp             factories wrapping Substances/* classes
│    BuiltinReactionMethods.cpp              factories wrapping Reactions/* classes
│    BuiltinSolventMethods.cpp               factories wrapping Solvent/* classes
│    Symbolic/ (phase 5, optional)           SymbolicExpression.h/.cpp (parse, differentiate), Evaluator.h
│                                            (tree walk on real), SymbolicMethods.cpp (bind Tier-1 records)
│
├─ Engine/                                   NEW  (phase 2, §06)
│    WorkContext.h                           what one evaluation needs (T, P, engine prefs, solvent symbol…)
│    SubstanceExecutor.h / .cpp              replaces ThermoEngine::Impl::thermoPropertiesSubstance switches
│    ReactionExecutor.h / .cpp               replaces thermoPropertiesReaction switches
│    SolventExecutor.h / .cpp                propertiesSolvent, electroPropertiesSolvent
│    DetailedResult.h                        DetailedPropertiesSubstance (total, byMethod, auxiliary)
│    Snapshot.h / .cpp                       content hash and provenance of a calculation (D33)
│
├─ IO/                                       NEW  (§04, §05)
│    Format.h                                enum class DataFormat { Auto, Legacy, V2 }
│    FormatDetect.h / .cpp                   detect file / record format
│    Legacy/LegacyReader.h / .cpp            legacy JSON → records (moved logic of ParseJsonToData.cpp)
│    Legacy/LegacyKeys.h                     every legacy key as a named constant
│    Legacy/LegacyQuirks.h / .cpp            documented fix-ups of bad legacy data
│    Legacy/LegacyWriter.h / .cpp            records → legacy JSON (phase 3)
│    V2/V2Reader.h / .cpp                    v2 JSON → records
│    V2/V2Writer.h / .cpp                    records → v2 JSON (shortest round-trippable form)
│    V2/Normalize.h / .cpp                   value-or-object union, units, defaults (§04 §3)
│    V2/V2Keys.h                             every v2 key as a named constant
│    Yaml.h / Yaml.cpp                       optional (TFUN_USE_YAML): YAML text → JSON text
│    DatasetLoader.h / .cpp                  directory layout (D64) → records
│    Resolve.h / .cpp                        reference resolution, cycles, D48 fallback chain
│    Validate.h / .cpp                       semantic checks → Diagnostics
│
├─ Substances/  Reactions/  Batch/           kept, unchanged (model maths)
│
python/src/thermofun/                        bindings (kept) + NEW files:
│    pyQuantity.cpp  pyRecords.cpp  pyDiagnostics.cpp  pyDetailedResult.cpp
│    io/__init__.py  io/yaml_loader.py       YAML → JSON for C++ (PyYAML)
│    convert.py                              python -m thermofun.convert  (phase 4)
│    validate.py                             python -m thermofun.validate (phase 3)
│    methods.py                              python -m thermofun.methods build|check (phase 5, needs SymPy)
│
tests/
│    golden/            NEW  C++-free golden suite lives in pytests/golden (see below)
│    model/             NEW  C++ unit tests of Model/, Common/Ucum, IO (ctest target `tfun-unit`)
│
pytests/
│    golden/            NEW  make_references.py, test_golden.py, data/*.json.gz
│    io/                NEW  test_legacy_reader.py, test_v2_reader.py, test_roundtrip.py, test_validate.py
│    data/v2/           NEW  hand-written v2 fixtures (§09)
│
docs/                   reorganised in phase 6 (§10)
schemas/thermohub/      NEW  pinned copy of the thermohub-schema release (JSON Schema + examples), phase 3
```

## 3. Naming conventions

The existing code style is kept for new code, so the library reads as one piece.

### C++

| Item | Rule | Example |
|---|---|---|
| Namespace | everything in `ThermoFun`; internal helpers in `ThermoFun::detail` | `ThermoFun::Quantity`, `ThermoFun::detail::packHKF` |
| Classes, structs, enums | PascalCase nouns | `SubstanceRecord`, `MethodRegistry`, `enum class Stage` |
| Enum values (new `enum class`) | PascalCase | `Stage::PCorrection`, `DataFormat::V2` |
| Functions, methods | camelCase verbs, trailing return type | `auto readLegacySubstance(const std::string& text) -> SubstanceRecord;` |
| Getters | noun, no `get` prefix (existing style) | `record.symbol()`, `spec.parameters()` (structs: public fields) |
| Data members of structs | snake_case (match the v2 field names) | `standard_state_properties`, `t_min` |
| Private members of classes | trailing underscore | `specs_`, `aliases_` |
| Constants | `constexpr` PascalCase or the existing `UPPER_CASE` in `GlobalVariables.h` | `constexpr double DefaultReferenceT = 298.15;` |
| Files | named after the main class, PascalCase, `.h` + `.cpp` | `MethodRegistry.h`, `MethodRegistry.cpp` |
| Header guard | `#pragma once` for new files | |
| Method ids (data) | PascalCase, family first (D22) | `HKFGems`, `Berman88Vol`, `LogKAnalytic` |
| Parameter names (data) | as in the source paper, ASCII | `a1`, `omega`, `Tc`, `kappa0` |
| v2 field names (data) | snake_case | `standard_state_properties`, `defining_reaction` |
| Vocabulary values (data) | snake_case | `aqueous_solute`, `water_solvent` |

### Python

The Python module mirrors the C++ names (today's rule: "the naming is the same in C++ and Python"), so the
reference documentation serves both. New functions follow the same camelCase. Pure-Python helper modules
(`thermofun.convert`, `thermofun.validate`, `thermofun.io`) use snake_case functions as normal Python
(`convert_file(...)`), since they have no C++ counterpart. See §12 Q1 for the alternative (snake_case aliases).

### JSON/YAML

Keys are snake_case; record ids (`symbol`) are free text as today (`H2O@`, `Ca+2`, `NaCl@`). The single-file
top-level sections are plural nouns (`elements`, `substances`, `reactions`, `methods`, `parameter_sets`,
`data_sources`) plus `thermodataset` (§04).

## 4. Coding rules

1. **C++20** (as `CMakeLists.txt`). Prefer value types and `std::optional`; no raw `new`.
2. **Public headers** contain declarations and small inline functions only; no `nlohmann`, no `yaml-cpp`, no
   `spdlog` (use the existing `thfun_logger` from `.cpp` files).
3. **Errors**
   - Programming or data errors that make a calculation impossible: the existing `Exception` + `RaiseError(...)`
     (`Common/Exception.h`), message naming the record, the field and what was expected.
   - Problems found while reading/validating data: add a `Diagnostic` to a `Diagnostics` list (§4.1 below) and
     continue. `Database` constructors keep today's behaviour (warn via logger for non-fatal, throw for fatal).
4. **Logging**: `thfun_logger->warn(...)` for things users should see once; never log inside the T,P hot path
   except through the existing "warn once" pattern (`warnIfLogKFunctionDisagrees`).
5. **Comments**: `///` Doxygen comment on every public type and function: one sentence on what it is or does,
   then units of every numeric argument and return value. No comments that narrate the change history.
6. **Units in code**: every `double` holding a physical value has its unit in the variable name or comment
   (`P_Pa`, `/// pressure in Pa`). Model records are SI (§07); model classes keep their own units.
7. **Thread safety**: records and the registry are immutable after load; `ThermoEngine` stays usable from
   several threads as today (memoization caches already handle this).
8. **Performance**: nothing in `Model/`, `IO/`, `Methods/Binding` runs per T,P call. The executor works on
   pre-bound data (vectors, function pointers); no string lookups per call.
9. **SPDX header** on every new file, as in existing files:
   ```cpp
   // SPDX-License-Identifier: LGPL-3.0-or-later
   // Copyright (C) 2026 ThermoFun contributors
   ```

### 4.1 Diagnostics

```cpp
// Common/Diagnostics.h
enum class Severity { Info, Warning, Error };

/// One problem found while reading or validating data.
struct Diagnostic {
    Severity severity = Severity::Error;
    std::string code;     ///< stable id, e.g. "V2-REF-001" (list in 04-format-v2.md §7)
    std::string file;     ///< file name if known
    std::string record;   ///< record symbol if known, e.g. "NaCl@"
    std::string path;     ///< JSON pointer inside the record, e.g. "/methods/0/parameters/a1"
    std::string message;  ///< one sentence for the user
};

/// A list of problems; `ok()` is true if there is no Error.
class Diagnostics {
public:
    auto add(Diagnostic d) -> void;
    auto ok() const -> bool;
    auto all() const -> const std::vector<Diagnostic>&;
    auto errors() const -> std::vector<Diagnostic>;
    auto toString() const -> std::string;   ///< one line per diagnostic: "error V2-REF-001 file:record:path: message"
private:
    std::vector<Diagnostic> items_;
};
```

## 5. Build options (CMake)

| Option | Default | Effect |
|---|---|---|
| `TFUN_USE_AUTODIFF` | ON | as today |
| `TFUN_BUILD_TESTS` | OFF | as today; adds `tests/model` unit tests (new) |
| `TFUN_BUILD_PYTHON` | ON | as today |
| `TFUN_USE_YAML` | OFF | NEW. Links yaml-cpp; enables YAML in C++ readers |
| `TFUN_USE_SYMENGINE` | OFF | NEW (phase 5). Enables Tier-1 expression methods |
| `TFUN_WASM` | — | as today; YAML and SymEngine forced OFF |

New compile definitions are `THERMOFUN_WITH_YAML` and `THERMOFUN_WITH_SYMENGINE`, set `PUBLIC` only if a public
header needs them (it should not), otherwise `PRIVATE`.

## 6. Data flow at load and at calculation

```
load:   file/string/dir ─► FormatDetect ─► LegacyReader | V2Reader(+Normalize) ─► records
        records ─► Resolve (references, D48 defaults) ─► Validate (diagnostics)
        records ─► Substance/Reaction/Element objects (record + legacy view)  ─► Database maps (by symbol)

engine: ThermoEngine(database) ─► for each substance on first use: Binding(record.methods) ─► BoundRecord
        thermoPropertiesSubstance(T,P,symbol) ─► SubstanceExecutor(BoundRecord, WorkContext) ─► ThermoPropertiesSubstance
```

Binding happens lazily on first use of a record and is cached in the engine (keyed by symbol and a record
revision counter, so `Database::setSubstance` invalidates it), alongside today's memoization.
