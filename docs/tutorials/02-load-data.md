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

# 2. Load data

You learn to load a database, look at its records, check them, and write your own data. It takes 15 minutes.

**You need:** ThermoFun, and the files `aq17-thermofun.json` from `Resources/databases/` and `minimal.yaml` from `examples/v2/` of the repository, in your folder.

A `Database` holds the records. A `ThermoEngine` calculates from them.

## Load a database

Give the path of a file. ThermoFun finds the format: the old ThermoFun JSON format, or the new format (version 2) in YAML or JSON.

```{code-cell} ipython3
import thermofun as tf

database = tf.Database("aq17-thermofun.json")
print(database.numberOfElements(), "elements,", database.numberOfSubstances(), "substances,", database.numberOfReactions(), "reactions")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert database.containsSubstance("Calcite")
assert database.numberOfSubstances() > 100
```

## Look at the records

```{code-cell} ipython3
print(database.getSubstancesList()[:10])
print(database.containsSubstance("Calcite"), database.containsSubstance("Quartzz"))
```

```{code-cell} ipython3
calcite = database.getSubstance("Calcite")
print(calcite.name(), calcite.formula())
print("G at the reference conditions:", calcite.thermoReferenceProperties().gibbs_energy.val, "J/mol")
```

## Choose records by a rule

A rule gives the symbols of the records that fit. It is shorter than a list, and tables and batch calculations use it too.

```{code-cell} ipython3
print(tf.tables.select_substances(database, {"class": "aqueous_solute", "elements": ["Ca"]}))
print(tf.tables.select_substances(database, {"only_elements": ["Ca", "C", "O"], "aggregate_state": "crystal"}))
print(tf.tables.select_substances(database, {"match": "Ca*", "exclude": ["CaC*"]}))
```

## Write your own data

`minimal.yaml` is a small file in the new format, written by hand. It has water, Na+, Cl- and halite, and one reaction.

```{code-cell} ipython3
print(open("minimal.yaml").read())
```

Each substance has:

- `formula`, `class` and `aggregate_state`;
- `standard_state_properties`: G and H in J/mol, S and Cp in J/(mol K), V in m3/mol unless a unit is given;
- `methods`: how to calculate at other T and P.

```{code-cell} ipython3
my_data = tf.Database("minimal.yaml")
print(my_data.getSubstancesList(), my_data.getReactionsList())

engine = tf.ThermoEngine(my_data)
sodium = engine.thermoPropertiesSubstance(373.15, 1e7, "Na+")
print(sodium.gibbs_energy.val, "J/mol")
```

`Na+` needs water. `HKF88` reads the density and the dielectric constant from the record `H2O@`, which has an equation of state of water and a dielectric model.

```{code-cell} ipython3
path = engine.calculationPath("Na+")
print(path["source"], [(s["symbol"], s["role"]) for s in path["steps"]])
```

```{code-cell} ipython3
:tags: [remove-cell]
assert sodium.gibbs_energy.sta[0] == tf.Status.calculated
assert path["source"] == "methods"
assert path["steps"][0]["role"] == "solvent" and path["steps"][0]["symbol"] == "H2O@"
```

The reaction has no methods, so its properties come from its reactants.

```{code-cell} ipython3
reaction = engine.thermoPropertiesReaction(298.15, 1e5, "Halite-dissolution")
print(reaction.reaction_gibbs_energy.val, "J/mol, from", engine.calculationPath("Halite-dissolution")["source"])
print(-256142.0 - 131290.0 + 384212.0, "J/mol, from the values in the file")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(reaction.reaction_gibbs_energy.val - (-256142.0 - 131290.0 + 384212.0)) < 0.01
```

All fields are in [Add data](../how-to/add-data.md).

## Check the data

`validate` lists all problems of the records. Loading a file refuses only what makes the file unusable, so check your data with `validate`. aq17 has two records whose volume method has no parameters.

```{code-cell} ipython3
print("problems in minimal.yaml:", len(my_data.validate()))
for problem in database.validate():
    print(problem.severity, problem.code, problem.record, problem.message)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert not [d for d in my_data.validate() if d.severity == tf.Severity.Error]
```

## Add more data

`appendData` adds the records of another file. A record with the same symbol replaces the record before.

```{code-cell} ipython3
merged = tf.Database("aq17-thermofun.json")
merged.appendData("minimal.yaml")
print(merged.numberOfSubstances(), merged.containsSubstance("Halite"), merged.containsReaction("Halite-dissolution"))
```

## Save in the new format

```{code-cell} ipython3
import os

os.makedirs("output", exist_ok=True)
database.save("output/aq17.yaml")
print(tf.Database("output/aq17.yaml").numberOfSubstances())
```

## Change a value

Change a value with a parameter handle. The change holds for this engine only.

```{code-cell} ipython3
omega = engine.parameter("Na+", "HKF88", "omega")
before = engine.thermoPropertiesSubstance(373.15, 1e7, "Na+").gibbs_energy.val
omega.setValue(omega.value() * 1.01)
after = engine.thermoPropertiesSubstance(373.15, 1e7, "Na+").gibbs_energy.val
print(before, "->", after, "J/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert after != before
```

**Next:** [3. Change the models](03-change-models.md).
