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

You learn where ThermoFun finds databases, the constants of the models and the templates, and how to see which data a calculation used. It takes 10 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

| What | Where | How to change it |
|---|---|---|
| Your data | Any file. You give the path to `tf.Database`. | Edit the file, or `appendData` another file. |
| Databases of ThermoFun | `Resources/databases/` of the [ThermoFun repository](https://github.com/thermohub/thermofun/tree/v2/Resources/databases) | |
| Constants of the models | Built into the library. A copy to read is in `include/ThermoFun/Methods/ParameterSets/` of your installation. | A value in a record, or a file in a folder of `THERMOFUN_PARAMETER_PATH`. |
| Templates of tables | Built into the library. A copy to read is in `include/ThermoFun/Batch/Templates/` of your installation. | A file in a folder of `THERMOFUN_TEMPLATE_PATH`. |
| Log | `thermofun.log` in the current folder. | |

The library does not read the copies under `include` when it runs, so a change to them changes nothing.

```{code-cell} ipython3
import os
import thermofun as tf
```

## Constants of the models

Many models have constants that belong to the model and not to one substance, such as the coefficients of a water equation. Each set has its file, with a description and its source. The catalog says which files a method reads.

```{code-cell} ipython3
for method in tf.methods.catalog():
    if method["constant_sets"]:
        print(method["id"], "->", [s["symbol"] for s in method["constant_sets"]])
```

```{code-cell} ipython3
hk = next(m for m in tf.methods.catalog() if m["id"] == "HelgesonKirkham74")
s = hk["constant_sets"][0]
print(s["symbol"], "\n\n", s["description"], "\n\n", s["reference"], "\n\n", s["formula_units"])
```

## Physical constants

The library is built with one set: `codata1986` by default. To use `codata2018`, build ThermoFun with the CMake option `-DTHERMOFUN_PHYSICAL_CONSTANTS=codata2018`.

```{code-cell} ipython3
tf.methods.physicalConstants()
```

## Your own folders

Two environment variables name your folders. Separate several folders with `:`, or with `;` on Windows.

- `THERMOFUN_PARAMETER_PATH`: variants of the constants of a model. A record uses a variant with `parameter_set`. A file here changes nothing until a record names it.
- `THERMOFUN_TEMPLATE_PATH`: your templates. See [5. Templates](05-templates.md).

```{code-cell} ipython3
print("parameters:", os.environ.get("THERMOFUN_PARAMETER_PATH"))
print("templates: ", os.environ.get("THERMOFUN_TEMPLATE_PATH"), tf.tables.template_folders())
```

## Which data did a calculation use

The snapshot of an engine names the version, the data, and the models of water. The SHA-256 changes when any value of the records changes.

```{code-cell} ipython3
engine = tf.ThermoEngine("aq17-thermofun.json")
for line in engine.snapshot().lines():
    print(line)
```

**Next:** [5. Templates](05-templates.md).
