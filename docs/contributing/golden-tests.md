# ThermoFun golden tests — a short guide

The golden tests check that the numbers of ThermoFun do not change when the code changes.
They compare new results with results stored in git.

## Words used here

| Word | Meaning |
|---|---|
| Golden reference | A stored result of a calculation. The test compares new results with it. |
| Subset | The records of a database that the tests use. A fixed list selects them. |
| Variant | A set of engine settings: `default`, `gas`, `errors`, `steam` or `berman`. |
| Fixture | A small input file made for a test. |
| Synthetic fixture | A fixture for a method that no bundled database uses. |
| Legacy format | The ThermoHub JSON format that ThermoFun reads today. |

## Run the tests

```bash
pytest pytests/golden -q
```

The tests need the `thermofun` Python module of the working tree.
Build and install it first. Use a clean checkout, not a tree with unfinished edits.

## What the tests check

| Test file | What it checks |
|---|---|
| `test_golden.py` | The values, the derivatives, the errors and the status of each stored property. The tolerance is 1e-10 (relative). |
| `test_key_inventory.py` | Each key in the legacy files is known. Each method number is valid. Each method has a golden record. |

## Compare two builds

The two-build comparison checks every record of every database, not only the subset.
It uses 15 temperatures and 6 pressures. The tolerance is 1e-10 (relative), as in `test_golden.py`.

- The values of G, H, U and A also have a margin of two ulps of P*V of the record: V passes through m3, and where the
  energy and P*V nearly cancel, one ulp of P*V is large compared with the energy.
- The differences between 1e-12 and 1e-10 are counted and shown. They are the rounding of the parameters between the
  units of a model and SI, where large terms nearly cancel. The comparison fails if their count is above
  `rounding_limit` in `expected_changes.json`: a change that raises the count updates the limit and says why.
- `expected_changes.json` lists the records that differ on purpose, each with its reason. An entry with `property` and
  `max_rel` accepts only differences of that property up to that size: the rest of the record stays checked.

```bash
bash pytests/golden/build_baseline.sh                 # once: builds the tag golden-baseline
python pytests/golden/compare_builds.py --base .golden-baseline/install
```

- `--new PREFIX` names the install prefix of the build to test. The default is the active environment.
- `--db NAME` and `--variant NAME` limit the work. Both may be used more than once.
- The run takes about 4 minutes and needs about 1 GB of temporary disk space.
- The exit code is 1 if one value, error or status differs.
- The baseline can be wrong. Then list the records in `pytests/golden/expected_changes.json`, with the reason.
  The comparison accepts differences in these records only, and prints how many it accepted.

The CI job `golden-diff` does the same on each push to `v2` and on each pull request. It keeps the baseline build
in a cache. The tag `golden-baseline` is on commit `da0e26a`.

## If a test fails

1. Read the table in the failure text. It shows the record, the property, the T and the P.
2. Find the change that moved the number.
3. Fix the change. Do not make the references again to make the test pass.

Make the references again only in a change that says so and explains each changed number.

## Add a database or a fixture

1. Put the file in `Resources/databases/` or under `pytests/`.
2. Add a new key to `KNOWN` in `test_key_inventory.py` only if the legacy reader handles it.
3. Run `python pytests/golden/make_references.py --new-only`.
4. Check that the total size of `pytests/golden/data/` is below 3 MB.
5. Commit the new data files together with the input file.

For a method that no database uses, add an entry to `make_synthetic_fixtures.py` and run it.
A method that legacy files cannot hold goes into `synthetic_api.py`.

## Limits of the tests

- A fixture records today's result. It does not prove that the result is correct.
- Some fixtures use made-up coefficients: `r14-logk_dolejs_manning10` and `s37-mv_eos_birch_murnaghan_gott97`.
- A model that does not set a property gives zero for that property. The reference keeps the zero.
