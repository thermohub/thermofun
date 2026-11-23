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

# 6. Batch templates

This page is the Jupyter notebook [`06-batch-templates.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/06-batch-templates.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn to run a batch calculation from a template, to combine templates, and to make your own.

**You need:** ThermoFun, pandas, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

**Related:** [Batch calculations](05-batch-calculations.md) · [Run a batch calculation](../how-to/make-tables.md) (all keys) · [Batch setups](../reference/batch.md) · [Units](../reference/units.md)

A **batch template** is a file with some blocks of a batch setup ([tutorial 5](05-batch-calculations.md)). Most have the conditions and a result with its properties: you give the database and your records. Some are **fragments**, with one block only: `conditions-*` give T and P, and `output-*` the units and the digits; you combine them with another template.

```{code-cell} ipython3
import os, pathlib
import thermofun as tf
import pandas as pd

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)
```

## Which templates are there

`tf.batch.catalog()` lists them: what each makes (the first line of its file), the blocks it gives, and where it comes from.
A template without `results` is a fragment.

```{code-cell} ipython3
pd.DataFrame(tf.batch.catalog())
```

## See what a template says

`tf.template` gives the setup of a template; it prints as YAML.

```{code-cell} ipython3
print(tf.template("species-properties"))
```

## Use a template with your records

`tf.batch` takes a template, the database and the keys to fill: `substances`, `reactions` and `properties` go into each result, `T` and `P` into the conditions, and the keys of `output` into the output.

```{code-cell} ipython3
results = tf.batch(template="species-properties", database=AQ, substances=["Ca+2", "Calcite"])
print(results["species"].to_text("md"))
```

```{code-cell} ipython3
results = tf.batch(template="logk-saturation", database=AQ, reactions=["Calcite = Ca+2 + CO3-2"])
print(results["logk"].to_text("md"))
```

Instead of a list of symbols, give a rule. The records that fit the rule are the rows. This rule takes all aqueous solutes that have the element Ca; the rules are in [Run a batch calculation](../how-to/make-tables.md).

```{code-cell} ipython3
results = tf.batch(template="species-properties", database=AQ, substances={"class": "aqueous_solute", "elements": ["Ca"]}, T={"values": [25, 100], "unit": "degC"})
print(results["species"].to_text("md"))
```

## Change any setting

```{code-cell} ipython3
setup = tf.template("species-properties", database=AQ, substances=["Ca+2"])
setup.set("/conditions/T", {"values": [25.0, 100.0], "unit": "degC"})
setup.set("/conditions/P", {"values": [1.0], "unit": "bar"})
setup.property("G", unit="J/mol", digits=1)
setup.remove_property("V")
setup.add_property("A", unit="kJ/mol", digits=2)
print(setup.run()["species"].to_text("md"))
```

## Combine templates

Give a list of templates. Here the three templates bring:

- `species-properties`: the result, with G, H, S, Cp and V of the records;
- `conditions-grid`: the conditions, 25 to 500 degC and 1 to 5000 bar;
- `output-publication`: the units and the digits of a publication, kJ/mol with 2 decimals.

A later template replaces what an earlier one says. Your settings come last. Here the grid is cut to two temperatures:

```{code-cell} ipython3
results = tf.batch(template=["species-properties", "conditions-grid", "output-publication"], database=AQ, substances=["Calcite"],
                   T={"values": [25, 300], "unit": "degC"})
print(results["species"].to_text("md"))
```

## Compare models and databases

`model-comparison` has two dielectric models of water in `compare`, and the difference between them. `database-comparison` has two databases: give the second one by its path.

```{code-cell} ipython3
results = tf.batch(template="model-comparison", database=AQ, substances=["Na+"], T={"values": [25, 150, 300], "unit": "degC"})
print(results["comparison"].to_text("md"))
```

```{code-cell} ipython3
setup = tf.template("database-comparison", database=AQ, substances=["Quartz"], T={"values": [25, 300], "unit": "degC"})
setup.set("/results/0/compare/1/database", AQ)
print(setup.run()["databases"].to_text("md"))
```

The same database twice gives a difference of 0: put your second database there.

## A setup that starts from templates

A setup in a file can start from templates, with the key `template`. A result with the name of a result of the template changes that result:

```{code-cell} ipython3
pathlib.Path("output/from-templates.yaml").write_text('''template: [species-properties, conditions-reference]
database: ../aq17-thermofun.json
results:
  - name: species
    substances: [Ca+2, Calcite]
    output: {layout: wide}
''')
print(tf.batch("output/from-templates.yaml")["species"].to_text("md"))
```

(make-your-own-template)=
## Make your own template

A template is a YAML file with any of the blocks of a setup. Its first line is a comment that says what it makes. Put it in a folder, and name the folder in `THERMOFUN_TEMPLATE_PATH`. To use it always, set the variable in your shell before you start Python. Here is a template for G and S of any record at two temperatures.

```{code-cell} ipython3
pathlib.Path("output/my-templates").mkdir(exist_ok=True)
pathlib.Path("output/my-templates/our-table.yaml").write_text('''# G and S at 25 and 50 degC, for our group
conditions:
  T: {values: [25, 50], unit: degC}
  P: {values: [1], unit: bar}
results:
  - name: ours
    properties:
      - {name: G, unit: kJ/mol, digits: 2}
      - {name: S, digits: 2}
''')

os.environ["THERMOFUN_TEMPLATE_PATH"] = "output/my-templates"
print(tf.batch.catalog("our-table"))
print(tf.batch.catalog("logk-grid")["source"])
print(tf.batch(template="our-table", database=AQ, substances=["Ca+2"])["ours"].to_text("md"))
del os.environ["THERMOFUN_TEMPLATE_PATH"]
```

A file with the name of a template of the library replaces that template. The `source` of the catalog says which one you have.

**Next:** [7. Fit parameters](07-fit-parameters.md).
