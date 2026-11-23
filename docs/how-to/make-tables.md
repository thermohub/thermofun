<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Run a batch calculation

**Related:** [Tutorial 5: Batch calculations](../tutorials/05-batch-calculations.md) · [Tutorial 6: Batch templates](../tutorials/06-batch-templates.md) · [Batch setups](../reference/batch.md) (all keys) · [Use ThermoBatch](use-thermobatch.md) · [Units](../reference/units.md)

**You need:** ThermoFun, and the file `aq17-thermofun.json` from `Resources/databases/` of the repository, in your folder.

A batch calculation calculates many records at many conditions at once (decision 0032). It comes from a **batch setup**: a YAML file, a batch template, or a dict in Python, with four blocks: `database` and `engine` (with what), `conditions` (T and P), `results` (one entry for each result table) and `output` (units, digits, layout, file). All the work is in the library (`ThermoFun/Batch/BatchSetup.h`), so C++ and Python have the same setups and templates. The same setup can also run as a `ThermoBatch`, the 0.x batch API: see section 7.

## 1. Use a template

Give only your records. The template has the rest.

```python
import thermofun as tf

results = tf.batch(template="species-properties", database="aq17-thermofun.json", substances=["Ca+2", "Calcite"])
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

`tf.templates()` lists them, and `python -m thermofun.batch --list` too. The batch templates are listed in [Batch setups](../reference/batch.md#batch-templates).

## 2. Change any setting of a template

`set` takes a path and a value: the keys from the top, and the number of a result in the list. `fill` takes keys by name (`substances`, `T`, `layout`, `file`, ...), and `property` changes one property.

```python
setup = tf.template("species-properties", database="aq17-thermofun.json", substances=["Ca+2"])
setup.set("/conditions/T", {"values": [25.0, 100.0], "unit": "degC"})
setup.fill(layout="wide", file="mine.md")
setup.property("G", unit="J/mol", digits=1)
setup.remove_property("V")
setup.add_property("A", unit="kJ/mol", digits=2)
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

Keep your changes as a file. It records how you made the results.

```python
setup.save("mine.yaml")
again = tf.batch("mine.yaml")
```

<!-- check
```python
assert again["species"].to_text() == results["species"].to_text()
```
-->

## 3. Write a setup

Start from the template `blank` (`python -m thermofun.batch --template blank`): it has a `<...>` for each value to give, and a setup that still has one does not run.

<!-- file: setup.yaml -->
```yaml
database: aq17-thermofun.json

conditions:
  T: {from: 25, to: 300, step: 25, unit: degC}
  P: {values: [saturation, 1000], unit: bar}

results:
  - name: species
    substances: [Ca+2, Na+, Calcite]
    properties:
      - {name: G, unit: kJ/mol, digits: 3}
      - {name: S, unit: J/(mol K), digits: 2}
      - {name: G, show: uncertainty, unit: kJ/mol, digits: 3}
      - {name: G, show: ddt, label: dG/dT}
    output: {file: species.csv}

  - name: water
    substances: [H2O@]
    properties: [density, epsilon, bornQ, bornX]
    output: {file: water.csv}

  - name: logK
    reactions: ["Calcite = Ca+2 + CO3-2"]
    properties: [{name: logK, digits: 3}, {name: dH, unit: kJ/mol}]
    output: {file: logk.md, layout: grid}
```

Run it from a shell with `python -m thermofun.batch setup.yaml`, or in Python:

```python
results = tf.batch("setup.yaml")
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

`substances` and `reactions` take a rule instead of a list, or a list with rules in it. The symbols come in alphabetical order, and each comes once.

```python
results = tf.batch(template="species-properties", database="aq17-thermofun.json",
                   substances={"class": "aqueous_solute", "elements": ["Ca"]})

database = tf.Database("aq17-thermofun.json")
```

<!-- check
```python
assert "Ca+2" in {row[0] for row in results["species"].rows}
assert "Calcite" in tf.batch.select_substances(database, {"elements": ["Ca", "C"], "exclude": ["CaCO3@"]})
```
-->

In YAML:

```yaml
substances: [H2O@, {class: aqueous_solute, only_elements: [Ca, C, O, H]}]
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

A template is a YAML file with any of the blocks of a setup, with what it makes in a comment on the first line. Put your files in a folder, and name the folder in `THERMOFUN_TEMPLATE_PATH`. Separate several folders with `:`, or with `;` on Windows.

<!-- file: my-templates/my-table.yaml -->
```yaml
# G and S at 25 and 50 degC, for my group
conditions:
  T: {values: [25, 50], unit: degC}
  P: {values: [1], unit: bar}
results:
  - name: mine
    properties: [{name: G, unit: kJ/mol, digits: 2}, {name: S, digits: 2}]
```

```python
import os
os.environ["THERMOFUN_TEMPLATE_PATH"] = "my-templates"

print(tf.templates()["my-table"])
results = tf.batch(template="my-table", database="aq17-thermofun.json", substances=["Ca+2"])
```

Output:

```text
G and S at 25 and 50 degC, for my group
```

<!-- check
```python
assert tf.templates()["my-table"] == "G and S at 25 and 50 degC, for my group"
assert tf.batch.template_source("my-table") == os.path.join("my-templates", "my-table.yaml")
del os.environ["THERMOFUN_TEMPLATE_PATH"]
```
-->

A file with the name of a template of the library replaces it. `template_source` says which one you have, and the output names the path. The first folder that has a name wins.

Any setup can start from templates, with the key `template`. What comes later replaces what comes before, and your own settings come last:

<!-- file: from-templates.yaml -->
```yaml
template: [species-properties, conditions-saturation, output-publication]
database: aq17-thermofun.json
results:
  - name: species             # changes the result of this name; a new name adds a result
    substances: {class: aqueous_solute, elements: [Ca]}
    output: {layout: wide}
```

```python
results = tf.batch("from-templates.yaml")
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

In Python, give a list: `tf.batch(template=["species-properties", "conditions-saturation"], ...)`, or call `setup.use("conditions-saturation")`. In C++, call `setup.useTemplate("conditions-saturation")`.

## 6. See how a result was made

A file starts with how it was made (its **provenance**), as comments that a table program can skip: the database with the SHA-256 of its records, the templates, and the summary of the engine: the ThermoFun version, the physical constants, the methods of water, the conventions, and the options that are not at their default.

```python
results = tf.batch(template="species-properties", database="aq17-thermofun.json", substances=["Ca+2"])
species = results["species"]
print(species.provenance.text())
```

Output:

```text
database: aq17-thermofun.json
templates: species-properties
ThermoFun ..., physical constants codata1986
data: format legacy, dataset aq17-gem, sha256 ...
water: H2O@: WaterIAPWS95/reaktoro, JohnsonNorton91/reaktoro
conventions: energy_reference.elements = benson_helgeson, energy_reference.ions = hydrogen_ion, energy_reference.water = helgeson_kirkham_1974, standard_state.concentration_scale = molality, standard_state.pressure = 1bar
```

<!-- check
```python
assert species.to_text("csv", with_provenance=True).startswith("# database: aq17-thermofun.json")
```
-->

`provenance` is a key of the block `output`:

| Value | What it does |
|---|---|
| `header` | The lines come first in the file: `# ` in CSV, `% ` in LaTeX, `<!-- -->` in Markdown. This is the default. |
| `file` | The file has only the result table. `<file>.provenance.yaml` has the lines and the whole setup of the result. |
| `none` | Nothing is written. |

The lines have no date, so making a file again gives the same text. A difference between two files is a difference in the results or in what they depend on.

To read a CSV file with its lines in pandas: `pandas.read_csv(path, comment="#")`.

## 7. Run a setup as a ThermoBatch

`setup.batch()` gives a `ThermoBatch` with the engine, units, digits and preferences of one result, and the records and T-P points to calculate. This is for code that needs an `Output`. More in [Use ThermoBatch](use-thermobatch.md).

```python
setup = tf.template(["species-properties", "conditions-reference"], database="aq17-thermofun.json",
                    substances={"class": "aqueous_solute", "elements": ["Ca"]})
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
tf.template("species-properties").apply_to(batch)          # units, digits, preferences
symbols = batch.selectSubstances({"class": "aqueous_solute", "elements": ["Ca"]})
output = batch.thermoPropertiesSubstance([[298.15, 1e5]], symbols, ["gibbs_energy"])
```

<!-- check
```python
assert any("sha256" in line for line in output.provenance())
```
-->

A `ThermoBatch` writes no record of how it was made unless you ask, as before: set `batch.preferences.writeProvenance = True`. A setup sets it for you, from `output.provenance`.

A setup cannot run as a `ThermoBatch` if a result has `compare`, a property with `show` other than `value`, the properties of water, or substances and reactions together. The message says so. Use `tf.batch` for these.

## 8. The same in C++

```cpp
#include <ThermoFun/ThermoFun.h>
#include <ThermoFun/Batch/BatchSetup.h>

auto setup = ThermoFun::BatchSetup::fromTemplate("species-properties");
setup.setDatabase("aq17-thermofun.json");
setup.setSubstances({"Ca+2", "Calcite"});
setup.setTemperatures(25.0, 100.0, 25.0, "degC");
setup.setProperty("G", "J/mol", 1);
setup.set("/output/layout", "\"wide\"");
auto results = setup.run();
results.at("species").write("species.md");
double G = results.at("species").cell("Ca+2", 25.0, 0.0, "G", true).value;
```

`setSubstanceRule`, `setReactionRule`, `useTemplate`, `batch` and `applyTo` are there too, and `selectSubstances` and `selectReactions` are in `ThermoFun/Batch/RecordRules.h`. Python has these functions too, on `setup.core`, with the same names.

## 9. When a value cannot be calculated

A calculation that fails leaves an empty value and a message. The other values are there.

```python
bad = tf.batch(template="species-properties", database="aq17-thermofun.json", substances=["Ca+2", "NoSuch"])
```

<!-- check
```python
assert bad["species"].rows and bad.problems[0][1] == "NoSuch"
```
-->

All the keys of a setup, the templates and the table from `ThermoBatch` to a setup are in [Batch setups](../reference/batch.md).
