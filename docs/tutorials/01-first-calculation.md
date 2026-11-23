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

This page is the Jupyter notebook [`01-first-calculation.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/01-first-calculation.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn to calculate a substance, a reaction and water, and to read a result.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

**Related:** [Results](../reference/results.md) · [The v2 format](../reference/format-v2.md) (the paths of a record) (what a calculation gives) · [Units](../reference/units.md) · [Methods](../reference/methods/index.md) · [Uncertainties](../reference/uncertainty.md) · [See how a result is calculated](../how-to/see-how-a-result-is-calculated.md) · [Glossary](../glossary.md)

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

Each property is not one number. It has five parts:

| Part | Name | What it is |
|---|---|---|
| value | `val` | the property, here G in J/mol |
| derivative by T | `ddt` | dG/dT, in J/(mol K) |
| derivative by P | `ddp` | dG/dP, in J/(mol Pa) |
| uncertainty | `err` | the uncertainty of the value, in the units of the value; 0 if the record has none |
| status | `sta` | whether the value was calculated |

```{code-cell} ipython3
G = calcite.gibbs_energy
print("value         ", G.val, "J/mol")
print("dG/dT         ", G.ddt, "J/(mol K)")
print("dG/dP         ", G.ddp, "J/(mol Pa)")
print("uncertainty   ", G.err, "J/mol")
print("status        ", G.sta)
```

The uncertainty is 0 because the record of Calcite in aq17 has none. See [Add an uncertainty](#add-an-uncertainty) below.

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

Always look at the status. If it is `notdefined`, a value that the calculation needs is missing, and `val` has no meaning. If `G.info.validity` is `extrapolated`, a model was used outside its range: the value is there, and `G.info.notes` say which range (see [Results](../reference/results.md#info)).

## Use the saturation pressure

A pressure of 0 means the saturation pressure of water at this T. It exists only below 647.096 K, the critical temperature of water. Above it, give a pressure.

```{code-cell} ipython3
print(engine.thermoPropertiesSubstance(473.15, 0.0, "Ca+2").gibbs_energy.val, "J/mol, at the saturation pressure")
print(engine.thermoPropertiesSubstance(473.15, 1554939.22, "Ca+2").gibbs_energy.val, "J/mol, at 15.55 bar: the saturation pressure at 200 degC")
```

## Calculate a reaction

Write an equation with the symbols of the substances of the database, reactants on the left and products on the right, or give the symbol of a reaction of the database.

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

## Calculate water solvent properties

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

## Calculate many values

To calculate many values at once, run a batch calculation. The batch template `reaction-properties` gives log K and the other properties of a reaction at many temperatures. A template is a YAML file: copy it and edit it to change the properties, the units, the temperatures and pressures, or the layout of the result, as [Batch calculations](05-batch-calculations.md) shows.

```{code-cell} ipython3
results = tf.batch(template="reaction-properties", database="aq17-thermofun.json", reactions=[dissolution])
print(results["reactions"].to_text("md"))
```

(add-an-uncertainty)=
## Add an uncertainty

A value in a record can have an uncertainty, and the engine propagates it to every result. aq17 has none for Calcite, so add them to the record. A value of a record is found by its **path**: the section (`substances`), the symbol of the record, and the keys of the record, separated by `/`. `get` reads a value, `set` changes it. [Tutorial 3](03-change-models.md) shows more changes.

For G, the value and its uncertainty are given together. For S, only the uncertainty: the path ends with `/uncertainty`, and the value stays as it is.

```{code-cell} ipython3
database = tf.Database("aq17-thermofun.json")

G0 = database.get("/substances/Calcite/standard_state_properties/G")
database.set("/substances/Calcite/standard_state_properties/G", {"value": G0, "uncertainty": 1500.0})
database.set("/substances/Calcite/standard_state_properties/S/uncertainty", 15.0)

calcite_engine = tf.ThermoEngine(database)
G = calcite_engine.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print(G.val, "+-", G.err, "J/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert G.err >= 1500.0
```

`set` changes the records in memory, not the file `aq17-thermofun.json`. An engine copies the records when it is made: make it after the changes. An engine made before them does not see them.

## See how a result is calculated

`engine.route(symbol)` shows how a record is calculated, before calculating: by what (`by`), with which methods, and which other records it needs (`needs`). It prints as YAML:

```{code-cell} ipython3
print(engine.route("Ca+2"))
```

Every result carries the same route with its numbers, `result.info.route`: the point, and what each method contributes to G, H, S, Cp and V. The contributions add up to the result. Calcite has a base method and a method for the volume:

```{code-cell} ipython3
calcite = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(calcite.info.route)
```

The route of a record at many points, as a table: [See how a result is calculated](../how-to/see-how-a-result-is-calculated.md).

## The same in C++

The names are the same. See `examples/cmake-project/` of the repository for the project files.

```cpp
ThermoFun::ThermoEngine engine("aq17-thermofun.json");
double P = 1e7;
auto calcite = engine.thermoPropertiesSubstance(473.15, P, "Calcite");
std::cout << calcite.gibbs_energy.val << std::endl;
```

**Next:** [2. Load data](02-load-data.md).
