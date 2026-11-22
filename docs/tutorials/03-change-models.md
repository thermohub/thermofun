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

# 3. Change the models

This page is the Jupyter notebook [`03-change-models.ipynb`](https://github.com/thermohub/thermofun-jupyter/blob/dev-v2/thermofun-basics/03-change-models.ipynb) of [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter/tree/dev-v2/thermofun-basics), which also has the data files it reads: download them into one folder to run it.

You learn which models there are, how a record combines them, and how to change the model of a record and the options of the engine.

**You need:** ThermoFun, and the files `aq17-thermofun.json` from `Resources/databases/` and `mineral-methods.yaml` from `examples/v2/` of the repository, in your folder.

**Related:** [List of methods](../reference/methods/index.md) · [How methods combine](../reference/how-methods-combine.md) · [Choose water models](../how-to/choose-water-models.md) · [Choose conventions](../how-to/choose-conventions.md) · [See what each method gives](../how-to/get-per-method-results.md) · [Add a method as data](../how-to/add-a-method-as-data.md) · [Options of the engine](../reference/engine-preferences.md) · [Conventions](../reference/conventions.md)

```{code-cell} ipython3
import thermofun as tf
import pandas as pd

AQ = "aq17-thermofun.json"
engine = tf.ThermoEngine(AQ)
```

## Which models are there

A **method** is one model. `tf.methods.catalog()` lists the methods, with their parameters and the source. The [method pages](../reference/methods/index.md) have the same data.

```{code-cell} ipython3
catalog = pd.DataFrame(tf.methods.catalog())
print(len(catalog), "methods")
catalog[catalog.water | catalog.dielectric][["id", "title", "implementations", "citation"]]
```

```{code-cell} ipython3
catalog[catalog.operates_on == "reaction"][["id", "title"]]
```

## Which models does a record use

```{code-cell} ipython3
print("H2O@   ", engine.calculationPath("H2O@")["methods"])
print("Ca+2   ", engine.calculationPath("Ca+2")["methods"])
print("Calcite", engine.calculationPath("Calcite")["methods"])
```

## Combine methods

A record can have several methods. In `mineral-methods.yaml`, Calcite has three:

- `CpPolynomial` is the base method. It gives G, H, S and Cp from the heat capacity.
- `LandauHP98` is an increment. It adds a phase transition.
- `MurnaghanHP98` is an increment. It gives the volume and the term of the pressure.

`explain` gives the properties after each method, in J/mol and m3/mol. Each row is the total after that method, so the last row is the result. The part that each method adds is in [See what each method gives](../how-to/get-per-method-results.md).

```{code-cell} ipython3
minerals = tf.ThermoEngine("mineral-methods.yaml")
steps = minerals.explain(473.15, 1e7, "Calcite")
result = minerals.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
pd.DataFrame(steps)[["method", "G", "S", "V"]]
```

```{code-cell} ipython3
:tags: [remove-cell]
assert [s["method"] for s in steps] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
assert abs(steps[-1]["G"] - result.gibbs_energy.val) < 1e-6
```

The engine sets the order from what each method does, so the order in the file does not change the result. Here the methods of Calcite are reversed in a copy of the database.

```{code-cell} ipython3
database = tf.Database("mineral-methods.yaml")
methods = database.get("/substances/Calcite/methods")
database.set("/substances/Calcite/methods", methods[::-1])
same = tf.ThermoEngine(database).thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(same.gibbs_energy.val == result.gibbs_energy.val)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert same.gibbs_energy.val == result.gibbs_energy.val
```

The reasons are in [How methods combine](../reference/how-methods-combine.md).

## How much each method adds

`explain` gives totals. `contributions` gives the part that each method adds to each property, at as many T as you like. A column such as `G:LandauHP98` is what that method changed G by, in J/mol, and `G:other` is what the conventions and options of the engine changed after the methods. The columns of a property add up to its value `G`. A method that sets a property (the base method) also removes what was there before it, so its column holds the whole value.

```{code-cell} ipython3
contributions = pd.DataFrame(minerals.contributions("Calcite", [473.15, 800.0, 1300.0], 1e7))
contributions[["T", "G", "G:CpPolynomial", "G:LandauHP98", "G:MurnaghanHP98"]]
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(contributions["G"][0] - contributions[["G:CpPolynomial", "G:LandauHP98", "G:MurnaghanHP98", "G:other"]].iloc[0].sum()) < 1e-6
```

## Look inside a method

A model has terms and intermediate functions. A method can give them as **named outputs**. `HKF88`, the model of aqueous species, gives its g function, the effective radius of the ion, the Born coefficient and the Born terms of G, H, S, Cp and V. The names and units are on the [page of the method](../reference/methods/HKF88.md), and in `tf.methods.catalog()`.

```{code-cell} ipython3
aqueous = tf.ThermoEngine(AQ)
T = [298.15, 373.15, 473.15, 573.15]                      # K
terms = pd.DataFrame(aqueous.auxiliaryOutputs("Ca+2", T, 1e8))   # at 1000 bar
terms[["T", "HKF88:g", "HKF88:omega", "HKF88:born_G", "HKF88:born_V"]]
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(terms["HKF88:g"][0]) < 1e-10 and terms["HKF88:g"][3] < 0.0
```

The g function is 0 at low T and becomes negative at high T, where it shrinks the effective radius of the ion and so changes its Born coefficient. Only some methods have named outputs; the others give only T and P. In a table, a named output is a column like any property: `{"property": "HKF88:born_G", "unit": "kJ/mol"}` ([tutorial 6](06-output-tables.md)).

## Change the model of a record

Methods are data. A value of a record is found by its **path**: the section, the symbol of the record, and the keys, for example `/substances/H2O@/methods`. `get` reads it, and `set` changes it in the database. Make the engine after the change.

Here water gets another dielectric model, `HelgesonKirkham74`, in place of the model of the record. The dielectric model changes the volume and the heat capacity of a dissolved ion. Use the model that the parameters of the ion were fitted with.

```{code-cell} ipython3
database = tf.Database(AQ)
print(database.get("/substances/H2O@/methods"))
```

```{code-cell} ipython3
database.set("/substances/H2O@/methods", [{"method": "WaterIAPWS95"}, {"method": "HelgesonKirkham74"}])
hk74_engine = tf.ThermoEngine(database)

print("aq17:", engine.calculationPath("H2O@")["methods"])
print("new: ", hk74_engine.calculationPath("H2O@")["methods"])
```

Compare the volume and the heat capacity of Ca+2 with the dielectric model of aq17 (JohnsonNorton91) and with HelgesonKirkham74:

```{code-cell} ipython3
ion_jn91 = engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2")
ion_hk74 = hk74_engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2")

print("V  (m3/mol)    JohnsonNorton91:", ion_jn91.volume.val, "  HelgesonKirkham74:", ion_hk74.volume.val)
print("Cp (J/(mol K)) JohnsonNorton91:", ion_jn91.heat_capacity_cp.val, "  HelgesonKirkham74:", ion_hk74.heat_capacity_cp.val)
```

To compare models for many records and conditions, a table can do it in one setup: see [Make tables](../how-to/make-tables.md).

## Remove a method

Without the transition, G of Calcite is different. Here the method `LandauHP98` is taken out of the list.

```{code-cell} ipython3
database = tf.Database("mineral-methods.yaml")
methods = database.get("/substances/Calcite/methods")
database.set("/substances/Calcite/methods", [m for m in methods if m["method"] != "LandauHP98"])
without = tf.ThermoEngine(database).thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(result.gibbs_energy.val, "->", without.gibbs_energy.val, "J/mol")
```

## Change a parameter

Change it in the same way. A value that you write in the record replaces the value of the file. Here the parameter `omega` of the model `HKF88` of Ca+2 is changed, and the path goes through the list of methods (`methods/0` is the first).

```{code-cell} ipython3
database = tf.Database(AQ)
print(database.get("/substances/Ca+2/methods/0/parameters"))
```

```{code-cell} ipython3
before = tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val
database.set("/substances/Ca+2/methods/0/parameters/omega", 2.0e5)
after = tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val
print("V:", before, "->", after, "m3/mol")
```

```{code-cell} ipython3
:tags: [remove-cell]
assert after != before
```

To change a value of one engine without a new database, use a parameter handle: [tutorial 2](02-load-data.md).

## Use another implementation of a model

Some methods have more than one implementation. They have the same equations, but they differ in the digits of the constants or in details, to reproduce the results of another code. The [page of each method](../reference/methods/index.md) lists its implementations and the default. For `HKF88` they are `reaktoro` (the default) and `gems`.

A record uses the default, unless it names one with the key `implementation`. A preference of the engine chooses the implementation for all records.

```{code-cell} ipython3
engine = tf.ThermoEngine(AQ)
print("default (reaktoro):", engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2").gibbs_energy.val, "J/mol")

preferences = engine.preferences
preferences.method_implementation = {"HKF88": "gems"}
engine.setPreferences(preferences)
print("gems:              ", engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2").gibbs_energy.val, "J/mol")
```

## Change the options of the engine

The options are in [Options of the engine](../reference/engine-preferences.md). For example, `round_to_uncertainty` is `False` by default, so a value has all its digits. If it is `True`, the engine rounds a value to the decimals of its uncertainty, with the rules of the NEA TDB (Thermochemical Database of the Nuclear Energy Agency). It rounds only the properties that have an uncertainty above 0.

Calcite has no uncertainty in aq17, so give one in the record:

```{code-cell} ipython3
database = tf.Database(AQ)
database.set("/substances/Calcite/standard_state_properties/G", {"value": database.get("/substances/Calcite/standard_state_properties/G"), "uncertainty": 1500.0})
database.set("/substances/Calcite/standard_state_properties/S/uncertainty", 15.0)
engine = tf.ThermoEngine(database)

G = engine.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print("not rounded:", G.val, "+-", G.err)

preferences = engine.preferences
preferences.round_to_uncertainty = True
engine.setPreferences(preferences)
G = engine.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print("rounded:    ", G.val, "+-", G.err)
```

## Change the conventions of the results

G has no absolute zero, so the same substance has different values of G and H in different conventions. A **convention** says what a value refers to. For the zero of water there are two:

- the **geochemical** scale (the default) gives the values of formation from the elements, as the databases of aqueous species do: G of liquid water at 25 degC is about -237 kJ/mol;
- the **steam tables** give G = 0 and S = 0 for liquid water at its triple point (273.16 K).

A convention does not change the units: results are always in J/mol, J/(mol K) and m3/mol. It changes what a value refers to:

- the **energy reference**: from what zero G, H and S are counted (for water: the formation scale of geochemistry, or the steam tables);
- the **standard state**: the standard pressure (1 bar or 1 atm) and, for solutes, the concentration scale (molality, molarity, mole fraction). These change G and S, and with molarity also H, Cp and V.

[Conventions](../reference/conventions.md) has the details, and [Choose conventions](../how-to/choose-conventions.md) the steps.

Here G of water is given on the scale of the steam tables. The change holds from the next calculation.

```{code-cell} ipython3
engine = tf.ThermoEngine(AQ)
print("geochemical: ", engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@").gibbs_energy.val, "J/mol")

conventions = tf.EngineConventions()
conventions.water_reference = "steam_tables"
engine.setConventions(conventions)
print("steam tables:", engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@").gibbs_energy.val, "J/mol")
```

## Use a method alone to calculate

A method, the code of one model such as WaterIAPWS95, calculates without a database and without an engine.

```{code-cell} ipython3
water = tf.methods.solventMethod("WaterIAPWS95", implementation="reaktoro").properties(298.15, 1e5, 0)
print("density", water.density.val, "kg/m3")
born = tf.methods.dielectricMethod("HelgesonKirkham74").properties(298.15, 1e5, water, 0)
print("epsilon", born.epsilon.val)
```

**Next:** [4. Where the files are](04-where-the-files-are.md).
