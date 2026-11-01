<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Your own data

You learn to read a small database that is written by hand, and to change it. It takes 15 minutes.

**You need:** ThermoFun, and the file `examples/v2/minimal.yaml` of the repository.

## 1. Read the file

`minimal.yaml` has water, Na+, Cl- and halite, and one reaction. Open it. Each substance has:

- `formula`, `class` and `aggregate_state`;
- `standard_state_properties`: G and H in J/mol, S and Cp in J/(mol K), V in m3/mol unless a unit is given;
- `methods`: how to calculate at other T and P.

## 2. Calculate from it

```python
import thermofun as tf

engine = tf.ThermoEngine("examples/v2/minimal.yaml")
sodium = engine.thermoPropertiesSubstance(373.15, 1e7, "Na+")
print(sodium.gibbs_energy.val, "J/mol")
assert sodium.gibbs_energy.sta[0] == tf.Status.calculated
```

`Na+` needs water. `HKF88` reads the density and the dielectric constant from the record `H2O@`, which has an equation of state of water and a dielectric model.

## 3. See how a record is calculated

```python
path = engine.calculationPath("Na+")
assert path["source"] == "methods"
assert path["steps"][0]["role"] == "solvent" and path["steps"][0]["symbol"] == "H2O@"
```

## 4. Calculate the reaction

The reaction has no methods, so its properties come from its reactants.

```python
reaction = engine.thermoPropertiesReaction(298.15, 1e5, "Halite-dissolution")
expected = -256142.0 - 131290.0 + 384212.0                    # J/mol, from the file
assert abs(reaction.reaction_gibbs_energy.val - expected) < 0.01
assert engine.calculationPath("Halite-dissolution")["source"] == "reactants"
```

## 5. Check the file

```python
problems = [d for d in tf.Database("examples/v2/minimal.yaml").validate() if d.severity == tf.Severity.Error]
assert not problems
```

## 6. Change a value

A record can replace a value only for itself. Give the record another method, or change a value with a parameter handle:

```python
omega = engine.parameter("Na+", "HKF88", "omega")
before = engine.thermoPropertiesSubstance(373.15, 1e7, "Na+").gibbs_energy.val
omega.setValue(omega.value() * 1.01)
assert engine.thermoPropertiesSubstance(373.15, 1e7, "Na+").gibbs_energy.val != before
```

**Next:** [Combine methods](03-combine-methods.md). All fields are in [Add data](../how-to/add-data.md).
