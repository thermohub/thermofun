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

**Related:** [List of methods](../reference/methods/index.md) · [How methods combine](../reference/how-methods-combine.md) · [Choose water models](../how-to/choose-water-models.md) · [Choose conventions](../how-to/choose-conventions.md) · [See how a result is calculated](../how-to/see-how-a-result-is-calculated.md) · [Add a method as data](../how-to/add-a-method-as-data.md) · [Options of the engine](../reference/engine-preferences.md) · [Conventions](../reference/conventions.md)

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

The route of a record names its methods and the records it needs (decision 0030). Ca+2 uses `HKF88`, which needs the water record `H2O@` with its two models:

```{code-cell} ipython3
print(engine.route("Ca+2"))
```

## Combine methods

A record can have several methods. In `mineral-methods.yaml`, Calcite has three:

- `CpPolynomial` is the base method. It gives G, H, S and Cp from the heat capacity.
- `LandauHP98` is an increment. It adds a phase transition.
- `MurnaghanHP98` is an increment. It gives the volume and the term of the pressure.

The route of the result gives what each method contributes, in J/mol, J/(mol K) and m3/mol. The contributions add up to the result.

```{code-cell} ipython3
minerals = tf.ThermoEngine("mineral-methods.yaml")
result = minerals.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(result.info.route)
```

```{code-cell} ipython3
:tags: [remove-cell]
route = result.info.route
assert [m.method for m in route.methods] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
assert abs(route.total()["G"] - result.gibbs_energy.val) < 1e-6
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

`routeTable` gives the route at as many T and P as you like, one row for each point. A column such as `LandauHP98:G` is what that method contributed to G, in J/mol. A conversion after the methods (a convention of the results, an option of gases) has its own column, `<what>(<detail>):G`. The columns of a property add up to its value `G`. A method that sets a property (the base method) also removes what was there before it, so its column holds the whole value.

```{code-cell} ipython3
contributions = pd.DataFrame(minerals.routeTable([473.15, 800.0, 1300.0], 1e7, "Calcite"))
contributions[["T", "G", "CpPolynomial:G", "LandauHP98:G", "MurnaghanHP98:G"]]
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(contributions["G"][0] - contributions[["CpPolynomial:G", "LandauHP98:G", "MurnaghanHP98:G"]].iloc[0].sum()) < 1e-6
```

## Look inside a method

A model is made of parts. A method can give two kinds: **terms**, the named parts of its contribution to one property, and **variables**, the quantities of the model that the terms come from. `HKF88`, the model of aqueous species, has the Born terms of G, H, S, Cp and V as terms, and the g function, the effective radius of the ion and the Born coefficient as variables. The names and units are on the [page of the method](../reference/methods/HKF88.md), and in `tf.methods.catalog("HKF88")`.

```{code-cell} ipython3
aqueous = tf.ThermoEngine(AQ)
T = [298.15, 373.15, 473.15, 573.15]                                   # K
variables = pd.DataFrame(aqueous.routeTable(T, 1e8, "Ca+2", "variables"))   # at 1000 bar
variables[["T", "HKF88:g", "HKF88:re", "HKF88:omega"]]
```

```{code-cell} ipython3
terms = pd.DataFrame(aqueous.routeTable(T, 1e8, "Ca+2", "terms"))
terms[["T", "HKF88:born_G", "HKF88:born_V"]]
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(variables["HKF88:g"][0]) < 1e-10 and variables["HKF88:g"][3] < 0.0
```

The g function is 0 at low T and becomes negative at high T, where it shrinks the effective radius of the ion and so changes its Born coefficient. Only some methods have terms and variables; for the others the rows have only T and P. They cost one more evaluation of the model, so the route of a result has them only with the option `route_terms_and_variables`. In a batch calculation, they are properties like any other: `{"name": "terms:HKF88:born_G", "unit": "kJ/mol"}` ([tutorial 5](05-batch-calculations.md)).

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

print(engine.route("H2O@"))
print(hk74_engine.route("H2O@"))
```

Compare the volume and the heat capacity of Ca+2 with the dielectric model of aq17 (JohnsonNorton91) and with HelgesonKirkham74:

```{code-cell} ipython3
ion_jn91 = engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2")
ion_hk74 = hk74_engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2")

print("V  (m3/mol)    JohnsonNorton91:", ion_jn91.volume.val, "  HelgesonKirkham74:", ion_hk74.volume.val)
print("Cp (J/(mol K)) JohnsonNorton91:", ion_jn91.heat_capacity_cp.val, "  HelgesonKirkham74:", ion_hk74.heat_capacity_cp.val)
```

To compare models for many records and conditions, a batch calculation does it in one setup, with the difference: see [Batch calculations](05-batch-calculations.md#compare-two-models-and-two-databases).

## Remove a method

Without the transition, G of Calcite is different. Here the method `LandauHP98`, the second of the list (`methods/1`), is removed by its path.

```{code-cell} ipython3
database = tf.Database("mineral-methods.yaml")
print(database.get("/substances/Calcite/methods/1/method"))
database.remove("/substances/Calcite/methods/1")
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

G has no absolute zero, so the same substance has different values of G and H in different conventions. A **convention** says what a value refers to. For water there are three words, two anchors and one scale:

- the anchor **helgeson_kirkham_1974** (the default) gives the values of formation from the elements, as HKF and SUPCRT92 data do: G of liquid water at 25 degC is -237181 J/mol;
- the anchor **codata_1989** gives the CODATA 1989 values, as NEA data do: -237140 J/mol;
- the **steam tables** give U = 0 and S = 0 for liquid water at its triple point (273.16 K).

The two anchors are data of water, not conventions: the species of a dataset are consistent with its own water in every reaction with H2O. A dataset declares its anchor (`energy_reference: {water: codata_1989}`), and the engine never converts between the two. Only the steam tables are asked for as a convention of the results.

A convention does not change the units: results are always in J/mol, J/(mol K) and m3/mol. It changes what a value refers to:

- the **energy reference**: from what zero G, H and S are counted (for water: the values of formation, or the steam tables);
- the **standard state**: the standard pressure (1 bar or 1 atm) and, for solutes, the concentration scale (molality, molarity, mole fraction). These change G and S, and with molarity also H, Cp and V.

[Conventions](../reference/conventions.md) has the details, and [Choose conventions](../how-to/choose-conventions.md) the steps.

Here G of water is given on the scale of the steam tables. The change holds from the next calculation.

```{code-cell} ipython3
engine = tf.ThermoEngine(AQ)
print("anchored:    ", engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@").gibbs_energy.val, "J/mol")

conventions = tf.EngineConventions()
conventions.energy_reference.water = "steam_tables"
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
