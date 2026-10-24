# 10 — Documentation

## 1. Structure (Diátaxis)

Four kinds of pages, kept apart (§01 §12). Existing pages (`docs/index.md`, `GUIDE.md`, `python.md`,
`autodiff.md`) are split into these in Phase 6; until then new pages are added under the same folders.

```
docs/
  index.md                          what ThermoFun is, install, links to the four sections
  tutorials/                        learning by doing, one complete small task each
    01-first-calculation.md         load a bundled database, compute G of a species at T, P (Python and C++)
    02-your-own-data-v2.md          write a 3-substance v2 YAML file (water, an ion, a mineral) and compute
    03-batch-calculations.md        ThermoBatch to CSV
  how-to/                           steps for one job, for users who know the basics
    convert-legacy-database.md      python -m thermofun.convert, reading the report
    write-a-substance-v2.md         fields, methods, parameters, units, uncertainty, examples per substance class
    write-a-reaction-v2.md          stoichiometry, logK, methods, reaction-defined substances
    use-a-dataset-directory.md      thermohub-data layout, project scope, include
    validate-data.md                validate(), diagnostic codes, JSON Schema check
    choose-water-models.md          water EoS and dielectric methods, solvent symbol
    propagate-uncertainties.md      uncertainty in files, propagate_parameter_errors
    get-per-method-results.md       thermoPropertiesSubstanceDetailed
    migrate-code-from-enum-api.md   old setter/getter → record API, table
    add-a-method-as-data.md         write a Tier-1 method (Cp, ΔCp, V, log K forms), build, test, contribute
    add-a-cpp-method.md             register a method (provisional API)
  reference/                        facts only, generated where possible
    format-v2.md                    the v2 profile (from 04-format-v2.md, kept in sync by hand)
    methods/index.md                GENERATED: table of all methods
    methods/<MethodId>.md           GENERATED: one page per method (§2)
    diagnostics.md                  GENERATED: all diagnostic codes with meaning
    units.md                        accepted unit syntax, SI per slot
    legacy-format.md                legacy keys and their v2 equivalents (from 05 §2–§4)
    engine-preferences.md           EnginePreferences, EngineConventions (from GUIDE.md)
    python-api.md, cpp-api.md       links to the API docs (§3)
  explanation/                      background and reasons
    data-model.md                   records, SI inside, missing ≠ zero
    methods-and-slots.md            how a substance is evaluated (General → T → P), water special case
    units-and-uncertainty.md        why SI at the boundary, uncertainty conventions
    legacy-and-v2.md                why two formats, compatibility promise, deprecation timeline
    autodiff.md                     (existing autodiff.md)
  redesign/                         this plan (kept as the record of the design)
```

## 2. Generated reference pages

`python -m thermofun.docs generate docs/reference` (new module `python/src/thermofun/docs.py`, phase 2) writes:

- `methods/index.md`: one row per `MethodSpec`: id, title, operates on, classes, slot, tier, implemented,
  legacy names.
- `methods/<id>.md`:
  ```markdown
  # HKFGems
  Revised HKF equation of state (GEMS implementation). Citation: 1981HEL/KIR.
  | | |
  |---|---|
  | Operates on | substance (aqueous_solute) |
  | Slot | General |
  | Tier | 2 |
  | Needs solvent | yes |
  | Legacy names | `solute_hkf88_gems` (TPMethods id 2), CTPM_HKF (101) |

  ## Parameters
  | Name | SI unit | Unit in the model | Required | Description |
  |---|---|---|---|---|
  | a1 | J/(mol.Pa) | cal/(mol*bar) | yes | HKF a1 |
  ...
  ## Example (v2)
  (YAML block generated from the spec with placeholder values)
  ```
- `diagnostics.md`: from a C++ table `diagnosticCatalog()` (code, severity, meaning) exposed to Python.

CI checks that the generated files are up to date (`generate` into a temp dir and `diff -r`).

## 3. API documentation

- **C++**: the existing hdoc setup (`.hdoc.toml`, hosted at docs.hdoc.io) is kept; add `Model/`, `IO/` (public
  headers), `Methods/MethodSpec.h`, `Methods/MethodCatalog.h`, `Methods/MethodRegistry.h`,
  `Engine/DetailedResult.h` to what it scans (they are not in its ignore list, so nothing to do unless a folder is
  ignored). Every public declaration has a `///` comment (§02 §4.5).
- **Python**: pybind11 docstrings on every new binding: one sentence + units, e.g.
  `"Return the per-method breakdown of the properties of a substance at T (K) and P (Pa)."`
- Site generator for the narrative docs (decision Q13): Markdown in `docs/`, rendered by GitHub and the existing
  site; MkDocs Material or Sphinx+MyST only if a searchable site is wanted later.

## 4. Writing rules

- Short sentences, plain words, SI units named every time.
- Every how-to starts with "You need:" (files, versions) and ends with "Check:" (how to see it worked).
- Every code block in tutorials and how-tos is run in CI: tutorials are mirrored by `pytests/docs/test_docs_*.py`
  that execute the same code (copy, not extraction, to keep it simple).
- Examples use the bundled databases or files in `examples/v2/`.

## 5. Examples

`examples/v2/` (phase 3):
- `minimal.yaml` — water (`WaterHGK` + `JN91Reaktoro`), Na+ and Cl- (`HKFReaktoro`), halite (`CpIntegration` +
  `ConMolVol`), a reaction `NaCl@`;
- `minimal.py`, `minimal.cpp` — load and compute;
- `dataset/` — the directory layout (§04 §5) with two substances, one method definition, one bibliography entry.

## 6. Changelog

`CHANGELOG.md` at the repository root (new), "Keep a Changelog" sections per release: Added, Changed,
Deprecated, Removed, Fixed. Every step of [11-implementation-steps.md](11-implementation-steps.md) that changes
behaviour or API adds a line under "Unreleased".
