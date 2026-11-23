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

# Use ThermoBatch, the 0.x batch API

**Related:** [Batch calculations](../tutorials/05-batch-calculations.md) · [Batch templates](../tutorials/06-batch-templates.md) · [Batch setups](../reference/batch.md) · [Migrate from the old API](migrate-from-the-old-api.md) · [Units](../reference/units.md)

`ThermoBatch` is the batch API of ThermoFun 0.x, kept so that programs and notebooks written for it keep working: you give the records, the points and the properties in code, and set the units and digits with functions. New features come only in batch setups (`tf.batch`, [tutorial 5](../tutorials/05-batch-calculations.md)), which also compare models and databases, give uncertainties, derivatives and the properties of water, and record how a result was made. A batch setup can give you a `ThermoBatch` (`setup.batch()`), or set one that you have (`setup.apply_to(batch)`), as below.

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

`to2DVectorDouble` gives one row for each record and condition, in the units of the engine and not in your units: J/mol, not kJ/mol. `values()` of a result of `tf.batch` follows your units.

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
print(batch.thermoPropertiesSubstance([[298.15, 1e5]], symbols, ["gibbs_energy"]).to2DVectorDouble())
```

## A batch from a template or a setup

`setup.batch()` gives a batch with the engine, units, digits and preferences of a result, and the records and points to calculate. The conditions can come from a template `conditions-*`.

```{code-cell} ipython3
setup = tf.template(["species-properties", "conditions-saturation"], database=AQ, substances={"class": "aqueous_solute", "elements": ["Ca"]}, digits={"P": 3})
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

`write` follows the layout of the setup, and starts the file with how it was made. In `points`, P = 0 is the saturation pressure, and the file has the pressure that was used. `output.digits` with `P` sets its decimals. The same setup also runs as a batch calculation, with `setup.run()`.

The setup does not need to be a template. This one could be a YAML file:

```{code-cell} ipython3
setup = tf.batch.load({
    "database": AQ,
    "conditions": {"T": {"from": 0, "to": 300, "step": 10, "unit": "degC"}, "P": {"values": ["saturation"], "unit": "bar"}},
    "output": {"provenance": "none"},
    "results": [{"name": "calcite", "reactions": ["Calcite = Ca+2 + CO3-2"], "properties": ["logK"]}],
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
tf.template(["species-properties", "conditions-reference", "output-publication"]).apply_to(batch)
print(batch.propertyUnits()["gibbs_energy"], batch.propertyUnits()["temperature"], batch.propertyDigits()["gibbs_energy"])

batch.preferences.writeProvenance = True
batch.thermoPropertiesSubstance([[25.0, 1.0]], ["Ca+2"], ["gibbs_energy"]).toCSV("output/mine.csv")
print(pathlib.Path("output/mine.csv").read_text())
```

## What needs tf.batch

A `ThermoBatch` has one engine, and writes the values of substances or of reactions. A setup with `compare`, with uncertainties or derivatives in its properties, or with the properties of water runs only with `tf.batch`. The message says so.

```{code-cell} ipython3
try:
    tf.template("water-properties", database=AQ).batch()
except tf.BatchSetupError as error:
    print(error)
```

## From ThermoBatch to a batch setup

| `ThermoBatch` | In a batch setup |
|---|---|
| `thermoPropertiesSubstance(points, symbols, properties)` | `conditions` (`points: pairs`), `results: [{substances, properties}]` |
| `thermoPropertiesReaction(...)` | `results: [{reactions, properties}]` |
| `toCSV` | `output: {layout: long, file: x.csv}` |
| `toCSVTransposed` | `output: {layout: wide}` |
| `toCSVPropertyGrid` | `output: {layout: grid, grid_across: P}` |
| `setPropertyUnit`, `setPropertyDigit` | `unit` and `digits` of a property, or `output: {units, digits}` (also `T`, `P`) |
| `setTemperatureIncrement`, `setPressureIncrement` | `conditions: {T: {from, to, step, unit}}` |
| `loopTemperatureThenPressure` | `conditions: {vary_first: P}` |
| `loopOverTPpairsFirst` | `output: {rows_by: substance}` |
| `separator`, `isFixed`, `isFloating`, `isScientific` | `output: {csv_separator, number_format}` |
| `writeNaNifNotDefinedValue` | `output: {undefined: NaN}`, or `undefined: number` |
| `substancePropertiesFromReaction`, `reactionPropertiesFromReactants` | `engine: {options: {property_source: reaction}}` (or `reactants`) |
| `writeProvenance` | `output: {provenance: header}` |

The other keys are in [Batch setups](../reference/batch.md).
