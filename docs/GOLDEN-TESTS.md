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

## If a test fails

1. Read the table in the failure text. It shows the record, the property, the T and the P.
2. Find the change that moved the number.
3. Fix the change. Do not make the references again to make the test pass.

Make the references again only in a plan step that says so.

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
- Some fixtures use made-up coefficients. The list is in the project notes.
- A model that does not set a property gives zero for that property. The reference keeps the zero.
