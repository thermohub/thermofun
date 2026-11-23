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

# 6. Output tables

This page is the Jupyter notebook [`06-output-tables.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/06-output-tables.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn to write a table with your units, digits and layout, and to see how a table was made.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

**Related:** [Make tables](../how-to/make-tables.md) (all settings) · [Templates](05-templates.md) · [Batch options](../reference/batch.md) · [Units](../reference/units.md) · [Results](../reference/results.md) · [Provenance and conventions](../reference/conventions.md)

In [tutorial 5](05-templates.md) you used a ready setup, a template. Here you write the setup yourself, key by key. A template is only a setup that someone saved, so everything here also works with a template. A setup says what to calculate (records, conditions, columns) and how to write it (units, digits, layout, format). Here it is a dict. In a file it is YAML, with the same keys.

```{code-cell} ipython3
import os, pathlib
import thermofun as tf
import pandas as pd

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)

setup = {
    "database": AQ,
    "conditions": {"T": {"from": 25, "to": 200, "step": 25, "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "tables": [{
        "name": "species",
        "records": ["Ca+2", "Calcite"],
        "columns": [
            {"property": "G", "unit": "kJ/mol", "digits": 3},
            {"property": "S", "digits": 2},
            {"property": "G", "show": "ddt", "label": "dG/dT"},
        ],
    }],
}
table = tf.table(setup)["species"]
print(table.to_text("csv"))
```

At the saturation pressure, the table gives the pressure that the calculation used, here in bar. To ask for a value by its point, say `"saturation"`: `table.value("Ca+2", 25, "saturation", "G")`, and `table.pressure_used("Ca+2", 25, "saturation")` gives that pressure.

## Get the numbers

`values` gives them in the units of the columns. `value` gives one value, and `scalar` gives one value with its derivatives and uncertainty, in the units of the engine.

```{code-cell} ipython3
print(table.value("Ca+2", 25, "saturation", "G"), "kJ/mol")
print(table.scalar("Ca+2", 25, "saturation", "G").ddt)

labels = [c["label"] for c in table.columns]
frame = pd.DataFrame(table.values(), columns=labels)
frame.insert(0, "Symbol", [row[0] for row in table.rows])
frame.insert(1, "T (degC)", [row[1] for row in table.rows])
frame.head()
```

## Layouts

`long` has one row for each record and condition. `transposed` has one column for each record. `grid` has one grid for each property: T down and P across, or the other way with `across: T`.

```{code-cell} ipython3
setup["tables"][0]["columns"] = [{"property": "G", "unit": "kJ/mol", "digits": 2}]
setup["tables"][0]["layout"] = "transposed"
print(tf.table(setup)["species"].to_text("md"))
```

```{code-cell} ipython3
grid = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200], "unit": "degC"}, "P": {"values": [100, 500, 1000], "unit": "bar"}},
    "tables": [{"name": "logK", "reactions": ["Calcite = Ca+2 + CO3-2"], "columns": [{"property": "logK", "digits": 3}], "layout": "grid"}],
}
print(tf.table(grid)["logK"].to_text("md"))
```

## Formats

The extension of the file gives the format: `csv`, `md` or `tex`. In a setup, `output` names the file, and `results.write()` writes each table that has one.

```{code-cell} ipython3
table = tf.table(setup)["species"]
print(table.write("output/species.csv"))
print(table.write("output/species.md"))
print(table.write("output/species.tex"))
print(pathlib.Path("output/species.tex").read_text()[-300:])
```

## How a table was made

A file starts with how it was made: the database and the SHA-256 of its records, the templates, the version, the physical constants, the methods of water, the conventions, and the options that you changed. The lines are comments.

The lines have no date. The same setup gives the same file, so a difference between two files is a difference in the results or in what they depend on.

```{code-cell} ipython3
print(pathlib.Path("output/species.csv").read_text())
```

To read such a CSV file with pandas, say that `#` starts a comment:

```{code-cell} ipython3
pd.read_csv("output/species.csv", comment="#").head(3)
```

The setting `provenance` changes this: `header` is the default, `file` writes a separate file with the whole setup, and `none` writes nothing.

```{code-cell} ipython3
setup["tables"][0]["provenance"] = "file"
tf.table(setup)["species"].write("output/plain.csv")
print(pathlib.Path("output/plain.csv").read_text()[:60], "...\n")
print(pathlib.Path("output/plain.csv.provenance.yaml").read_text()[:700])
```

You can also read it from the table:

```{code-cell} ipython3
print(table.provenance.lines())
```

## Water

A table of the water record gives its properties. The template `water-properties` has the density and its derivatives, the dielectric constant and the others.

For a table of your own, name the columns. For example, the dielectric constant of water and the Born functions Z, Y and Q at three temperatures, which the HKF model of aqueous species uses:

```{code-cell} ipython3
born = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 100, 200], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
    "tables": [{"name": "born", "records": ["H2O@"],
                "columns": [{"property": "epsilon", "digits": 3}, {"property": "bornZ", "digits": 5}, {"property": "bornY"}, {"property": "bornQ"}]}],
}
print(tf.table(born)["born"].to_text("md"))
```

The parts of the route of a result are columns too (decision 0030): `contribution:<method>:<property>`, `terms:<method>:<name>` and `variables:<method>:<name>`; the names of the terms and variables are on the [page of the method](../reference/methods/HKF88.md). Here the Born coefficient (a variable) and the Born term of G (a term) of two ions:

```{code-cell} ipython3
terms = {
    "database": AQ,
    "conditions": {"T": {"values": [25, 200, 300], "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "tables": [{"name": "terms", "records": ["Ca+2", "Na+"],
                "columns": [{"property": "variables:HKF88:omega", "unit": "kJ/mol", "digits": 2}, {"property": "terms:HKF88:born_G", "unit": "kJ/mol", "digits": 3}]}],
}
print(tf.table(terms)["terms"].to_text("md"))
```

The template `water-properties`:

```{code-cell} ipython3
results = tf.table(template="water-properties", database=AQ)
print(results["water"].to_text("md"))
```

## When a value cannot be calculated

A value that cannot be calculated is written as `NaN`, and `problems` says why. The other values are there. The saturation pressure of water does not exist above the critical temperature (374 degC), so the cell at 400 degC has no value.

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"values": [300, 350, 400], "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "tables": [{"name": "water", "records": ["Ca+2"], "columns": [{"property": "G", "unit": "kJ/mol", "digits": 2}]}],
}
results = tf.table(setup)
print(results["water"].to_text("md"))
print(results.problems)
```

A value can also be calculated and still be outside what its model is made for. It is in the table, and `problems` says so, so that you can decide. Here Ca+2 is calculated with the HKF model at a high T, where the density of water is below the range of the model:

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records=["Ca+2"], T={"values": [300, 400], "unit": "degC"}, P={"values": [500], "unit": "bar"})
print(results["species"].to_text("md"))
print(results.problems)
```

A record that is not in the database is a problem too:

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records=["Ca+2", "NoSuch"])
print(results.problems)
```

A setup that cannot run says why, with the nearest word:

```{code-cell} ipython3
try:
    tf.table(template="species-properties", database=AQ, records=["Ca+2"], layuot="grid")
except tf.tables.SetupError as error:
    print(error)
```

**Next:** [7. Batch calculations](07-batch-calculations.md).
