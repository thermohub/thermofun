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

# 4. Where the files are

This page is the Jupyter notebook [`04-where-the-files-are.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/04-where-the-files-are.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn where ThermoFun finds databases, the model parameters and the templates, and how to see which data a calculation used.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

**Related:** [Add data](../how-to/add-data.md) · [Methods](../reference/methods/index.md) · [Units](../reference/units.md) · [Run a batch calculation](../how-to/make-tables.md) · [Glossary](../glossary.md)

| What | Where | How to change it |
|---|---|---|
| Your data | Any file. You give the path to `tf.Database`. | Edit the file, or `appendData` another file. |
| Databases of ThermoFun | `Resources/databases/` of the [ThermoFun repository](https://github.com/thermohub/thermofun/tree/v2/Resources/databases) | |
| Model parameters | Built into the library. A copy to read is in `include/ThermoFun/Methods/ParameterSets/` of your installation. | A value in a record, or a file in a folder of `THERMOFUN_PARAMETER_PATH`. |
| Templates of tables | Built into the library. A copy to read is in `include/ThermoFun/Batch/Templates/` of your installation. | A file in a folder of `THERMOFUN_TEMPLATE_PATH`. |
| Log | `thermofun.log` in the current folder. | |

The library does not read the copies under `include` when it runs, so a change to them changes nothing.

```{code-cell} ipython3
import os
import thermofun as tf
```

## Model parameters

Many models have model parameters: values that belong to the model and not to one substance, such as the coefficients of a water equation. They are not parameters of a record. Each set has its file, with a description and its source. The catalog entry of a method says which sets it reads.

```{code-cell} ipython3
hk = tf.methods.catalog("HelgesonKirkham74")
s = hk["model_parameter_sets"][0]
print(s["symbol"], "\n\n", s["description"], "\n\n", "; ".join(s["data_sources"]), "\n\n", s["formula_units"])
```

## Physical constants

The constants of physics are an option of the engine: `codata1986` (the default: most stored G and log K values were derived
with its R) or `codata2018`. The R of the option is the R of the calculation: G and log K of a reaction, the term RT ln(P/p°)
of a gas, the conversions of conventions, and the cubic equations of state of gases (Peng-Robinson, PRSV, Soave-Redlich-Kwong), in
which R is a physical constant. Fitted equations, such as CORK, Sterner-Pitzer, Churakov-Gottschalk and NASA polynomials,
keep the R of their fit. A dataset can say which set its values were derived with (`physical_constants` in `thermodataset`);
an engine with another set gives a warning.

```{code-cell} ipython3
print(tf.methods.physicalConstantsNames())
print(tf.methods.physicalConstants("codata2018"))
```

```{code-cell} ipython3
engine = tf.ThermoEngine("aq17-thermofun.json")
engine.preferences.physical_constants = "codata2018"
print(engine.preferences.physical_constants)
```

## Your own folders

Two environment variables name your folders. Separate several folders with `:`, or with `;` on Windows.

- `THERMOFUN_PARAMETER_PATH`: variants of the model parameters of a method. A record uses a variant with `parameter_set`. A file here changes nothing until a record names it.
- `THERMOFUN_TEMPLATE_PATH`: your templates. See [6. Batch templates](06-batch-templates.md).

```{code-cell} ipython3
print("parameters:", os.environ.get("THERMOFUN_PARAMETER_PATH"))
print("templates: ", os.environ.get("THERMOFUN_TEMPLATE_PATH"), tf.batch.template_folders())
```

## Which data and settings a calculation used

The summary of an engine names the version, the constants of physics, the data, the models of water, the conventions, and the options that are not at their default. The SHA-256 changes when any value of the records changes. A batch calculation writes the same lines at the top of its files.

```{code-cell} ipython3
engine = tf.ThermoEngine("aq17-thermofun.json")
print(engine.summary())
```

**Next:** [5. Batch calculations](05-batch-calculations.md).
