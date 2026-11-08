<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Units

## In a file

| Quantity | Default unit |
|---|---|
| G, H, U, A | J/mol |
| S, Cp, Cv | J/(mol K) |
| V | m3/mol |
| T | K |
| P | Pa |
| The molar mass of an element | g/mol |
| A parameter of a method | its SI unit, on the page of the method |

Write another unit with `{value: 1.8, unit: J/bar}`, with text `"1.8 J/bar"`, or with a `units` block. [Add data](../how-to/add-data.md) has examples. Unit names follow UCUM, for example `J/(mol.K)`, `cm3/mol` and `Cel`.

## In the results of the engine

SI, also V, in m3/mol (before 1.0: J/bar; 1 J/bar is 1e-5 m3/mol). So dG/dP is V. Results of `explain`, `parameterSensitivities` and `tf.methods` are in SI too. A `ThermoBatch` or a table gives V in m3/mol unless you ask for another unit, for example J/bar or cm3/mol.

## In the parameter files

The built-in constants are in the units of the formulas, and are not converted. `ThermoFun/Methods/ParameterSets/README.md` has a table for each model.

## Conversions

```python
import thermofun as tf

assert tf.convert_units(1.0, "kJ", "J") == 1000.0
assert tf.units_convertible("kJ", "J")
```
