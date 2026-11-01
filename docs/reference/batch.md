<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Batch options

| Method of `ThermoBatch` | What it does |
|---|---|
| `thermoPropertiesSubstance`, `thermoPropertiesReaction` | One T and P, a list of pairs, or a list of T and a list of P, for one or many symbols and properties. |
| `setPropertyUnit`, `setPropertyDigit`, `setPropertyUnitDigit`, `setPropertiesUnits`, `setPropertiesDigits`, `setUnits`, `setDigits` | The unit and the decimals of the output. Temperature and pressure units are also those of your input. |
| `setTemperatureIncrement(first, last, step)`, `setPressureIncrement(first, last, step)` | A grid of T and P. |
| `setSolventSymbol(symbol)` | The water record. |
| `setBatchPreferences(preferences)` | The options below. |

`tf.availablePropertiesSubstance()` and `tf.availablePropertiesReaction()` list the properties and their units.

| Method of `Output` | What it gives |
|---|---|
| `toCSV(file)`, `toCSVTransposed(file)`, `toCSVPropertyGrid(file)` | A CSV file, in the units that you set. |
| `to2DVectorDouble()` | A list of rows, in the units of the engine. |
| `toDouble()`, `toThermoScalar()` | The first value, in the units of the engine. |

| `BatchPreferences` | Default | What it does |
|---|---|---|
| `isFixed`, `isFloating`, `isScientific` | fixed | The number format of the CSV file. |
| `separator` | `,` | The CSV separator. |
| `fileName`, `solventFileName` | `tpresults.csv`, `tpSolventResults.csv` | The output files. |
| `outputSolventProperties` | false | Also writes the properties of the solvent. |
| `substancePropertiesFromReaction` | false | Calculates substances from their defining reaction. |
| `reactionPropertiesFromReactants` | false | Calculates reactions from their reactants. |
| `loopOverTPpairsFirst`, `loopTemperatureThenPressure` | true | The order of the loops. |
| `writeNaNifNotDefinedValue` | true | Writes `NaN` for a value that is not defined. |
