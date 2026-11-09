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

You learn to write a table with your units, digits and layout, and to see how a table was made. It takes 15 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A setup says what to calculate and how to write it. Here it is a dict. In a file it is YAML, with the same keys.

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

A table of the water record gives its properties.

```{code-cell} ipython3
results = tf.table(template="water-properties", database=AQ)
print(results["water"].to_text("md"))
```

## When a value cannot be calculated

A value that cannot be calculated stays empty, with a message. The other values are there.

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records=["Ca+2", "NoSuch"])
print(results["species"].to_text("md"))
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
