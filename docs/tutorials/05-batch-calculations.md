<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Batch calculations

You learn to calculate many T and P values, with your units, and to write a table. It takes 15 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

## 1. Make a batch

A batch uses the engine and its options. Its default units are K, Pa and J/mol.

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
batch = tf.ThermoBatch(engine)
```

## 2. Choose units and digits

```python
batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
batch.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
```

Your T and P values are now in degC and bar. The results in a CSV file are in your units. A list from `to2DVectorDouble` is always in the units of the engine: J/mol, not kJ/mol.

## 3. Calculate a list of T and P

```python
table = batch.thermoPropertiesSubstance([[25.0, 1.0], [100.0, 100.0]], ["Na+", "Cl-"], ["gibbs_energy", "entropy"])
rows = table.to2DVectorDouble()
assert len(rows) == 4
print(rows)   # one row for each substance at each T and P: G in J/mol, S in J/(mol K)
assert abs(rows[0][0] - engine.thermoPropertiesSubstance(298.15, 1e5, "Na+").gibbs_energy.val) < 1e-6
```

## 4. Write a CSV file

```python
table.toCSV("results.csv")
with open("results.csv") as f:
    lines = f.read().splitlines()
assert "kJ/mol" in lines[0]                                     # G in the unit you chose
assert len(lines) == 5                                          # the header, and one line for each row
```

## 5. Calculate a grid

Give the first value, the last value and the step, in your units.

```python
batch.setTemperatureIncrement(25.0, 75.0, 25.0)
batch.setPressureIncrement(1.0, 1.0, 0.0)
grid = batch.thermoPropertiesReaction(["Calcite = Ca+2 + CO3-2"], ["logKr"]).to2DVectorDouble()
assert len(grid) == 3
```

The other options are in [Batch options](../reference/batch.md).
