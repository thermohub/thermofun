<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Routes

**Related:** [See how a result is calculated](../how-to/see-how-a-result-is-calculated.md) · [Results](results.md) · [How methods combine](how-methods-combine.md) · [Conventions](conventions.md) · [Batch setups](batch.md)

Every result says how it was calculated: its **route** (decision 0030). Each value of a result points to it,
`x.info.route`, and so does the result, `result.info.route`. `engine.route(symbol)` gives the same structure without numbers,
before calculating. A route prints in full as YAML.

## The fields

| Field | Meaning |
|---|---|
| `symbol`, `kind` | The record; `substance` or `reaction`. |
| `T`, `P` | The point, P as used (the saturation pressure for P = 0); none in `engine.route`. |
| `by` | Where the values come from: `methods` (the methods of the record), `reaction` (its defining reaction), `reactants` (a reaction from its species), `water` (the equation of state of water), `reference` (the values of the record), `hydrogen_ion` (0 by convention), `hydrogen_electrode` (the electron, from 1/2 H2(g)). |
| `contribution` | What the source gives, if it is not the methods: the defining reaction, the reactants, the values of the record, the hydrogen electrode. |
| `methods[]` | `method`, `implementation`, `region` (water and dielectric methods: the region of the method used), `contribution` (what the method adds to, or replaces in, G, H, S, Cp, V), `terms`, `variables`. |
| `needs[]` | `role` (`solvent`, `defining_reaction`, `reactant`), `coefficient`, `route` (the route of that record). |
| `conversions[]` | `what` (`convention`, `gas_options`, `fallback`, `rounding`), `detail` (`berman_brown`, `1atm`, `NASA9 to canonical`, ...), `change`. |
| `notes` | In `engine.route`: what cannot be calculated. The notes of a result are in `result.info.notes`; each names its `method` and the records it came through (`via`). |
| `total()` | The sum of the contribution of the source, of the methods and of the conversions. |

The values are in SI: J/mol, J/(mol K), m3/mol. A reaction has the same keys G, H, S, Cp, V for dG, dH, dS, dCp, dV.

**For each property the result is the sum of its route:** `contribution` + the contributions of the methods + the changes of
the conversions. A method that sets a property (the base method) replaces what came before it, so its contribution holds the
whole value. A test checks the sum on every bundled record.

## Before calculating

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
print(engine.route("Calcite"))
```

Output:

```text
symbol: Calcite
kind: substance
by: methods
methods:
  - method: CpPolynomial
  - method: LandauHP98
  - method: MurnaghanHP98
```

A substance and a reaction can have the same symbol (a substance and its defining reaction often do): `route` gives the
substance, and `engine.route(symbol, "reaction")` the reaction. `routeTable` and `parameterSensitivities` take the same `kind`.

## The route of a result

A gas with a fluid equation of state: the base method gives the ideal gas, and `CorkHP91` adds the rest.

```python
co2 = engine.thermoPropertiesSubstance(373.15, 1e7, "CO2")
print(co2.info.route)
```

Output:

```text
symbol: CO2
kind: substance
T: 373.15
P: 10000000
by: methods
methods:
  - method: CpPolynomial
    contribution: {G: -410711.083398, H: -390610.984976, S: 222.3603176, Cp: 40.1759294265, V: 0}
  - method: CorkHP91
    contribution: {G: 13484.0566004, H: -3102.45511011, S: -44.4499844848, Cp: 25.0872435764, V: 0.000228387192622}
```

A convention of the results is a conversion: here the energies of quartz counted from the elements as Berman and Brown do.

```python
conventions = engine.conventions
conventions.energy_reference.elements = "berman_brown"
engine.conventions = conventions
quartz = engine.thermoPropertiesSubstance(373.15, 1e7, "Quartz")
print(quartz.info.route.conversions[0].detail, round(quartz.info.route.conversions[0].change["G"], 3))
print(round(quartz.info.route.total()["G"] - quartz.gibbs_energy.val, 6))
```

Output:

```text
berman_brown -66770.096
0.0
```

A reaction without methods comes from its reactants: its route needs each of them, with its coefficient and its route.

```python
reaction = engine.thermoPropertiesReaction(298.15, 1e5, "Calcite = Ca+2 + CO3-2")
print(reaction.info.route.by, reaction.info.route.needs[2].route.symbol, reaction.info.route.needs[2].coefficient)
```

Output:

```text
reactants Calcite -1.0
```

## At many points

`engine.routeTable(T, P, symbol, part)` gives one row for each (T, P); T or P can be one number. `part`:

| `part` | Columns |
|---|---|
| `contribution` (default) | `G`, `H`, `S`, `Cp`, `V` (the result); `<by>:G` (the source, if it is not the methods); `<method>:G`; `<what>(<detail>):G` for each conversion. The columns of a property add up to it. |
| `terms` | `<method>:<name>`: the parts of the contribution of a method to one property, as its catalog entry names them (HKF88: `born_G`, ...). |
| `variables` | `<method>:<name>`: the quantities of the model that the terms come from (HKF88: `g`, `re`, `omega`, `omega_reference`). |

The formula of a method gives its terms and variables as it calculates the values, so they cost no other evaluation of the
model; the route of every result has them. A batch calculation takes the same columns as properties:
`contribution:<method>:<property>`, `terms:<method>:<name>`, `variables:<method>:<name>`.

## In C++

`ThermoFun::Route` and its parts are in `ThermoFun/Common/Route.h`; `x.info.route` is a `std::shared_ptr<const Route>`.
`ThermoEngine::route(symbol, kind)`, `ThermoEngine::routeTable(T, P, symbol, part, kind)` (a `RouteTable`: `columns`, `T`, `P`,
`values`), and `ThermoFun::toYaml(route)`. The static part of a route is built once for each record; a cached result keeps its route.
