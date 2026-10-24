# ThermoFun restructuring for the ThermoHub v2 data format — plan index

This folder is the full plan for restructuring ThermoFun so that it reads the new ThermoHub v2 data format
(LinkML/JSON-Schema based, YAML authored) while keeping the current ThermoHub JSON format working.

It is written to be implemented **step by step by a coding agent** (or a person) that has not taken part in
the design discussion. Every step in [11-implementation-steps.md](11-implementation-steps.md) names the files
to touch, the code to write, the tests to add, the commands to run, and when the step is done.

Background documents (not in this repository, kept by the ThermoHub redesign effort):
`thermohub-redesign-summary.md` (decisions D1–D65, open points O1–O84) and `thermohub-redesign-examples.md`
(examples E1–E24). References like `D13`, `O30`, `E7` point to them. The parts that matter for ThermoFun are
restated in these files, so the plan can be followed without them.

## Files

| File | Content | Read when |
|---|---|---|
| [01-research.md](01-research.md) | How comparable projects (Cantera, Reaktoro, CHNOSZ, ESPEI/pycalphad, OPTIMADE, LinkML, NumPy) handle formats, models, units, compatibility, docs; what ThermoFun adopts | Before design changes |
| [02-architecture.md](02-architecture.md) | Layers, dependency rules, folder and file layout, naming and coding conventions | Before writing any code |
| [03-data-model.md](03-data-model.md) | The format-neutral C++ record types (`Quantity`, `SubstanceRecord`, …) with full declarations; vocabularies | Steps in phase 1 |
| [04-format-v2.md](04-format-v2.md) | The v2 file format as ThermoFun reads it ("ThermoFun profile" of ThermoHub v2), normalisation rules, file layout, diagnostics | Phase 3 |
| [05-legacy-compatibility.md](05-legacy-compatibility.md) | Legacy JSON → model mapping key by key, quirks, legacy writer, old API kept as a view, downstream users, deprecation timeline | Phases 1, 3, 6 |
| [06-methods-and-executor.md](06-methods-and-executor.md) | Method registry, `MethodSpec`, per-method parameter tables, stages, executor algorithm, every special case of today's engine | Phase 2 |
| [07-units-and-uncertainty.md](07-units-and-uncertainty.md) | Unit strings (UCUM front end), conversion at the boundary, uncertainty model and conversions | Phases 1, 3 |
| [08-public-api.md](08-public-api.md) | C++ and Python API: what stays, what is new, signatures, names, examples | All phases |
| [09-testing.md](09-testing.md) | Golden-reference suite, round-trip tests, fixtures, CI changes | Phase 0 and every step |
| [10-documentation.md](10-documentation.md) | Documentation structure (Diátaxis), what is generated, docstring rules, changelog | Phases 2, 3, 6 |
| [11-implementation-steps.md](11-implementation-steps.md) | **The ordered list of steps** (S0.1 … S6.x) with files, code, tests, done-criteria, commit messages | While implementing |
| [12-open-questions.md](12-open-questions.md) | Decisions still to take (with a recommendation each), to discuss one by one | Before the phase that needs them |

## Phases at a glance

| Phase | Goal | Release |
|---|---|---|
| 0 | Safety net: golden references (committed subset + two-build comparison in CI), key inventory, fixtures | — |
| 1 | Format-neutral internal model; legacy reader fills it; engine unchanged | 0.8.0 |
| 2 | Method registry, named parameters, executor replaces the `switch` blocks; per-method results | 0.9.0 |
| 3 | v2 reader/writer (JSON core, YAML optional), dataset directory loader, validation | 0.10.0 |
| 4 | Converter legacy → v2 (Python CLI), migration report; v2 copies of bundled databases | 0.10.x |
| 5 | Shared constants as parameter sets; optional Tier-1 symbolic methods | 0.11.0 |
| 6 | v2 is the documented default; deprecations of the enum/positional API | 1.0.0 |

## Rules for the implementer

These apply to every step. When a step seems to need breaking one, stop and ask.

1. **Numbers never change.** After each step, `pytest` (including `pytests/golden/`) and `ctest` pass, and the
   two-build comparison against the `golden-baseline` tag shows no difference. Together they compare values,
   `ddt`, `ddp`, `err` and status of every substance and reaction of every bundled database with the results
   before the refactoring (Phase 0, 09 §1). A difference is a bug in the step,
   not a reason to regenerate the references. References are regenerated only in a step that says so.
2. **The public API only grows.** Do not remove or change the signature of anything in an installed header or
   in the Python module until Phase 6 says so. `tests/installed_headers` must stay green.
3. **One step = one commit** (or a few small ones), with the commit message given in the step. Do not mix
   steps. Do not reformat code you do not otherwise change.
4. **Follow the conventions** in [02-architecture.md](02-architecture.md) (names, files, comments, errors).
5. **Write the tests the step lists before or with the code**, not after the next step.
6. **Do not add dependencies** other than those named in a step. Optional dependencies are behind CMake
   options that default to `OFF` and the build must work without them.
7. **Keep the maths.** Model classes in `Substances/` and `Reactions/` are not edited except where a step says
   so (Phase 5). New code calls them; it does not re-implement them.
8. **When the code disagrees with this plan, the code is right about today's behaviour.** Note the difference
   in the step's commit message and in [12-open-questions.md](12-open-questions.md), keep today's behaviour.
9. **Build and test commands** (Linux, conda environment from `environment.devenv.yml`):
   ```bash
   cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DTFUN_BUILD_TESTS=ON
   cmake --build build
   ctest --test-dir build --output-on-failure
   cmake --install build          # installs the python package into the active environment
   pytest -ra .
   bash pytests/golden/build_baseline.sh                                  # once
   python pytests/golden/compare_builds.py --base .golden-baseline/install
   ```
   Also build once with `-DTFUN_USE_AUTODIFF=OFF` at the end of each phase.

## Glossary (short)

| Term | Meaning here |
|---|---|
| **legacy format** | Today's ThermoHub JSON: `TPMethods`, `sm_gibbs_energy: {values, errors, units}`, `{"29": "water_eos_hgk84_lvs83_gems"}` |
| **v2 format** | ThermoHub v2: records with `standard_state_properties`, `methods: [{method, parameters, applicability}]`, `stoichiometry`; YAML for authored files, JSON on the wire |
| **model** (record model) | The C++ types in `ThermoFun/Model/` that hold data independent of the file format |
| **method** | A named algorithm (`HKFGems`, `Berman88Vol`, `LogKAnalytic`) that computes properties of a substance or reaction at T, P |
| **method application** | The use of one method on one record, with its parameter values and range |
| **binding** | Turning a method application into ready-to-run inputs for a model class (names → positions, SI → the class's units) once, at load |
| **executor** | The code that runs a record's method applications in order and assembles the result (replaces today's `switch` blocks) |
| **golden suite** | The reference results that guard against numerical changes |
| **view** | An old API call kept by computing its answer from the new model |
