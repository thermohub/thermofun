<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Batch setups

**Related:** [Tutorial 5: Batch calculations](../tutorials/05-batch-calculations.md) · [Tutorial 6: Batch templates](../tutorials/06-batch-templates.md) · [Run a batch calculation](../how-to/make-tables.md) · [Use ThermoBatch](../how-to/use-thermobatch.md) · [Options of the engine](engine-preferences.md) · [Units](units.md)

A **batch calculation** calculates many records at many conditions at once. Its **setup** has four blocks, plus
`template`. A block at the top is the default of every result; a result can give its own `engine`, `conditions` or `output`,
which replace the defaults key by key (the groups of `engine` and the maps `units` and `digits` one level further).

| Block | Job |
|---|---|
| `database` + `engine` | with what to calculate |
| `conditions` | where: T and P |
| `results` | what: one entry for each result table |
| `output` | how to give the results; a file only if `output.file` is given |

A value that names a concept of the format uses the word of the format (`substances`, `property_source`, `standard_state`, ...).

## The keys

| Key | Values |
|---|---|
| `database` | A file, found next to the setup file. In Python, also a `Database` or an engine. |
| `template` | A name or a list of names: the templates go under the setup, a later one over an earlier one, and the setup over them. A result with the name of a result of a template changes that result. |
| `engine.options` | The [options of the engine](engine-preferences.md) by their names: `solvent_symbol`, `property_source`, `round_to_uncertainty`, `physical_constants`, `method_implementation`, ... |
| `engine.conventions` | The conventions of the results: `standard_state: {pressure, concentration_scale}`, `energy_reference: {elements, water}` ([Conventions](conventions.md)). |
| `engine.property_sources` | Where the properties of single records come from: `{Calcite: reaction, R1: reactants}`. |
| `conditions.T`, `conditions.P` | A list, `{values, unit}`, or `{from, to, step, unit}`. `saturation` in P is the saturation pressure of water: the result shows the pressure that the calculation used, and a value is asked for with the word `saturation`. |
| `conditions.points` | `grid` (the default: every T with every P) or `pairs` (the first T with the first P, and so on). |
| `conditions.vary_first` | The condition that changes from one row to the next: `P` (the default: the rows of one T together) or `T`. |
| `results[].name`, `description` | The name of the result table, and a text. |
| `results[].substances`, `reactions` | Symbols, a rule, or a list with rules in it ([Run a batch calculation](../how-to/make-tables.md#4-choose-records-by-a-rule)). A reaction can be an equation. |
| `results[].properties` | A name, or `{name, show, unit, digits, number_format, label}`. Substance: `G`, `H`, `S`, `Cp`, `Cv`, `V`, `U`, `A`. Reaction: `logK`, `lnK`, `dG`, `dH`, `dS`, `dCp`, `dCv`, `dV`, `dU`, `dA`. Water: `density`, `Alpha`, `Beta`, `dAldT`, `epsilon`, `bornZ`, `bornY`, `bornQ`, `bornX` and others. The route of a result ([See how a result is calculated](../how-to/see-how-a-result-is-calculated.md)): `contribution:<method>:<property>`, `terms:<method>:<name>`, `variables:<method>:<name>` (value only). |
| `show` | `value` (the default), `uncertainty`, `ddt`, `ddp`, `status`. |
| `results[].compare` | A list: `{label, database, replace, engine}`. Each entry gives its own columns: another database, other methods for some records (`replace: {H2O@: {JohnsonNorton91: HelgesonKirkham74}}`), or another engine block. |
| `results[].compare_difference` | `true`: for each property, a column with the difference of each entry to the first one. |
| `output.file`, `file_format` | The file. Its extension says the format, `csv`, `md` or `tex`, unless `file_format` does. |
| `output.number_format`, `digits` | `floating` (the default), `fixed`, `scientific`, and the decimals: one number, or for each property, `T` and `P`: `{G: 2, S: 3, T: 1}`. |
| `output.units` | The unit of a property whose entry gives none, of `T` and of `P`: `{G: kJ/mol, T: K}`. |
| `output.layout` | `long` (a row for each record and condition), `wide` (a column for each record), `grid` (a grid for each property). |
| `output.grid_across` | For `grid`: `P` (the default) or `T` across. |
| `output.rows_by` | For `long`: `substance` (the default: the rows of one record together) or `condition`. |
| `output.csv_separator` | Of a CSV file. |
| `output.undefined` | The text for a value that is not defined (`NaN`), or `number`: the number the calculation gave. |
| `output.provenance` | `header` (the default: the lines first in the file), `file` (`<file>.provenance.yaml` with the lines and the whole setup), `none`. |

A key that a block does not have is an error that names the nearest key and lists the keys. A placeholder `<...>` left in a
setup (the template `blank` has them) is an error that names its paths.

## Batch templates

`tf.batch.catalog()` lists them (a row each: `name`, `summary`, `blocks`, `source`; with a name, one row), and
`python -m thermofun.batch --list` too; `tf.template(name)` gives the setup of one, which prints as
YAML. Folders of your own go in `THERMOFUN_TEMPLATE_PATH`; a file there with the name of a template of the library replaces it.

| Template | What it makes | You give |
|---|---|---|
| `blank` | The smallest setup, with a `<...>` for each value to give | every `<...>` |
| `species-properties` | G, H, S, Cp and V | `substances` |
| `species-grid` | G on a grid: T down and P across | `substances` |
| `reaction-properties` | log K, dG, dH, dS and dCp | `reactions` |
| `logk-grid` | log K on a grid: T down and P across | `reactions` |
| `logk-saturation` | log K from 0 to 300 °C at the saturation pressure of water | `reactions` |
| `water-properties` | density, dielectric constant and Born functions of water | nothing |
| `uncertainties` | G and S with their uncertainties | `substances` |
| `derivatives` | G with its derivatives by T and P | `substances` |
| `model-comparison` | V and Cp with two dielectric models, and their difference | `substances` |
| `database-comparison` | G from two databases, and their difference | `substances`, the second database |

Fragments, with one block only, to combine with a template above:

| Template | What it sets |
|---|---|
| `conditions-reference` | 25 °C and 1 bar |
| `conditions-saturation` | 25 to 300 °C at the saturation pressure of water |
| `conditions-grid` | 25 to 500 °C, and 1 to 5000 bar |
| `output-publication` | kJ/mol and fixed decimals |
| `output-all-digits` | All digits, in the units of the engine, and the provenance in a separate file |

## Python and C++

| Python | C++ (`ThermoFun/Batch/BatchSetup.h`) | What it does |
|---|---|---|
| `tf.batch(setup)`, `tf.batch(template=..., database=..., substances=...)` | `BatchSetup::fromFile`, `fromText`, `fromTemplate`, `run()` | Runs a batch calculation; gives `Results` (C++ `BatchResults`) |
| `tf.template(name, **keys)` | `BatchSetup::fromTemplate` | The setup of a template, as a `BatchSetup` |
| `setup.set(path, value)`, `setup.get(path)` | `set(path, json)`, `get(path)` | A value by its path, for example `/results/0/name` |
| `setup.fill(substances=..., T=..., layout=...)` | `setSubstances`, `setTemperatures`, `setLayout`, ... | Values by name: each result, the conditions, the output |
| `setup.property`, `add_property`, `remove_property` | `setProperty`, `addProperty`, `removeProperty`, `changeProperty` | A property entry |
| `setup.use(name)` | `useTemplate` | Puts a template under the setup |
| `setup.save(path)`, `str(setup)` | `save`, `toYaml` | The setup as YAML |
| `results["name"]` | `results.at("name")` | A `ResultTable`: `to_text`, `write`, `value`, `scalar`, `values`, `pressure_used`, `rows`, `columns`, `problems`, `provenance` |
| `results.problems`, `results.write()` | `problems()`, `write()` | What could not be calculated; writes each result with `output.file` |
| `table.provenance.text()` | `Provenance::text()` | How a result was made: the database, the templates and `engine.summary()` of each engine |
| `setup.batch(result)`, `setup.apply_to(batch)` | `batch`, `applyTo` | A `ThermoBatch` from one result ([Use ThermoBatch](../how-to/use-thermobatch.md)) |
| `python -m thermofun.batch setup.yaml`, `--template NAME`, `--list` | | The same from a shell |

## ThermoBatch, the 0.x batch API

| Method of `ThermoBatch` | What it does |
|---|---|
| `thermoPropertiesSubstance`, `thermoPropertiesReaction` | One T and P, a list of pairs, or a list of T and a list of P, for one or many symbols and properties. |
| `setPropertyUnit`, `setPropertyDigit`, `setPropertyUnitDigit`, `setPropertiesUnits`, `setPropertiesDigits`, `setUnits`, `setDigits` | The unit and the decimals of the output. Temperature and pressure units are also those of your input. |
| `setTemperatureIncrement(first, last, step)`, `setPressureIncrement(first, last, step)` | A grid of T and P. |
| `setSolventSymbol(symbol)` | The water record. |
| `setBatchPreferences(preferences)` | The options below. |
| `selectSubstances(rule)`, `selectReactions(rule)` | The symbols that a rule chooses: see [Run a batch calculation](../how-to/make-tables.md#4-choose-records-by-a-rule). |
| `provenance()` | What the results depend on, as lines of text. |

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
| `writeProvenance` | false | A CSV file starts with lines `# ...` that say how it was made, as `Output.provenance()` gives them. |

The table from `ThermoBatch` calls to the keys of a setup is in [Use ThermoBatch](../how-to/use-thermobatch.md#from-thermobatch-to-a-batch-setup).
