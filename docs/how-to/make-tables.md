<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Make tables

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A table comes from a setup. A setup is a YAML file, a template, or a dict in Python. All the work is in the library (`ThermoFun/Batch/Tables.h`), so C++ and Python have the same tables and templates. The same setup can also run as a `ThermoBatch`: see section 7.

## 1. Use a template

Give only your records. The template has the rest.

```python
import thermofun as tf

results = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2", "Calcite"])
species = results["species"]
print(species.to_text("md"))
print(species.columns[0]["label"])
```

Output:

```text
| Symbol | T(degC) | P(bar) | G(kJ/mol) | H(kJ/mol) | S(J/K/mol) | Cp(J/K/mol) | V(J/bar) |
|---|---|---|---|---|---|---|---|
| Ca+2 | 25 | 0.0316982 | -552.819 | -543.098 | -56.48 | -31.04 | -1.8449 |
| Ca+2 | 50 | 0.123525 | -551.378 | -543.790 | -58.71 | -26.48 | -1.8277 |
| Ca+2 | 75 | 0.385967 | -549.885 | -544.494 | -60.81 | -30.79 | -1.8682 |
| Ca+2 | 100 | 1.01418 | -548.337 | -545.374 | -63.24 | -40.45 | -1.9572 |
| Ca+2 | 125 | 2.32234 | -546.722 | -546.554 | -66.29 | -54.62 | -2.0995 |
| Ca+2 | 150 | 4.76159 | -545.024 | -548.130 | -70.11 | -72.08 | -2.3063 |
| Ca+2 | 175 | 8.92601 | -543.225 | -550.157 | -74.74 | -91.95 | -2.5906 |
| Ca+2 | 200 | 15.5494 | -541.307 | -552.795 | -80.42 | -124.66 | -2.9732 |
| Ca+2 | 225 | 25.4975 | -539.240 | -556.474 | -87.92 | -180.12 | -3.5180 |
| Ca+2 | 250 | 39.762 | -536.977 | -561.709 | -98.05 | -257.43 | -4.3541 |
| Ca+2 | 275 | 59.464 | -534.469 | -568.627 | -110.77 | -326.93 | -5.6161 |
| Ca+2 | 300 | 85.8787 | -531.705 | -575.943 | -123.53 | -283.35 | -7.1252 |
| Calcite | 25 | 0.0316982 | -1129.199 | -1207.474 | 92.50 | 83.37 | 3.6978 |
| Calcite | 50 | 0.123525 | -1131.597 | -1205.340 | 99.37 | 87.18 | 3.6997 |
| Calcite | 75 | 0.385967 | -1134.164 | -1203.118 | 105.99 | 90.46 | 3.7017 |
| Calcite | 100 | 1.01418 | -1136.891 | -1200.817 | 112.36 | 93.31 | 3.7038 |
| Calcite | 125 | 2.32234 | -1139.773 | -1198.448 | 118.50 | 95.83 | 3.7059 |
| Calcite | 150 | 4.76159 | -1142.800 | -1196.014 | 124.40 | 98.09 | 3.7082 |
| Calcite | 175 | 8.92601 | -1145.966 | -1193.521 | 130.09 | 100.13 | 3.7104 |
| Calcite | 200 | 15.5494 | -1149.263 | -1190.969 | 135.58 | 102.00 | 3.7128 |
| Calcite | 225 | 25.4975 | -1152.682 | -1188.361 | 140.87 | 103.71 | 3.7152 |
| Calcite | 250 | 39.762 | -1156.215 | -1185.696 | 145.99 | 105.30 | 3.7176 |
| Calcite | 275 | 59.464 | -1159.854 | -1182.973 | 150.93 | 106.78 | 3.7201 |
| Calcite | 300 | 85.8787 | -1163.589 | -1180.189 | 155.73 | 108.18 | 3.7225 |

G(kJ/mol)
```

<!-- check
```python
assert species.columns[0]["label"] == "G(kJ/mol)"
```
-->

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

These templates have no tables. They give settings to one of the templates above:

| Template | What it sets |
|---|---|
| `batch-reference` | 25 °C and 1 bar |
| `batch-saturation` | 25 to 300 °C at the saturation pressure of water |
| `batch-grid` | 25 to 500 °C, and 1 to 5000 bar |
| `output-publication` | kJ/mol and fixed decimals |
| `output-data` | All digits, in the units of the engine, and how it was made in a separate file |

## 2. Change any setting of a template

`set` takes any key of a table. `column` changes one column.

```python
setup = tf.tables.template("species-properties", database="aq17-thermofun.json", records=["Ca+2"])
setup.set(T={"values": [25.0, 100.0], "unit": "degC"}, layout="transposed", output="mine.md")
setup.column("G", unit="J/mol", digits=1)
setup.remove_column("V")
setup.add_column("A", unit="kJ/mol", digits=2)
results = setup.run()
print(results.write())
```

Output:

```text
['mine.md']
```

<!-- check
```python
assert results.write() == ["mine.md"]
```
-->

Keep your changes as a file. It records how you made the table.

```python
setup.save("mine.yaml")
again = tf.table("mine.yaml")
```

<!-- check
```python
assert again["species"].to_text() == results["species"].to_text()
```
-->

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
print(sorted(results.write()))
```

Output:

```text
['logk.md', 'species.csv', 'water.csv']
```

<!-- check
```python
assert sorted(results.write()) == ["logk.md", "species.csv", "water.csv"]
```
-->

## 4. Choose records by a rule

`records` and `reactions` take a rule instead of a list, or a list with rules in it. The symbols come in alphabetical order, and each comes once.

```python
results = tf.table(template="species-properties", database="aq17-thermofun.json",
                   records={"class": "aqueous_solute", "elements": ["Ca"]})

database = tf.Database("aq17-thermofun.json")
```

<!-- check
```python
assert "Ca+2" in {row[0] for row in results["species"].rows}
assert "Calcite" in tf.tables.select_substances(database, {"elements": ["Ca", "C"], "exclude": ["CaCO3@"]})
```
-->

In YAML:

```yaml
records: [H2O@, {class: aqueous_solute, only_elements: [Ca, C, O, H]}]
```

| Key | For | Which records |
|---|---|---|
| `class` | substances | Of this class, or one of a list: `component`, `gas_fluid`, `aqueous_solute`, `water_solvent`, `surface_species`, `site_moiety`, `other`. |
| `aggregate_state` | substances | In this state, or one of a list: `gas`, `aqueous`, `crystal` and others. |
| `charge` | substances | With this charge, or one of a list. |
| `elements` | both | With all of these elements. |
| `only_elements` | both | With no other elements. The charge does not count. A reaction counts the elements of all its species. |
| `involving` | reactions | With all of these species. |
| `match` | both | With a symbol like this, or one of a list: `Ca*`. `*` is any text, and `?` is one character. |
| `exclude` | both | Leaves out these symbols, with `*` and `?` too. |

A rule that chooses nothing is an error, and so is a misspelled key. The message gives the nearest word.

## 5. Use templates of your own

A template is a YAML file, with what it makes in a comment on the first line. Put your files in a folder, and name the folder in `THERMOFUN_TEMPLATE_PATH`. Separate several folders with `:`, or with `;` on Windows.

<!-- file: my-templates/my-table.yaml -->
```yaml
# G and S at 25 and 50 degC, for my group
conditions:
  T: {values: [25, 50], unit: degC}
  P: {values: [1], unit: bar}
tables:
  - name: mine
    columns: [{property: G, unit: kJ/mol, digits: 2}, {property: S, digits: 2}]
```

```python
import os
os.environ["THERMOFUN_TEMPLATE_PATH"] = "my-templates"

print(tf.tables.templates()["my-table"])
results = tf.table(template="my-table", database="aq17-thermofun.json", records=["Ca+2"])
```

Output:

```text
G and S at 25 and 50 degC, for my group
```

<!-- check
```python
assert tf.tables.templates()["my-table"] == "G and S at 25 and 50 degC, for my group"
assert tf.tables.template_source("my-table") == os.path.join("my-templates", "my-table.yaml")
del os.environ["THERMOFUN_TEMPLATE_PATH"]
```
-->

A file with the name of a template of the library replaces it. `template_source` says which one you have, and the output names the path. The first folder that has a name wins.

Any setup can start from templates, with the key `template`. What comes later replaces what comes before, and your own settings come last:

<!-- file: from-templates.yaml -->
```yaml
template: [species-properties, batch-saturation, output-publication]
database: aq17-thermofun.json
records: {class: aqueous_solute, elements: [Ca]}
tables:
  - name: species             # changes the table of this name; a new name adds a table
    layout: transposed
```

```python
results = tf.table("from-templates.yaml")
print(results["species"].columns[0]["unit"])
```

Output:

```text
kJ/mol
```

<!-- check
```python
assert results["species"].columns[0]["unit"] == "kJ/mol"
```
-->

In Python, give a list: `tf.table(template=["species-properties", "batch-saturation"], ...)`, or call `setup.use("batch-saturation")`. In C++, call `setup.useTemplate("batch-saturation")`.

## 6. See how a table was made

A file starts with how it was made, as comments that a table program can skip: the database with the SHA-256 of its records, the templates, the ThermoFun version, the physical constants, the methods of water, the conventions, and the options that are not at their default.

```python
results = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2"])
species = results["species"]
print("\n".join(species.provenance.lines()))
```

Output:

```text
database: aq17-thermofun.json
templates: species-properties
ThermoFun ..., physical constants codata1986
data: format legacy, dataset aq17-gem, sha256 ...
water: H2O@: WaterIAPWS95/reaktoro, JohnsonNorton91/reaktoro
conventions: energy_reference.elements = benson_helgeson, energy_reference.water = geochemical, standard_state.concentration_scale = molality, standard_state.pressure = 1bar
```

<!-- check
```python
assert species.to_text("csv", with_provenance=True).startswith("# database: aq17-thermofun.json")
```
-->

`provenance` is a setting of a table:

| Value | What it does |
|---|---|
| `header` | The lines come first in the file: `# ` in CSV, `% ` in LaTeX, `<!-- -->` in Markdown. This is the default. |
| `file` | The file has only the table. `<file>.provenance.yaml` has the lines and the whole setup of the table. |
| `none` | Nothing is written. |

The lines have no date, so making a file again gives the same text. A difference between two files is a difference in the results or in what they depend on.

To read a CSV file with its lines in pandas: `pandas.read_csv(path, comment="#")`.

## 7. Run a setup as a ThermoBatch

`setup.batch()` gives a `ThermoBatch` with the engine, units, digits and preferences of one table, and the records and T-P points to calculate. This is for code that needs an `Output`.

```python
setup = tf.tables.template(["species-properties", "batch-reference"], database="aq17-thermofun.json",
                           records={"class": "aqueous_solute", "elements": ["Ca"]})
calculation = setup.batch("species")
print(calculation.properties[0])

output = calculation.run()                  # the Output of ThermoBatch

path = calculation.write("batch.csv")       # in the layout of the setup, with how it was made
```

Output:

```text
gibbs_energy
```

<!-- check
```python
assert calculation.properties[0] == "gibbs_energy"
assert len(output.to2DVectorDouble()) == len(calculation.symbols)
assert open(path).read().startswith("# database:")
```
-->

```cpp
auto calculation = setup.batch("species");
auto output = calculation.run();
output.toCSV("batch.csv");
```

A batch that you already have takes the settings and the rules too:

```python
batch = tf.ThermoBatch("aq17-thermofun.json")
tf.tables.template("species-properties").apply_to(batch)          # units, digits, preferences
symbols = batch.selectSubstances({"class": "aqueous_solute", "elements": ["Ca"]})
output = batch.thermoPropertiesSubstance([[298.15, 1e5]], symbols, ["gibbs_energy"])
```

<!-- check
```python
assert any("sha256" in line for line in output.provenance())
```
-->

A `ThermoBatch` writes no record of how it was made unless you ask, as before: set `batch.preferences.writeProvenance = True`. A setup sets it for you, from `provenance`.

A setup cannot run as a batch if a table has `compare`, a column with `show` other than `val`, the properties of water, or records and reactions together. The message says so. Use a table for these.

## 8. The same in C++

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

`setRecordRule`, `setReactionRule`, `useTemplate`, `batch` and `applyTo` are there too, and `selectSubstances` and `selectReactions` are in `ThermoFun/Batch/RecordRules.h`. Python has these functions too, on `setup.core`, with the same names.

## 9. All settings

Settings at the top of a setup are the defaults of its tables.

| Key | Values |
|---|---|
| `database` | A file. In Python, also a `Database` or an engine. |
| `template` | A name or a list of names, under the other settings. |
| `conditions` | `T` and `P`: a list, `{values, unit}` or `{from, to, step, unit}`. `saturation` is a value of `P`: the table then shows the saturation pressure that the calculation used, and a value is asked for with the word `saturation`. `pairs: true` goes through T and P together. `order: P_first` changes the order. |
| `records`, `reactions` | Symbols, or a rule, or a list with rules in it. A reaction can be an equation. |
| `columns` | A property, or `{property, show, unit, digits, format, label}`. A property is G, H, S, Cp, Cv, V, U, A (or logK, dG, ... for reactions), a property of water such as `density`, `epsilon` or `bornQ`, or a named output of a method such as `HKF88:born_G` (its value only). |
| `show` | `val`, `err`, `ddt`, `ddp`, `status`. |
| property | Substance: `G`, `H`, `S`, `Cp`, `Cv`, `V`, `U`, `A`. Reaction: `logK`, `lnK`, `dG`, `dH`, `dS`, `dCp`, `dCv`, `dV`, `dU`, `dA`. Water: `density`, `Alpha`, `Beta`, `dAldT`, `epsilon`, `bornZ`, `bornY`, `bornQ`, `bornX` and others. |
| `layout` | `long`, `transposed`, `grid`. |
| `across` | For `grid`: `P` (the default) or `T`. |
| `loop` | For `long`: `records` (the default) or `conditions` first. |
| `format`, `digits`, `digits_T`, `digits_P` | `fixed`, `floating`, `scientific`, and the decimals. `digits` can be for each property: `{G: 2, S: 3}`. |
| `units` | The unit of a property in the columns that give no unit: `{G: kJ/mol}`. |
| `separator` | Of a CSV file. |
| `not_defined`, `write_not_defined` | The text for a value that is not defined, and if it is written. |
| `output`, `format_file` | The file. The extension says the format: `csv`, `md` or `tex`. |
| `provenance` | `header`, `file` or `none`: see section 6. |
| `options` | The options of the engine: see [Options of the engine](../reference/engine-preferences.md). |
| `conventions` | The conventions of the results. |
| `solvent` | The symbol of the water record. |
| `source`, `sources` | `source: reaction` or `reactants` for the table, `sources` for each symbol. |
| `compare` | A list: `{label, replace, options, conventions, database}`. Each gives its own columns. |

## 10. From ThermoBatch to tables

`ThermoBatch` stays. A table does all that it does:

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
| `writeProvenance` | `provenance` |

A calculation that fails leaves an empty value and a message. The other values are there.

```python
bad = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2", "NoSuch"])
```

<!-- check
```python
assert bad["species"].rows and bad.problems[0][1] == "NoSuch"
```
-->
