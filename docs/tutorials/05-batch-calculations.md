---
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors
jupytext:
  text_representation:
    extension: .md
    format_name: myst
    format_version: 0.13
kernelspec:
  display_name: Python 3
  language: python
  name: python3
---

# 5. Batch calculations

This page is the Jupyter notebook [`05-batch-calculations.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/05-batch-calculations.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn to calculate many records at many temperatures and pressures at once, to write the results with your units, digits and layout, and to compare two models and two databases.

**You need:** ThermoFun, pandas, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

**Related:** [Batch templates](06-batch-templates.md) · [Run a batch calculation](../how-to/make-tables.md) (all keys) · [Batch setups](../reference/batch.md) · [Use ThermoBatch, the 0.x batch API](../how-to/use-thermobatch.md) · [Units](../reference/units.md) · [Results](../reference/results.md)

A **batch calculation** calculates many records at many conditions at once. Its **setup** has four blocks:

| Block | What it says |
|---|---|
| `database` and `engine` | with what to calculate |
| `conditions` | where: T and P |
| `results` | what: one entry for each result table, with its substances or reactions and its properties |
| `output` | how to give the results: units, digits, layout, file |

A setup is a dict in Python, or a YAML file with the same keys. The smallest one is the template `blank`:

```{code-cell} ipython3
import os, pathlib
import thermofun as tf
import pandas as pd

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)

setup = tf.template("blank")
print(setup)
```

Each `<...>` is a placeholder: a setup with one left in does not run, and the message names where it is. `set` gives a value by its path: the keys from the top, and the number of a result in the list.

```{code-cell} ipython3
setup.set("/database", AQ)
setup.set("/conditions/T/values", [25, 100, 200])
setup.set("/conditions/P/values", ["saturation"])
setup.set("/results/0/name", "species")
setup.set("/results/0/substances", ["Ca+2", "Calcite"])
results = setup.run()
print(results["species"].to_text("md"))
```

At the saturation pressure, the result gives the pressure that the calculation used, here in bar.

```{code-cell} ipython3
:tags: [remove-cell]
assert len(results["species"].rows) == 6
```

## The block conditions

`T` and `P` are lists, `{values, unit}`, or `{from, to, step, unit}`; `saturation` in P is the saturation pressure of water. By default every T goes with every P (`points: grid`); `points: pairs` takes the first T with the first P, and so on. `vary_first` says which one changes from one row to the next: `P` (the default) or `T`.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100], "unit": "degC"}, "P": {"values": [1, 1000], "unit": "bar"}, "points": "pairs"},
    "results": [{"name": "species", "substances": ["Ca+2"], "properties": ["G", "S"]}],
}
print(tf.batch(setup)["species"].to_text("md"))
```

## The block results

A result has a name, the substances or the reactions, and the properties. A property is a name, or `{name, show, unit, digits, number_format, label}`. `show` is `value` (the default), `uncertainty`, `ddt`, `ddp` (the derivatives by T and P) or `status`. Instead of a list of symbols, give a rule: here all aqueous solutes with Ca. A setup can have several results.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"from": 25, "to": 100, "step": 25, "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "results": [
        {"name": "species", "substances": {"class": "aqueous_solute", "elements": ["Ca"]},
         "properties": [{"name": "G", "unit": "kJ/mol", "digits": 3}, {"name": "G", "show": "ddt", "label": "dG/dT"}]},
        {"name": "reaction", "reactions": ["Calcite = Ca+2 + CO3-2"], "properties": [{"name": "logK", "digits": 3}]},
    ],
}
results = tf.batch(setup)
print(results["species"].to_text("md"))
print(results["reaction"].to_text("md"))
```

The parts of the route of a result are properties too: `contribution:<method>:<property>`, `terms:<method>:<name>` and `variables:<method>:<name>` ([See how a result is calculated](../how-to/see-how-a-result-is-calculated.md)). Here the Born coefficient (a variable) and the Born term of G (a term) of two ions; the names are on the [page of the method](../reference/methods/HKF88.md):

```{code-cell} ipython3
terms = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 200, 300], "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "results": [{"name": "terms", "substances": ["Ca+2", "Na+"],
                 "properties": [{"name": "variables:HKF88:omega", "unit": "kJ/mol", "digits": 2}, {"name": "terms:HKF88:born_G", "unit": "kJ/mol", "digits": 3}]}],
}
print(tf.batch(terms)["terms"].to_text("md"))
```

The properties of water come from the water record: the density, the dielectric constant, the Born functions.

```{code-cell} ipython3
born = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
    "results": [{"name": "born", "substances": ["H2O@"], "properties": [{"name": "epsilon", "digits": 3}, {"name": "bornZ", "digits": 5}, "bornY", "bornQ"]}],
}
print(tf.batch(born)["born"].to_text("md"))
```

## The block engine

`options` are the [options of the engine](../reference/engine-preferences.md) by their names, `conventions` the conventions of the results, and `property_sources` where the properties of single records come from.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100], "unit": "degC"}, "P": {"values": [1], "unit": "bar"}},
    "engine": {"options": {"round_to_uncertainty": True}, "conventions": {"energy_reference": {"elements": "berman_brown"}}},
    "results": [{"name": "species", "substances": ["Calcite"], "properties": [{"name": "G", "unit": "kJ/mol"}]}],
}
print(tf.batch(setup)["species"].to_text("md"))
```

## The block output

`units` and `digits` take a property, `T` or `P`; `number_format` is `floating`, `fixed` or `scientific`. `layout` is `long` (a row for each record and condition), `wide` (a column for each record) or `grid` (a grid for each property: T down and P across, or the other way with `grid_across: T`). `rows_by: condition` puts the records of one condition together, `undefined` is the text of a value that is not defined, and `file` writes a file.

A block at the top is the default of every result; a result can give its own `engine`, `conditions` or `output`, which replace the defaults key by key.

```{code-cell} ipython3
grid = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200], "unit": "degC"}, "P": {"values": [100, 500, 1000], "unit": "bar"}},
    "output": {"layout": "grid", "units": {"T": "K"}},
    "results": [{"name": "logK", "reactions": ["Calcite = Ca+2 + CO3-2"], "properties": [{"name": "logK", "digits": 3}]}],
}
print(tf.batch(grid)["logK"].to_text("md"))
```

The extension of the file gives the format: `csv`, `md` or `tex` (or `file_format`). `results.write()` writes each result that has a file.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"from": 25, "to": 200, "step": 25, "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "output": {"file": "output/species.csv"},
    "results": [{"name": "species", "substances": ["Ca+2", "Calcite"], "properties": [{"name": "G", "unit": "kJ/mol", "digits": 3}, {"name": "S", "digits": 2}]}],
}
results = tf.batch(setup)
print(results.write())
```

## Get the numbers

`value` gives one value in the unit of its property, `scalar` one value with its derivatives and uncertainty in the units of the engine. The whole result goes to pandas from its text; `#` starts the comments of how it was made.

```{code-cell} ipython3
table = results["species"]
print(table.value("Ca+2", 25, "saturation", "G"), "kJ/mol")
print(table.scalar("Ca+2", 25, "saturation", "G").ddt)
pd.read_csv("output/species.csv", comment="#").head(3)
```

## How a result was made

A file starts with how it was made: the database and the SHA-256 of its records, the templates, the version, the physical constants, the methods of water, the conventions, and the options that you changed. The lines have no date: the same setup gives the same file, so a difference between two files is a difference in the results or in what they depend on.

```{code-cell} ipython3
print(table.provenance)
```

`output.provenance` changes this: `header` is the default, `file` writes a separate file with the whole setup, and `none` writes nothing.

## Compare two models and two databases

`compare` calculates each result once for each entry, with its own `database`, `replace` (other methods for some records) or `engine` block. `compare_difference: true` adds, for each property, the difference to the first entry. Here the volume and heat capacity of Ca+2 with the dielectric model of aq17 and with `HelgesonKirkham74`:

```{code-cell} ipython3
models = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200, 300], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
    "results": [{"name": "models", "substances": ["Ca+2"],
                 "properties": [{"name": "V", "unit": "cm3/mol", "digits": 3}, {"name": "Cp", "digits": 2}],
                 "compare": [{"label": "JN91"}, {"label": "HK74", "replace": {"H2O@": {"JohnsonNorton91": "HelgesonKirkham74"}}}],
                 "compare_difference": True}],
}
print(tf.batch(models)["models"].to_text("md"))
```

The volumes are close up to 100 degC (0.17 cm3/mol apart at 25 degC) and part above it (2.4 cm3/mol at 300 degC): the volume of an ion follows the derivative of the dielectric constant by P (the Born function Q), which the two models give differently.

Two databases: here a copy of aq17 in which G of Calcite is 1 kJ/mol lower. The log K of its dissolution is then lower by 1000/(RT ln 10): 0.175 at 25 degC, less at higher T.

```{code-cell} ipython3
changed = tf.Database(AQ)
G = changed.get("/substances/Calcite/standard_state_properties/G")
changed.set("/substances/Calcite/standard_state_properties/G", G - 1000.0)
changed.saveDataset("output/aq17-changed.json")

databases = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200], "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "results": [{"name": "databases", "reactions": ["Calcite = Ca+2 + CO3-2"], "properties": [{"name": "logK", "digits": 4}],
                 "compare": [{"label": "aq17"}, {"label": "changed", "database": "output/aq17-changed.json"}],
                 "compare_difference": True}],
}
print(tf.batch(databases)["databases"].to_text("md"))
```

```{code-cell} ipython3
:tags: [remove-cell]
difference = tf.batch(databases)["databases"].rows[0][3][2].value
assert abs(difference + 1000.0 / (8.31451 * 298.15 * 2.302585093)) < 1e-6
```

## When a value cannot be calculated

A value that cannot be calculated is written as `NaN`, and `problems` says why. The other values are there. The saturation pressure of water does not exist above the critical temperature (374 degC), so the value at 400 degC is not there.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"values": [300, 350, 400], "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "results": [{"name": "water", "substances": ["Ca+2"], "properties": [{"name": "G", "unit": "kJ/mol", "digits": 2}]}],
}
results = tf.batch(setup)
print(results["water"].to_text("md"))
print(results.problems)
```

A value can also be calculated and still be outside what its model is made for. It is in the result, and `problems` says so, so that you can decide. A setup that cannot run says why, with the nearest word:

```{code-cell} ipython3
try:
    tf.batch({"database": AQ, "conditions": {"T": [298.15], "P": [1e5]}, "output": {"layuot": "grid"},
              "results": [{"name": "x", "substances": ["Ca+2"], "properties": ["G"]}]})
except tf.BatchSetupError as error:
    print(error)
```

## Keep the setup in a file

```{code-cell} ipython3
setup = tf.template("species-properties", database=AQ, substances=["Ca+2"])
setup.save("output/my-setup.yaml")
print(pathlib.Path("output/my-setup.yaml").read_text())
```

Run it with `tf.batch("my-setup.yaml")`, or from a shell with `python -m thermofun.batch my-setup.yaml`.

**Next:** [6. Batch templates](06-batch-templates.md).
