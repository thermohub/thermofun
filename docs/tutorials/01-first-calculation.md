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

# 1. Your first calculation

You learn to calculate a substance, a reaction and water, and to read a result. It takes 15 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

## Make an engine

A `ThermoEngine` calculates from the records of a database. The engine takes **T in K and P in Pa**.

```{code-cell} ipython3
import thermofun as tf
import pandas as pd

engine = tf.ThermoEngine("aq17-thermofun.json")
```

## Calculate a substance

```{code-cell} ipython3
calcite = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print("G  ", calcite.gibbs_energy.val, "J/mol")
print("H  ", calcite.enthalpy.val, "J/mol")
print("S  ", calcite.entropy.val, "J/(K mol)")
print("Cp ", calcite.heat_capacity_cp.val, "J/(K mol)")
print("V  ", calcite.volume.val, "m3/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert -1.2e6 < calcite.gibbs_energy.val < -1.1e6
```

## Read a result

Each property has a value, its derivatives by T and P, its uncertainty and a status.

```{code-cell} ipython3
G = calcite.gibbs_energy
print("value", G.val, " dG/dT", G.ddt, " dG/dP", G.ddp, " uncertainty", G.err, " status", G.sta)
```

The derivatives agree with the other properties: dG/dT = -S, and dG/dP = V with V in m3/mol and P in Pa.

```{code-cell} ipython3
print(G.ddt + calcite.entropy.val, G.ddp - calcite.volume.val)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(G.ddt + calcite.entropy.val) < 1e-6
assert abs(G.ddp - calcite.volume.val) < 1e-11
assert G.sta[0] == tf.Status.calculated
```

Always look at the status. If it is `notdefined`, a value that the calculation needs is missing, and `val` has no meaning.

## Use the saturation pressure

A pressure of 0 means the saturation pressure of water at this T. It exists only below 647.096 K, the critical temperature of water. Above it, give a pressure.

```{code-cell} ipython3
print(engine.thermoPropertiesSubstance(473.15, 0.0, "Ca+2").gibbs_energy.val, "J/mol, at the saturation pressure")
print(engine.thermoPropertiesSubstance(473.15, 1554939.22, "Ca+2").gibbs_energy.val, "J/mol, at 15.55 bar: the saturation pressure at 200 degC")
```

## Calculate a reaction

Write an equation, or give the symbol of a reaction of the database.

```{code-cell} ipython3
dissolution = "Calcite = Ca+2 + CO3-2"
reaction = engine.thermoPropertiesReaction(298.15, 1e5, dissolution)
print("log K", reaction.log_equilibrium_constant.val)
print("dG   ", reaction.reaction_gibbs_energy.val, "J/mol")
print("dH   ", reaction.reaction_enthalpy.val, "J/mol")
print("dV   ", reaction.reaction_volume.val, "m3/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert -9.0 < reaction.log_equilibrium_constant.val < -8.0
```

## Calculate water

```{code-cell} ipython3
water = engine.propertiesSolvent(298.15, 1e5, "H2O@")
electro = engine.electroPropertiesSolvent(298.15, 1e5, "H2O@")
print("density             ", water.density.val, "kg/m3")
print("dielectric constant ", electro.epsilon.val)
print("Born Q              ", electro.bornQ.val, "1/Pa")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert 997.0 < water.density.val < 997.1
assert 860.0 < engine.propertiesSolvent(473.15, 0.0, "H2O@").density.val < 870.0
```

## Calculate many values with a loop

For a few values, a loop is enough. For many, [batch calculations](07-batch-calculations.md) and [output tables](06-output-tables.md) are shorter.

```{code-cell} ipython3
rows = []
for t in [25, 50, 100, 150, 200]:
    result = engine.thermoPropertiesReaction(t + 273.15, 0.0, dissolution)
    rows.append({"T (degC)": t, "log K": result.log_equilibrium_constant.val})
pd.DataFrame(rows)
```

## Add an uncertainty

A value in a record can have an uncertainty, and the engine propagates it. aq17 has none for Calcite, so add them to the record. [Tutorial 3](03-change-models.md) shows this way to change a record.

```{code-cell} ipython3
import json

records = json.loads(tf.Database("aq17-thermofun.json").toJson(False))
properties = records["substances"]["Calcite"]["standard_state_properties"]
properties["G"] = {"value": properties["G"], "uncertainty": 1500.0}
properties["S"] = {"value": properties["S"], "uncertainty": 0.5}

uncertain = tf.ThermoEngine(tf.Database(json.dumps(records)))
G = uncertain.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print(G.val, "+-", G.err, "J/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert G.err > 1500.0
```

## See how a result is calculated

`calculationPath` shows the methods of a record, and what else the record needs. `explain` gives the state after each method.

```{code-cell} ipython3
path = engine.calculationPath("Ca+2")
print(path["symbol"], path["methods"])
for step in path["steps"]:
    print(" needs", step["symbol"], step["methods"], "as", step["role"])
```

```{code-cell} ipython3
pd.DataFrame(engine.explain(473.15, 1e7, "Calcite"))[["method", "G", "H", "S", "Cp", "V"]]
```

## The same in C++

The names are the same. See `examples/cmake-project/` of the repository for the project files.

```cpp
ThermoFun::ThermoEngine engine("aq17-thermofun.json");
double P = 1e7;
auto calcite = engine.thermoPropertiesSubstance(473.15, P, "Calcite");
std::cout << calcite.gibbs_energy.val << std::endl;
```

**Next:** [2. Load data](02-load-data.md).
