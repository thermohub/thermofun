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

# 5. Templates

This page is the Jupyter notebook [`05-templates.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/05-templates.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn to make a table from a template, to change it, and to make your own template.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A template is a ready setup of a table. You give your records, and the template has the rest: the conditions (T and P), the columns, the units and the digits. The table is then calculated and written as text (`md`, `csv`, `tex`) or read as numbers.

There are two kinds of templates:

- **templates of tables** say what to calculate, for example `species-properties`: G, H, S, Cp and V of substances;
- **templates of settings** do not calculate anything. They give a part of a setup: `batch-*` give the conditions (T and P), and `output-*` give the units and the digits. You combine them with a template of tables.

You can also write your own templates, to export the data as you want them: see [Make your own template](#make-your-own-template) at the end. A template is a short YAML file.

**Related:** [Make tables](../how-to/make-tables.md) (all settings of a setup) · [Output tables](06-output-tables.md) · [Batch options](../reference/batch.md) · [Units](../reference/units.md) · [Properties and their symbols](../reference/results.md)

```{code-cell} ipython3
import os, pathlib
import thermofun as tf

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)
```

## Which templates are there

The first line of a template says what it makes. The templates `batch-*` and `output-*` have no tables: they are settings to combine with the others.

```{code-cell} ipython3
import pandas as pd

pd.Series(tf.tables.templates(), name="what it makes").to_frame()
```

## See what a template says

```{code-cell} ipython3
print(tf.TableSetup.templateText("species-properties"))
```

## Use a template with your records

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records=["Ca+2", "Calcite"])
print(results["species"].to_text("md"))
```

```{code-cell} ipython3
results = tf.table(template="reaction-properties", database=AQ, reactions=["Calcite = Ca+2 + CO3-2"])
print(list(results), "\n")
print(results[list(results)[0]].to_text("md")[:900])
```

Instead of a list of symbols, give a rule. The records that fit the rule are the rows of the table. This rule takes all aqueous solutes that have the element Ca. The rules are in [Make tables](../how-to/make-tables.md).

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records={"class": "aqueous_solute", "elements": ["Ca"]})
print(results["species"].to_text("md"))
```

## Change any setting

```{code-cell} ipython3
setup = tf.tables.template("species-properties", database=AQ, records=["Ca+2"])
setup.set(T={"values": [25.0, 100.0], "unit": "degC"}, P={"values": [1.0], "unit": "bar"})
setup.column("G", unit="J/mol", digits=1)
setup.remove_column("V")
setup.add_column("A", unit="kJ/mol", digits=2)
print(setup.run()["species"].to_text("md"))
```

## Combine templates

Give a list of templates. Here the three templates bring:

- `species-properties`: the table, with the columns G, H, S, Cp and V of the records;
- `batch-saturation`: the conditions, 25 to 300 degC at the saturation pressure of water;
- `output-publication`: the units and the digits of a publication, kJ/mol with 2 decimals.

A later template replaces what an earlier one says. Your settings come last.

```{code-cell} ipython3
results = tf.table(template=["species-properties", "batch-saturation", "output-publication"], database=AQ, records=["Ca+2"])
print(results["species"].to_text("md"))
```

## Keep your setup in a file

The file has all the settings. It records how you made the table, and it runs without the templates.

```{code-cell} ipython3
setup = tf.tables.template(["species-properties", "batch-reference"], database=AQ, records=["Ca+2"])
setup.save("output/my-setup.yaml")
print(pathlib.Path("output/my-setup.yaml").read_text())
print(tf.table("output/my-setup.yaml")["species"].to_text("md"))
```

A setup in a file can also start from templates, with the key `template`. A table with the name of a table of the template changes that table:

```yaml
template: [species-properties, batch-saturation]
database: aq17-thermofun.json
records: {class: aqueous_solute, elements: [Ca]}
tables:
  - name: species
    layout: transposed
```

Run it with `tf.table("my-setup.yaml")`, or from a shell with `python -m thermofun.tables my-setup.yaml`.

(make-your-own-template)=
## Make your own template

A template is a YAML file. It has the same keys as a setup, and you can give as many or as few as you like: only the columns, only the conditions, or a whole table. Its first line is a comment that says what it makes. Put it in a folder, and name the folder in `THERMOFUN_TEMPLATE_PATH`. To use it always, set the variable in your shell before you start Python. Here is a template for G and S of any record at two temperatures.

```{code-cell} ipython3
pathlib.Path("output/my-templates").mkdir(exist_ok=True)
pathlib.Path("output/my-templates/our-table.yaml").write_text('''# G and S at 25 and 50 degC, for our group
conditions:
  T: {values: [25, 50], unit: degC}
  P: {values: [1], unit: bar}
tables:
  - name: ours
    columns:
      - {property: G, unit: kJ/mol, digits: 2}
      - {property: S, digits: 2}
''')

os.environ["THERMOFUN_TEMPLATE_PATH"] = "output/my-templates"
print(tf.tables.templates()["our-table"])
print(tf.tables.template_source("our-table"), "|", tf.tables.template_source("logk-grid"))
print(tf.table(template="our-table", database=AQ, records=["Ca+2"])["ours"].to_text("md"))
del os.environ["THERMOFUN_TEMPLATE_PATH"]
```

A file with the name of a template of the library replaces that template. `template_source` says which one you have.

**Next:** [6. Output tables](06-output-tables.md).
