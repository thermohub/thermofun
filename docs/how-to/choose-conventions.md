<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Choose conventions

**You need:** ThermoFun. The reasons are in [Conventions](../explanation/conventions.md).

There are two questions: what the data are in, and what you want the results in.

## What the data are in

Write `convention` in a record, or in `thermodataset` for all records. [Add data](add-data.md) lists the words. The engine converts the values to its canonical convention.

## What you want the results in

Set the conventions of the engine.

```python
import thermofun as tf

engine = tf.ThermoEngine("examples/v2/minimal.yaml")
geochemical = engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@")

conventions = tf.EngineConventions()
conventions.water_reference = "steam_tables"
engine.setConventions(conventions)
steam = engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@")
```

**Check:** on the steam tables, G = H - T S, and S is 0 at the triple point of water.

```python
assert abs(steam.gibbs_energy.val - (steam.enthalpy.val - 298.15 * steam.entropy.val)) < 2.0   # J/mol
assert abs(engine.thermoPropertiesSubstance(273.16, 611.655, "H2O@").entropy.val) < 0.1        # J/(mol K)
assert steam.gibbs_energy.val != geochemical.gibbs_energy.val
```

A change applies from the next calculation.
