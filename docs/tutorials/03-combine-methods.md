<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Combine methods

You learn that a record can have several methods, and that their order in the file does not count. It takes 15 minutes.

**You need:** ThermoFun, and the file `examples/v2/mineral-methods.yaml` of the repository.

## 1. Read the record

Calcite has three methods:

- `CpPolynomial` is the base method. It gives G, H, S and Cp from the heat capacity.
- `LandauHP98` is a correction. It adds a phase transition.
- `MurnaghanHP98` is a correction. It gives the volume and the term of the pressure.

## 2. See what each method gives

```python
import thermofun as tf

engine = tf.ThermoEngine("examples/v2/mineral-methods.yaml")
steps = engine.explain(473.15, 1e7, "Calcite")
assert [s["method"] for s in steps] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
for step in steps:
    print(step["method"], step["G"], step["V"])
result = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
assert abs(steps[-1]["G"] - result.gibbs_energy.val) < 1e-6
```

Each row is the state after that method, in SI units: J/mol and m3/mol.

## 3. Change the order

The engine sets the order from what each method does. The order in the file gives the same result.

```python
import json

document = json.loads(tf.Database("examples/v2/mineral-methods.yaml").toJson(readable=False))
document["substances"]["Calcite"]["methods"].reverse()
reversed_engine = tf.ThermoEngine(tf.Database(json.dumps(document)))
same = reversed_engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
assert same.gibbs_energy.val == result.gibbs_energy.val
```

## 4. Remove a method

Without the transition, G is different.

```python
document["substances"]["Calcite"]["methods"] = [m for m in document["substances"]["Calcite"]["methods"] if m["method"] != "LandauHP98"]
without = tf.ThermoEngine(tf.Database(json.dumps(document))).thermoPropertiesSubstance(473.15, 1e7, "Calcite")
assert without.gibbs_energy.val != result.gibbs_energy.val
```

**Next:** [Fit parameters](04-fit-parameters.md). The reasons are in [How methods combine](../explanation/how-methods-combine.md).
