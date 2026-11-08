<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Your first calculation

You learn to load a database, calculate one substance and one reaction, and read a result. It takes 15 minutes.

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

## 1. Load the database

```python
import thermofun as tf

database = tf.Database("aq17-thermofun.json")
assert database.containsSubstance("Calcite")
assert database.numberOfSubstances() > 100
```

## 2. Calculate a substance

Make an engine. Give T in K, P in Pa and the symbol.

```python
engine = tf.ThermoEngine(database)
calcite = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(calcite.gibbs_energy.val, "J/mol")
print(calcite.volume.val, "J/bar")
assert -1.2e6 < calcite.gibbs_energy.val < -1.1e6
```

## 3. Read a result

Each property has a value, two derivatives, an uncertainty and a status.

```python
G = calcite.gibbs_energy
print(G.val, G.ddt, G.ddp, G.err, G.sta)
assert abs(G.ddt + calcite.entropy.val) < 1e-6                # dG/dT = -S
assert abs(G.ddp * 1e5 - calcite.volume.val) < 1e-6           # dG/dP = V, with V in J/bar
assert G.sta[0] == tf.Status.calculated
```

Always look at the status. If it is `notdefined`, a value that the calculation needs is missing, and `val` has no meaning.

## 4. Calculate a reaction

Give an equation, or the symbol of a reaction of the database.

```python
reaction = engine.thermoPropertiesReaction(298.15, 1e5, "Calcite = Ca+2 + CO3-2")
print("log K", reaction.log_equilibrium_constant.val)
assert -9.0 < reaction.log_equilibrium_constant.val < -8.0
```

## 5. Use the saturation pressure

A pressure of 0 means the saturation pressure of water at this T. It exists only below 647.096 K, the critical temperature of water; above it, give a pressure.

```python
at_saturation = engine.propertiesSolvent(473.15, 0.0, "H2O@")
assert 860.0 < at_saturation.density.val < 870.0              # kg/m3
```

## 6. The same in C++

The names are the same. See `examples/cmake-project/` for the project files.

```cpp
ThermoFun::ThermoEngine engine("aq17-thermofun.json");
double P = 1e7;
auto calcite = engine.thermoPropertiesSubstance(473.15, P, "Calcite");
std::cout << calcite.gibbs_energy.val << std::endl;
```

**Next:** [Your own data](02-your-own-data.md).
