# 01 — What comparable projects do, and what ThermoFun adopts

A survey of how similar scientific libraries and data standards handle data formats, model selection, units,
compatibility with old formats, extension, and documentation. Each section ends with what ThermoFun takes from it.
Sources are listed at the end; claims that come from a project's own benchmarks are marked as such.

---

## 1. Cantera (C++/Python, chemical kinetics and thermodynamics)

**Format.** One YAML file has top-level *sections* (`phases`, `species`, `reactions`, `elements`), each a list of
mappings. A species has `name`, `composition` (`{C: 1, H: 4}`), and a `thermo` mapping with a `model` string
(`NASA7`, `NASA9`, `Shomate`, `constant-cp`, `piecewise-Gibbs`) and that model's parameters. Piecewise models use
`temperature-ranges: [300, 1000, 5000]` plus one `data` row per range, in ascending order.

**Units.** Cantera works internally in SI. Any value may carry units after it, separated by a space
(`Ea: 21033 kJ/kmol`), or a `units:` mapping sets default units for everything in the same mapping or list and
below it (`units: {energy: cal, quantity: mol, pressure: atm}`); inner mappings inherit and can override.
Compound units use `*`, `/`, `^`, with metric prefixes.

**Legacy formats.** YAML was introduced in Cantera 2.5 as the replacement for the CTI and XML (CTML) formats.
Cantera shipped converters as Python modules (`python -m cantera.cti2yaml mech.cti`, `ctml2yaml`, `ck2yaml`
for Chemkin), with options such as `--no-validate`, and a tutorial on converting legacy files. Old formats stayed
loadable for some releases while the converters existed.

**Extension.** Object creation goes through factories (`Factory<T>`): a model name is registered with a
constructor function (`reg`), and `addAlias` adds other names for an existing model. Species objects keep the
input data they were created from, and a `YamlWriter` writes objects back to YAML.

**ThermoFun adopts:**
- Piecewise-in-T parameters as an ordered list of ranges with one parameter set each (our `segments`, §04).
- A `model`/`method` string per thermodynamic contribution, resolved through a registry with **aliases** — the
  legacy method names (`solute_hkf88_gems`) become aliases of the new ids (`HKFGems`) (§06).
- Converters shipped as Python modules (`python -m thermofun.convert`) with a validation step that can be skipped.
- Keeping unknown input fields on the record so a written file round-trips (§03 `extra`).
- Scoped default units (`units:` at file, record or method level) — optional convenience in v2 (§07).

## 2. Reaktoro v2 (C++/Python, chemical equilibrium and kinetics)

**Models as functions built from named parameter structs.** For example `StandardThermoModelParamsHKF` has the
named fields `Gf, Hf, Sr, a1, a2, a3, a4, c1, c2, wref, charge, Tmax`, each documented with its SI unit
(`a1` in J/(mol·Pa)). `StandardThermoModelHKF(params)` returns a callable model. Serialisation is declared per
struct with `required(...)` / `optional(...)` decoders, so missing required parameters are reported by name.
A generic `Model<Result(Args...)>` supports memoisation and `chain(models...)` (apply several models in order).

**Databases.** YAML databases with one block per species and its standard thermodynamic model. Reaktoro also
reads ThermoFun databases through its `ThermoFunDatabase` extension, which calls ThermoFun's public C++ API
(`Database::mapSubstances()`, `Substance::symbol()`, `Substance::aggregateState()` with the
`ThermoFun::AggregateState` enum, `Element`, and engine property calls).

**ThermoFun adopts:**
- **Named parameters with documented SI units per method**, declared once (our `MethodSpec.parameters`, §06).
- Required/optional parameter declarations with errors naming the missing parameter.
- An ordered chain of model steps (our executor stages, §06).
- **A compatibility constraint:** the API Reaktoro uses must keep working unchanged (§05, §08).

## 3. CHNOSZ / OBIGT (R)

The OBIGT database is a set of flat CSV files with one row per species: name, abbreviation, formula, state,
references, date, a `model` column (`HKF` for most aqueous species, `Berman` for many minerals, `CGL` for other
crystalline/gas/liquid species), an `E_units` column (cal or J for that row), G, H, S, Cp, V, then parameter
columns with compound names (`a1.a`, …, `omega.lambda`, `z.T`) whose meaning depends on the model. Unknown values
are `NA`. If the model is missing, it defaults to HKF for aqueous species and CGL otherwise.

**ThermoFun adopts:**
- Defaults by substance class are useful, but they must be **explicit and documented**, never silent.
- Per-record energy unit defaults (`E_units`) show that a record-level `units:` default is natural for authors.
- Tabular authoring is a tooling layer that compiles to the canonical format (as the design summary already says),
  not a second format inside ThermoFun.

## 4. ESPEI / pycalphad (Python, CALPHAD)

Input data are JSON files; a dedicated command (`espei --check-datasets <folder>`) validates all datasets before a
fit and reports errors, which stop the fit.

**ThermoFun adopts:** a separate validation entry point that **collects all problems** and reports them as a list
(`Database::validate()` and `python -m thermofun.validate`), instead of stopping at the first error or ignoring it.

## 5. OPTIMADE (materials database API standard)

The specification uses semantic versioning for the API: minor releases must not break clients that followed the
earlier version. Provider-specific extensions live in a namespace marked by a unique, registered prefix, so
extensions never collide with standard names. The changelog notes when a change could break strict clients.

**ThermoFun adopts:**
- Reading a v2 file with a **newer minor** schema version must work (unknown fields kept, warned once); a newer
  **major** version is refused with a clear message.
- ThermoFun-specific fields written into v2 files (if any) are prefixed `_thermofun_` and never required.

## 6. LinkML and JSON Schema

LinkML generates JSON Schema, JSON-LD context, Pydantic and documentation from one YAML schema. Units can be
declared on slots (`unit: {ucum_code: ...}`, mapped to `qudt:ucumCode`; unit support since LinkML 1.3).
`gen-json-schema` produces a JSON Schema (draft-07 in the published examples).

For C++, the nlohmann-based `json-schema-validator` (pboettch) supports JSON Schema draft-7; Blaze supports drafts
4 to 2020-12 but uses its own JSON library.

**ThermoFun adopts:**
- The JSON Schema released by `thermohub-schema` is the contract (D62); ThermoFun hand-writes its JSON reading.
- Schema validation of files runs in **Python/CI** (`jsonschema` package) and in `thermofun.validate`; the C++
  library does its own semantic checks (references, units, parameters) and does not depend on a schema validator.
  An optional C++ validator can be added later behind a CMake option (decision Q8).

## 7. Compatibility and deprecation policy (NumPy NEP 23)

NumPy's policy: avoid breaking user code without need, **never make changes that silently produce wrong
results**, and give deprecation warnings for at least two releases before removing behaviour. Many users do not
read warnings and upgrade late.

**ThermoFun adopts** (§05): deprecations only after the replacement is the documented default (1.0), warnings for
at least two minor releases, removal not before 2.0; any change that could alter numbers is guarded by the golden
suite and is a bug unless a step explicitly accepts it.

## 8. Units libraries in C++

`mp-units` is a compile-time units library (proposed for C++ standardisation); it checks dimensions in code but
is not a parser for unit strings read from files. LLNL `units` is a runtime library with conversion from and to
strings and a UCUM domain.

**ThermoFun adopts:** keep the existing runtime converter (`Common/Units.*`, already used for every value read)
and add a small **UCUM syntax front end** for the unit strings that occur in thermodynamic data (§07). LLNL
`units` is the fallback if the unit coverage proves insufficient (decision Q5).

## 9. YAML in C++

`yaml-cpp` is widely used (Cantera uses it). `rapidyaml` reports itself as 10–70× faster than yaml-cpp for
parsing (project's own benchmarks). Database files are at most a few MB, read once.

**ThermoFun adopts:** YAML support in C++ is optional (`TFUN_USE_YAML=OFF` by default); the YAML tree is converted
to `nlohmann::json` and parsed by the JSON code, so there is a single reader. Library: yaml-cpp
(decision Q4: available on conda-forge, used by Cantera; speed does not matter here).
Python users always get YAML through PyYAML.

## 10. Python bindings and packaging

pybind11 (used today) is the most widely used binding library. nanobind has near-identical syntax and reports
faster compiles, smaller binaries and lower call overhead (project's own benchmarks). scikit-build-core is a PEP 517
build backend for CMake-based extensions that works with both.

**ThermoFun adopts:** keep pybind11 during the restructuring (no binding rewrite while the API grows). Moving the
Python packaging to scikit-build-core is a separate, optional step at the end (decision Q9).

## 11. Project layout (P1204R0, Pitchfork)

P1204R0 (canonical project structure, proposal to the C++ committee's tooling group) keeps a library's headers and
sources together in a directory named after the library, with tests next to the code or in a tests directory.
The Pitchfork layout prescribes root directories such as `docs/`, `tests/`, `build/` (ephemeral, not part of the
source) and discourages other root directories.

**ThermoFun adopts:** the current layout already matches P1204 (headers and sources together in `ThermoFun/`,
installed with the same hierarchy). New code goes into **new subdirectories of `ThermoFun/`**; existing model
folders (`Substances/`, `Reactions/`) are not moved (§02). Root directories stay as they are.

## 12. Documentation (Diátaxis)

Diátaxis splits documentation into four kinds, kept on separate pages: **tutorials** (learning by doing a small
complete task), **how-to guides** (steps for a specific job), **reference** (facts: API, format, parameters — no
instructions) and **explanation** (background, reasons). Mixing kinds, especially explanation inside how-to
guides, is the common failure.

**ThermoFun adopts** (§10): `docs/` organised by the four kinds; the format reference and the method/parameter
reference are **generated from the method registry** so they cannot drift from the code.

---

## Summary of choices

| Topic | Choice | From |
|---|---|---|
| Method selection | String ids + registry + aliases for legacy names | Cantera factories |
| Parameters | Named, unit-documented, required/optional per method | Reaktoro params structs |
| Piecewise models | Ordered `segments` with T ranges | Cantera `temperature-ranges` |
| Composition of models | Ordered stages (chain), with additive terms | Reaktoro `chain`, ThermoFun today |
| Units | SI inside, UCUM strings in files, scoped defaults optional | Cantera, LinkML, D14 |
| Legacy formats | Converter as Python module; legacy reader kept for 1.x | Cantera |
| Validation | Collect-all diagnostics, separate validate command | ESPEI |
| Versioning | Minor = compatible, major refused; prefixed extensions | OPTIMADE |
| Deprecation | ≥ 2 minor releases of warnings; never silent wrong results | NumPy NEP 23 |
| YAML (C++) | Optional, converted to JSON tree | — |
| Bindings | pybind11 kept | — |
| Layout | Keep P1204-style `ThermoFun/`, add subfolders | P1204R0 |
| Docs | Diátaxis; generated reference | Diátaxis |

## Sources

- Cantera YAML input reference and general structure (units, sections): [cantera.org/3.2/yaml](https://cantera.org/3.2/yaml/index.html), [YAML format tutorial](https://cantera.org/tutorials/yaml/yaml-format.html), source of the species and general pages in the Cantera repository (`doc/sphinx/yaml/species.md`, `general.md`).
- Cantera legacy conversion: [Converting legacy CTI and XML input files to YAML](https://cantera.org/documentation/dev/sphinx/html/userguide/legacy2yaml-tutorial.html), [cti2yaml](https://cantera.org/3.1/yaml/cti2yaml.html), [ctml2yaml](https://cantera.org/3.1/yaml/ctml2yaml.html).
- Cantera factories: [ExtensionManagerFactory](https://cantera.org/3.1/cxx/d7/d2a/classCantera_1_1ExtensionManagerFactory.html), [ThermoFactory](https://cantera.org/3.2/cxx/dc/d8e/ThermoFactory_8h_source.html), [YamlWriter](https://cantera.org/3.2/cxx/de/d52/classCantera_1_1YamlWriter.html).
- Reaktoro: [Creating thermodynamic databases](https://reaktoro.org/tutorials/advanced/creating-custom-databases.html), [StandardThermoModelParamsHKF](https://reaktoro.org/api/structReaktoro_1_1StandardThermoModelParamsHKF.html), [ThermoFunDatabase](https://reaktoro.org/api/classReaktoro_1_1ThermoFunDatabase.html); source files `Reaktoro/Models/StandardThermoModels/StandardThermoModelHKF.hpp`, `Reaktoro/Serialization/Models/StandardThermoModels.cpp`, `Reaktoro/Core/Model.hpp`, `Reaktoro/Extensions/ThermoFun/ThermoFunDatabase.cpp`.
- CHNOSZ: [Customizing the thermodynamic database](https://cran.r-project.org/web/packages/CHNOSZ/vignettes/custom_data.html), [thermo() reference](https://search.r-project.org/CRAN/refmans/CHNOSZ/html/thermo.html).
- ESPEI: [Making ESPEI datasets](https://espei.readthedocs.io/en/0.8/input_data.html).
- OPTIMADE: [specification](https://github.com/Materials-Consortia/OPTIMADE/blob/master/optimade.rst), [changelog](https://github.com/Materials-Consortia/OPTIMADE/blob/develop/CHANGELOG.md).
- LinkML: [ucum_code slot](https://linkml.io/linkml-model/latest/docs/ucum_code/), [generators](https://linkml.io/linkml/_sources/generators/index.rst.txt).
- JSON Schema in C++: [pboettch/json-schema-validator README](https://github.com/pboettch/json-schema-validator/blob/main/README.md), [Blaze](https://tool.apievangelist.com/store/blaze).
- NumPy: [NEP 23 — Backwards compatibility and deprecation policy](https://numpy.org/neps/nep-0023-backwards-compatibility.html).
- Units: [mp-units](https://github.com/mpusz/units), [LLNL units](https://github.com/LLNL/units), [LLNL units docs](https://units.readthedocs.io/en/stable/details/index.html).
- YAML: [rapidyaml "Is it rapid?"](https://rapidyaml.readthedocs.io/latest/sphinx_is_it_rapid.html).
- Bindings/packaging: [nanobind](https://github.com/hawkinsp/nanobind), [scikit-build-core](https://pydevtools.com/handbook/reference/scikit-build-core/).
- Layout: [P1204R0 Canonical Project Structure](https://open-std.org/JTC1/SC22/WG21/docs/papers/2018/p1204r0.html), [Pitchfork-based template](https://github.com/kardbord/cpp-project-template).
- Documentation: [Diátaxis at Canonical](https://ubuntu.com/blog/diataxis-a-new-foundation-for-canonical-documentation), [Diátaxis template](https://diataxis-template.readthedocs.io/en/latest/reference/diataxis-documentation/).
