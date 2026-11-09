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

You learn to make a table from a template, to change it, and to make your own template. It takes 15 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A template is a ready setup of a table. You give your records, and the template has the rest: conditions, columns, units and digits.

```{code-cell} ipython3
import os, pathlib
import thermofun as tf

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)
```

## Which templates are there

The templates `batch-*` and `output-*` have no tables. They give settings to one of the others.

```{code-cell} ipython3
for name, what in tf.tables.templates().items():
    print(f"{name:22} {what}")
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

Records can come from a rule too:

```{code-cell} ipython3
results = tf.table(template="species-properties", database=AQ, records={"class": "aqueous_solute", "elements": ["Ca"]})
print(sorted({row[0] for row in results["species"].rows}))
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

## Make your own template

A template is a YAML file. Its first line is a comment that says what it makes. Put it in a folder, and name the folder in `THERMOFUN_TEMPLATE_PATH`. To use it always, set the variable in your shell before you start Python.

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
