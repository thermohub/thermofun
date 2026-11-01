<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Make tables

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A table comes from a setup. A setup is a YAML file, a template of the library, or a dict in Python. All the work is in the library (`ThermoFun/Batch/Tables.h`), so C++ and Python have the same tables and templates.

## 1. Use a template

Give only your records. The template has the rest.

```python
import thermofun as tf

results = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2", "Calcite"])
species = results["species"]
print(species.to_text("md"))
assert species.columns[0]["label"] == "G(kJ/mol)"
```

`tf.tables.templates()` lists them, and `python -m thermofun.tables --list` too.

| Template | What it makes | You give |
|---|---|---|
| `species-properties` | G, H, S, Cp and V | `records` |
| `reaction-properties` | log K, dG, dH, dS and dCp | `reactions` |
| `logk-grid` | log K on a grid: T down and P across | `reactions` |
| `water-properties` | density, dielectric constant and Born functions of water | nothing |
| `uncertainties` | G and S with their uncertainties | `records` |
| `derivatives` | G with its derivatives by T and P | `records` |
| `model-comparison` | V and Cp with two dielectric models, side by side | `records` |

## 2. Change any setting of a template

`set` takes any key of a table. `column` changes one column.

```python
setup = tf.tables.template("species-properties", database="aq17-thermofun.json", records=["Ca+2"])
setup.set(T={"values": [25.0, 100.0], "unit": "degC"}, layout="transposed", output="mine.md")
setup.column("G", unit="J/mol", digits=1)
setup.remove_column("V")
setup.add_column("A", unit="kJ/mol", digits=2)
results = setup.run()
assert results.write() == ["mine.md"]
```

Keep your changes as a file. It records how you made the table.

```python
setup.save("mine.yaml")
again = tf.table("mine.yaml")
assert again["species"].to_text() == results["species"].to_text()
```

## 3. Write a setup

<!-- file: setup.yaml -->
```yaml
database: aq17-thermofun.json

conditions:
  T: {from: 25, to: 300, step: 25, unit: degC}
  P: {values: [saturation, 1000], unit: bar}

tables:
  - name: species
    records: [Ca+2, Na+, Calcite]
    columns:
      - {property: G, unit: kJ/mol, digits: 3}
      - {property: S, unit: J/(mol K), digits: 2}
      - {property: G, show: err, unit: kJ/mol, digits: 3}
      - {property: G, show: ddt, label: dG/dT}
    output: species.csv

  - name: water
    records: [H2O@]
    columns: [density, epsilon, bornQ, bornX]
    output: water.csv

  - name: logK
    reactions: ["Calcite = Ca+2 + CO3-2"]
    columns: [{property: logK, digits: 3}, {property: dH, unit: kJ/mol}]
    layout: grid
    output: logk.md
```

Run it from a shell with `python -m thermofun.tables setup.yaml`, or in Python:

```python
results = tf.table("setup.yaml")
assert sorted(results.write()) == ["logk.md", "species.csv", "water.csv"]
```

## 4. The same in C++

```cpp
#include <ThermoFun/ThermoFun.h>
#include <ThermoFun/Batch/Tables.h>

auto setup = ThermoFun::TableSetup::fromTemplate("species-properties");
setup.setDatabase("aq17-thermofun.json");
setup.setRecords({"Ca+2", "Calcite"});
setup.setTemperatures(25.0, 100.0, 25.0, "degC");
setup.setColumn("G", "J/mol", 1);
auto results = setup.run();
results.at("species").write("species.md");
double G = results.at("species").cell("Ca+2", 25.0, 0.0, "G", true).value;
```

Python has these functions too, on `setup.core`, with the same names.

## 5. All settings

Settings at the top of a setup are the defaults of its tables.

| Key | Values |
|---|---|
| `database` | A file. In Python, also a `Database` or an engine. |
| `conditions` | `T` and `P`: a list, `{values, unit}` or `{from, to, step, unit}`. `saturation` is a value of `P`. `pairs: true` goes through T and P together. `order: P_first` changes the order. |
| `records`, `reactions` | Symbols. A reaction can be an equation. |
| `columns` | A property, or `{property, show, unit, digits, format, label}`. |
| `show` | `val`, `err`, `ddt`, `ddp`, `status`. |
| property | Substance: `G`, `H`, `S`, `Cp`, `Cv`, `V`, `U`, `A`. Reaction: `logK`, `lnK`, `dG`, `dH`, `dS`, `dCp`, `dCv`, `dV`, `dU`, `dA`. Water: `density`, `Alpha`, `Beta`, `dAldT`, `epsilon`, `bornZ`, `bornY`, `bornQ`, `bornX` and others. |
| `layout` | `long`, `transposed`, `grid`. |
| `across` | For `grid`: `P` (the default) or `T`. |
| `loop` | For `long`: `records` (the default) or `conditions` first. |
| `format`, `digits`, `digits_T`, `digits_P` | `fixed`, `floating`, `scientific`, and the decimals. |
| `separator` | Of a CSV file. |
| `not_defined`, `write_not_defined` | The text for a value that is not defined, and if it is written. |
| `output`, `format_file` | The file. The extension says the format: `csv`, `md` or `tex`. |
| `options` | The options of the engine: see [Options of the engine](../reference/engine-preferences.md). |
| `conventions` | The conventions of the results. |
| `solvent` | The symbol of the water record. |
| `source`, `sources` | `source: reaction` or `reactants` for the table, `sources` for each symbol. |
| `compare` | A list: `{label, replace, options, conventions, database}`. Each gives its own columns. |

## 6. From ThermoBatch to tables

`ThermoBatch` stays. A setup does all that it does:

| `ThermoBatch`, `Output` and `BatchPreferences` | In a setup |
|---|---|
| `toCSV` | `layout: long`, `output: x.csv` |
| `toCSVTransposed` | `layout: transposed` |
| `toCSVPropertyGrid` | `layout: grid`, with `across` |
| `to2DVectorDouble` | `table.values()`, in the units of the columns |
| `toDouble` | `table.value(record, T, P, column)` |
| `toThermoScalar` | `table.scalar(record, T, P, column)` |
| `setPropertyUnit`, `setPropertyDigit` and others | `unit` and `digits` of a column |
| `setTemperatureIncrement`, `setPressureIncrement` | `from`, `to`, `step` |
| `isFixed`, `isFloating`, `isScientific` | `format` |
| `separator`, `fileName` | `separator`, `output` |
| `solventFileName`, `outputSolventProperties` | A table of the water record |
| `substancePropertiesFromReaction` | `source: reaction` |
| `reactionPropertiesFromReactants` | `source: reactants` |
| `loopOverTPpairsFirst` | `loop` |
| `loopTemperatureThenPressure` | `order`, and `across` |
| `writeNaNifNotDefinedValue` | `write_not_defined` |

**Check:** a calculation that fails leaves an empty value and a message. The other values are there.

```python
bad = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2", "NoSuch"])
assert bad["species"].rows and bad.problems[0][1] == "NoSuch"
```
