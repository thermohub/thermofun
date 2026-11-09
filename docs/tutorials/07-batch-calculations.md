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

# 7. Batch calculations

You learn to calculate many records at many temperatures and pressures with `ThermoBatch`, directly or from a template or a YAML setup. It takes 15 minutes.

**You need:** ThermoFun, matplotlib, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

```{code-cell} ipython3
import os, pathlib
import thermofun as tf
import matplotlib.pyplot as plt

AQ = "aq17-thermofun.json"
os.makedirs("output", exist_ok=True)
```

## A batch by hand

A batch uses an engine and its options. Its default units are K, Pa and J/mol. Set the units first. Then T and P are in your units too.

```{code-cell} ipython3
batch = tf.ThermoBatch(AQ)
batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
batch.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
batch.setPropertyDigit("entropy", 2)

output = batch.thermoPropertiesSubstance([[25.0, 1.0], [100.0, 100.0]], ["Ca+2", "Calcite"], ["gibbs_energy", "entropy"])
output.toCSV("output/batch.csv")
print(pathlib.Path("output/batch.csv").read_text())
```

## The results as numbers

`to2DVectorDouble` gives one row for each record and condition, in the units of the engine and not in your units: J/mol, not kJ/mol. `values()` of a table follows your units.

```{code-cell} ipython3
rows = output.to2DVectorDouble()
rows
```

```{code-cell} ipython3
:tags: [remove-cell]
assert len(rows) == 4
engine = tf.ThermoEngine(AQ)
assert abs(rows[0][0] - engine.thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy.val) < 1e-6
assert "kJ/mol" in pathlib.Path("output/batch.csv").read_text().splitlines()[0]
```

## A T-P grid

Give the first value, the last value and the step, in your units. `toCSVPropertyGrid` writes one file for each property, named `<property>_<file>`.

```{code-cell} ipython3
batch = tf.ThermoBatch(AQ)
batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
batch.setPropertyDigit("logKr", 3)
batch.setTemperatureIncrement(25.0, 300.0, 25.0)
batch.setPressureIncrement(500.0, 1500.0, 500.0)
output = batch.thermoPropertiesReaction(["Calcite = Ca+2 + CO3-2"], ["logKr"])

os.chdir("output")
output.toCSVPropertyGrid("grid.csv")
os.chdir("..")
print(pathlib.Path("output/logKr_grid.csv").read_text())
```

## Choose the records by a rule

```{code-cell} ipython3
batch = tf.ThermoBatch(AQ)
symbols = batch.selectSubstances({"class": "aqueous_solute", "elements": ["Ca"]})
print(symbols)
values = batch.thermoPropertiesSubstance([[298.15, 1e5]], symbols, ["gibbs_energy"]).to2DVectorDouble()
for symbol, row in zip(symbols, values):
    print(f"{symbol:10} {row[0]:12.1f} J/mol")
```

## A batch from a template or a setup

`setup.batch()` gives a batch with the engine, units, digits and preferences of a table, and the records and points to calculate. The conditions can come from a template `batch-*`.

```{code-cell} ipython3
setup = tf.tables.template(["species-properties", "batch-saturation"], database=AQ, records={"class": "aqueous_solute", "elements": ["Ca"]}, digits_P=3)
calculation = setup.batch("species")
print(calculation.symbols)
print(calculation.properties)
print(calculation.points)
```

```{code-cell} ipython3
output = calculation.run()     # the same Output as above
print(len(output.to2DVectorDouble()), "rows")
print(pathlib.Path(calculation.write("output/from-template.csv")).read_text()[:800])
```

`write` follows the layout of the setup, and starts the file with how it was made. In `points`, P = 0 is the saturation pressure, and the file has the pressure that was used. `digits_P` sets its decimals. The same setup can also give a table, with `setup.run()`.

The setup does not need to be a template. This one could be a YAML file:

```{code-cell} ipython3
setup = tf.tables.load({
    "database": AQ,
    "conditions": {"T": {"from": 0, "to": 300, "step": 10, "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "tables": [{"name": "calcite", "reactions": ["Calcite = Ca+2 + CO3-2"], "columns": [{"property": "logK"}], "provenance": "none"}],
})
calculation = setup.batch()
logK = [row[0] for row in calculation.run().to2DVectorDouble()]
T = [point[0] for point in calculation.points]

plt.plot(T, logK)
plt.xlabel("T (degC)")
plt.ylabel("log K of Calcite = Ca+2 + CO3-2")
plt.title("At the saturation pressure of water")
plt.show()
```

## A batch that you already have

`apply_to` sets the units, digits and preferences of a batch from a setup. A batch alone writes no record of how it was made unless you ask.

```{code-cell} ipython3
batch = tf.ThermoBatch(AQ)
tf.tables.template(["species-properties", "batch-reference", "output-publication"]).apply_to(batch)
print(batch.propertyUnits()["gibbs_energy"], batch.propertyUnits()["temperature"], batch.propertyDigits()["gibbs_energy"])

batch.preferences.writeProvenance = True
batch.thermoPropertiesSubstance([[25.0, 1.0]], ["Ca+2"], ["gibbs_energy"]).toCSV("output/mine.csv")
print(pathlib.Path("output/mine.csv").read_text())
```

## What needs a table

A batch has one engine, and writes the values of substances or of reactions. A setup with `compare`, with uncertainties or derivatives in its columns, or with the properties of water runs only as a table. The message says so.

```{code-cell} ipython3
try:
    tf.tables.template("water-properties", database=AQ).batch()
except tf.tables.SetupError as error:
    print(error)
```

## From ThermoBatch to tables

| `ThermoBatch` | In a setup |
|---|---|
| `toCSV` | `layout: long`, `output: x.csv` |
| `toCSVTransposed` | `layout: transposed` |
| `toCSVPropertyGrid` | `layout: grid`, with `across` |
| `setPropertyUnit`, `setPropertyDigit` | `unit` and `digits` of a column, or `units` and `digits` of the table |
| `setTemperatureIncrement`, `setPressureIncrement` | `from`, `to`, `step` in `conditions` |
| `separator`, `isFixed`, `isScientific` | `separator`, `format` |
| `writeProvenance` | `provenance` |

The other options are in [Batch options](../reference/batch.md), and the full list of settings is in [Make tables](../how-to/make-tables.md).

**Next:** [8. Fit parameters](08-fit-parameters.md).
