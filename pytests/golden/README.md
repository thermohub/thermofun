# Golden reference results

These tests guard the numbers of ThermoFun during the redesign.
They compare today's results with results stored in git.

## Run

```bash
pytest pytests/golden -q
```

Each test loads one database, computes the stored records again and compares them.
The relative tolerance is 1e-10.
The test compares `val`, `ddt`, `ddp`, `err` and the status code of every property.
It also compares the first line of every error message.

## Files

| File | Purpose |
|---|---|
| `golden_common.py` | Computation and comparison. The two-build comparison uses it too. |
| `make_references.py` | Writes `data/subset.json` and `data/*.json.gz`. |
| `test_golden.py` | The pytest test. |
| `test_key_inventory.py` | Checks the legacy keys and method ids, and that every method has a golden record. |
| `make_synthetic_fixtures.py` | Writes `pytests/data/legacy-synthetic/*.json`. |
| `synthetic_api.py` | The database for the methods that legacy files cannot hold (ids 41 and 42). |
| `data/subset.json` | The selected records of each database. Fixed after the first run. |
| `data/<database>__<variant>.json.gz` | The stored results of one database and one variant. |

## When to regenerate

Regenerate the references only in a redesign step that says so.
A difference in a test is a defect in the change, not a reason to regenerate.

```bash
python pytests/golden/make_references.py            # uses data/subset.json
python pytests/golden/make_references.py --reselect  # selects the records again
python pytests/golden/make_references.py --new-only  # only databases and variants without a data file
```

## What is computed

- Grid: T = 273.15, 298.15, 373.15, 623.15, 1273.15 K. P = 0 (saturation), 1e5, 1e8, 5e8 Pa.
- Databases: `Resources/databases/*.json`, the test databases in `pytests/` that load,
  and `tests/extra_models/extra_models_db.json`. A file with the same content as an earlier file is skipped.
- Variants: `default` and `gas` for every database.
  `errors`, `steam` and `berman` only for `aq17-thermofun`, `mines16-thermofun`,
  `pytests--Substances--Solute--test-hkf-thermofun` and `tests--extra_models--extra_models_db`.
- Substance properties: gibbs_energy, enthalpy, entropy, heat_capacity_cp, volume, heat_capacity_cv,
  internal_energy, helmholtz_energy.
- Reaction properties: log_equilibrium_constant, reaction_gibbs_energy, reaction_enthalpy,
  reaction_entropy, reaction_heat_capacity_cp, reaction_volume.
- Solvent records (class AQSOLVENT): all scalar members of `propertiesSolvent` and `electroPropertiesSolvent`.
- Synthetic fixtures: `pytests/data/legacy-synthetic/*.json`, one for each method that no bundled database uses,
  and the API database `synthetic-api`. All their records are in the subset.
  The fixtures with a water solvent have the variants `default`, `steam` and `berman` only.
- A call that raises an exception is stored as an error with the first line of the message.

## Subset rules

Each subset is computed inside its full database.

1. Substances are grouped by class, aggregate state, the three legacy method codes, defining reaction,
   phase-transition parameters and number of Cp intervals. The first 2 symbols of each group are taken.
2. Reactions are grouped by the three legacy method codes and by logK, volume and Cp reference values.
   The first 2 symbols of each group are taken.
3. Always included: every solvent record, `H+`, the defining reaction of each selected substance,
   and the substances in the reactions that are in the subset.

Size rule (09-testing.md section 1.3, rule 5): the data must stay below 3 MB.
Taking 3 symbols per group gave 5.5 MB. So these reductions apply:

- 2 symbols per group instead of 3.
- `errors`, `steam` and `berman` only for the four databases above.
