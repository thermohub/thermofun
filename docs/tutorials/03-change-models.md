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

You learn which models there are, how a record combines them, and how to change the model of a record and the options of the engine. It takes 20 minutes.

**You need:** ThermoFun, and the files `aq17-thermofun.json` from `Resources/databases/` and `mineral-methods.yaml` from `examples/v2/` of the repository, in your folder.

```{code-cell} ipython3
import json
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
for symbol in ["H2O@", "Ca+2", "Calcite"]:
    print(symbol, engine.calculationPath(symbol)["methods"])
```

## Combine methods

A record can have several methods. In `mineral-methods.yaml`, Calcite has three:

- `CpPolynomial` is the base method. It gives G, H, S and Cp from the heat capacity.
- `LandauHP98` is a correction. It adds a phase transition.
- `MurnaghanHP98` is a correction. It gives the volume and the term of the pressure.

`explain` gives the state after each method, in J/mol and m3/mol.

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

The engine sets the order from what each method does, so the order in the file does not change the result.

```{code-cell} ipython3
document = json.loads(tf.Database("mineral-methods.yaml").toJson(readable=False))
document["substances"]["Calcite"]["methods"].reverse()
same = tf.ThermoEngine(tf.Database(json.dumps(document))).thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(same.gibbs_energy.val == result.gibbs_energy.val)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert same.gibbs_energy.val == result.gibbs_energy.val
```

The reasons are in [How methods combine](../explanation/how-methods-combine.md).

## Change the model of a record

Methods are data. Take the records as text, change them, and make a new database. Here water gets another dielectric model.

The dielectric model changes the volume and the heat capacity of a dissolved ion. Use the model that the parameters of the ion were fitted with.

```{code-cell} ipython3
def with_water_methods(path, *methods):
    records = json.loads(tf.Database(path).toJson(False))
    records["substances"]["H2O@"]["methods"] = [{"method": m} for m in methods]
    return tf.ThermoEngine(tf.Database(json.dumps(records)))

johnson_norton = tf.ThermoEngine(AQ)
helgeson_kirkham = with_water_methods(AQ, "WaterIAPWS95", "HelgesonKirkham74")
bradley_pitzer = with_water_methods(AQ, "WaterIAPWS95", "BradleyPitzer79")

print(helgeson_kirkham.calculationPath("H2O@")["methods"])

rows = []
for label, e in [("JohnsonNorton91", johnson_norton), ("HelgesonKirkham74", helgeson_kirkham), ("BradleyPitzer79", bradley_pitzer)]:
    x = e.thermoPropertiesSubstance(373.15, 1e8, "Ca+2")
    rows.append({"dielectric model": label, "V (cm3/mol)": x.volume.val * 1e6, "Cp (J/K/mol)": x.heat_capacity_cp.val})
pd.DataFrame(rows)
```

A table can do the same comparison in one setup, with `compare`:

```{code-cell} ipython3
setup = {
    "database": AQ,
    "conditions": {"T": {"values": [100], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
    "tables": [{
        "name": "t", "records": ["Ca+2", "Na+"],
        "columns": [{"property": "V", "unit": "cm3/mol", "digits": 4}, {"property": "Cp", "digits": 2}],
        "compare": [{"label": "JN91"}, {"label": "HK74", "replace": {"H2O@": {"JohnsonNorton91": "HelgesonKirkham74"}}}],
    }],
}
print(tf.table(setup)["t"].to_text("md"))
```

## Remove a method

Without the transition, G of Calcite is different.

```{code-cell} ipython3
document["substances"]["Calcite"]["methods"] = [m for m in document["substances"]["Calcite"]["methods"] if m["method"] != "LandauHP98"]
without = tf.ThermoEngine(tf.Database(json.dumps(document))).thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(result.gibbs_energy.val, "->", without.gibbs_energy.val, "J/mol")
```

## Change a parameter

Change it in the same way. A value that you write in the record replaces the value of the file.

```{code-cell} ipython3
records = json.loads(tf.Database(AQ).toJson(False))
before = tf.ThermoEngine(tf.Database(json.dumps(records))).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val

print(json.dumps(records["substances"]["Ca+2"]["methods"][0]["parameters"])[:150], "...")
records["substances"]["Ca+2"]["methods"][0]["parameters"]["omega"] = 2.0e5
after = tf.ThermoEngine(tf.Database(json.dumps(records))).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val
print("V:", before, "->", after, "m3/mol")
```

## Use another implementation of a model

Some methods have more than one implementation, to give the results of another code. A preference of the engine chooses the implementation, for all records. The catalog above lists them.

```{code-cell} ipython3
engine = tf.ThermoEngine(AQ)
preferences = engine.preferences
print("as in the record:", engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2").gibbs_energy.val, "J/mol")
preferences.method_implementation = {"HKF88": "gems"}
engine.setPreferences(preferences)
print("gems:            ", engine.thermoPropertiesSubstance(373.15, 1e8, "Ca+2").gibbs_energy.val, "J/mol")
```

## Change the options of the engine

Here a value is rounded to the decimals of its uncertainty, with the rules of the NEA TDB (Thermochemical Database of the Nuclear Energy Agency). All options are in [Options of the engine](../reference/engine-preferences.md).

```{code-cell} ipython3
records = json.loads(tf.Database(AQ).toJson(False))
properties = records["substances"]["Calcite"]["standard_state_properties"]
properties["G"] = {"value": properties["G"], "uncertainty": 1500.0}
properties["S"] = {"value": properties["S"], "uncertainty": 0.5}
engine = tf.ThermoEngine(tf.Database(json.dumps(records)))

G = engine.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print(G.val, "+-", G.err)

preferences = engine.preferences
preferences.round_to_uncertainty = True
engine.setPreferences(preferences)
G = engine.thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy
print(G.val, "+-", G.err)
```

## Change the conventions of the results

For example, give G and H of water on the scale of the steam tables, not on the geochemical scale. The change holds from the next calculation.

```{code-cell} ipython3
engine = tf.ThermoEngine(AQ)
print("geochemical: ", engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@").gibbs_energy.val, "J/mol")

conventions = tf.EngineConventions()
conventions.water_reference = "steam_tables"
engine.setConventions(conventions)
print("steam tables:", engine.thermoPropertiesSubstance(298.15, 1e5, "H2O@").gibbs_energy.val, "J/mol")
```

## Use a model alone

You do not need a database to run a water model.

```{code-cell} ipython3
water = tf.methods.solventMethod("WaterIAPWS95", implementation="reaktoro").properties(298.15, 1e5, 0)
print("density", water.density.val, "kg/m3")
born = tf.methods.dielectricMethod("HelgesonKirkham74").properties(298.15, 1e5, water, 0)
print("epsilon", born.epsilon.val)
```

**Next:** [4. Where the files are](04-where-the-files-are.md).
