<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Results

## A value

Each property is a `ThermoScalar`.

| Member | Meaning |
|---|---|
| `val` | The value. |
| `ddt`, `ddp` | The derivatives by T and by P. |
| `err` | The uncertainty. It is 0 if the data give none. |
| `sta` | The status and a message: `notdefined`, `read`, `assigned`, `calculated`, `initialized`. The message is the text of the notes in `info`. |
| `info` | What the value says about itself: see [Info](#info). |

`ThermoScalar` has `+ - * /` and `**`. They carry the derivatives, the errors, the status and the notes of range and
uncertainty.

## Info

`info` of a value:

| Member | Meaning |
|---|---|
| `validity` | `Validity.within`: inside the range of every model and data that the value depends on. `Validity.extrapolated`: calculated, but one of them is used outside its range; the notes say which. `Validity.undefined`: there is no value. |
| `status` | `notdefined`, `read`, `assigned` or `calculated`, as in `sta`. |
| `error` | Whether `err` can be trusted. `ErrorState.known`: the uncertainty of the data, propagated. `ErrorState.partial`: a contribution could not be propagated, and `err` is not a number. `ErrorState.unknown`: the data give no uncertainty; `err` is 0, which does not mean that the value is exact. |
| `route` | How the result was calculated: `route.by` is where the values came from (`methods`, `reaction` for a record defined by a reaction, `reactants` for a reaction from its species, `reference`, `water`, `hydrogen_ion`, `hydrogen_electrode`), with the methods and their contributions, the records it needed and the conversions. See [Routes](routes.md). |
| `pressure` | The pressure used, in Pa. At P = 0 it is the saturation pressure of water. |
| `notes` | The list of notes behind the other members. |

`validity`, `status` and `error` are made from the notes, the status and `err`, so they always agree with them. A result
(of a substance, a reaction or water) has `info` too: the notes of all its values, and `extrapolated` if one of them is.

A note:

| Member | Meaning |
|---|---|
| `kind` | `Note.Kind.Range`: a value outside the range of a model. `Note.Kind.Source`: where the values come from, for example "Calculated from the reaction components". `Note.Kind.Undefined`: what is missing for a value that is not defined, for example "H0 of component C". `Note.Kind.Uncertainty`: a contribution to `err` could not be propagated. |
| `severity` | `Note.Severity.Soft`: the value is calculated, use it with care. `Note.Severity.Hard`: the model gives no value. |
| `method` | The method that made the note, for example `JohnsonNorton91`. |
| `text` | What is outside, and of what. |
| `via` | The path of a note that comes from another record, nearest first. A solute calculated on water that is outside the range of its model gets the note of the water, with `via` `['the water']`. |
| `properties` | The properties the note is about: `G`, `H`, `S`, `Cp`, `V`. Empty: all of them. |

A range note travels with the value: a method that is inside its own range, but uses one that is outside, says so. A
reaction calculated from its species gets their range notes (`via` the species, then its path), and so does a record defined
by a reaction (`via` `the reaction R` and the other species). Source and undefined notes stay with their record. The range
of a model is on its [method page](methods/index.md).

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
G = engine.thermoPropertiesSubstance(1373.15, 1e8, "Na+").gibbs_energy
print(G.info.validity, G.info.route.by, G.info.pressure)
print(G.info.notes[0])
print(G.info.notes[1])
print(G.info.notes[2])
```

Output:

```text
Validity.extrapolated methods 100000000.0
Outside the range of the model: HKF model: outside of 600-1400 kg/m3 density of pure H2O interval
Outside the range of the model: via the water: WaterIAPWS95: T = 1373.15 K, P = 1000 bar, outside its range: T 251.165 to 1273 K, P up to 10000 bar (Wagner and Pruss (2002), Sect. 6.3.1: from the melting curve to 1273 K, up to 1000 MPa)
Outside the range of the model: via the water: JohnsonNorton91: T = 1373.15 K, P = 1000 bar, outside its range: T 273.15 to 1273.15 K, P up to 5000 bar (Johnson et al. (1992), SUPCRT92: 0 to 1000 degC, 1 to 5000 bar)
```

The first note is of the HKF model itself; the other two come from the water, which is outside the range of its equation
of state and of its dielectric model at 1100 degC.

Each note names its method and the records it came through (`via`). The route of the result (`G.info.route`)
says how it was calculated; the notes are not in the route.

In C++: `x.info` (`Reaktoro_::Info`: `notes`, `route`, `pressure`) of each `ThermoScalar`, `ThermoFun::validity(x)` and
`ThermoFun::errorState(x)` in `Common/OutsideRange.h`, and `info` of `ThermoPropertiesSubstance`,
`ThermoPropertiesReaction`, `PropertiesSolvent` and `ElectroPropertiesSolvent`. The notes are `ThermoFun::Note` in
`Common/Notes.h`.

## A substance

`thermoPropertiesSubstance(T, P, symbol)`, with T in K and P in Pa.

| Property | Unit |
|---|---|
| `gibbs_energy`, `enthalpy`, `internal_energy`, `helmholtz_energy` | J/mol |
| `entropy`, `heat_capacity_cp`, `heat_capacity_cv` | J/(mol K) |
| `volume` | m3/mol (J/bar before 1.0; 1 J/bar is 1e-5 m3/mol). |

## A reaction

`thermoPropertiesReaction(T, P, symbol)`. The symbol can be an equation. The properties are `reaction_` and the names above, and `log_equilibrium_constant` and `ln_equilibrium_constant`.

## Water

- `propertiesSolvent(T, P, symbol)`: `density` in kg/m3 and its derivatives, `Alpha` in 1/K, `Beta` in 1/Pa, `dAldT` in 1/K2,
  `Albe` (alpha/beta) in Pa/K. The GEMS water (`WaterHGK84` and `WaterLVS83`) also gives `speed_of_sound` in m/s,
  `dynamic_viscosity` in Pa s, `thermal_conductivity` in W/(m K), `surface_tension` in N/m (at saturation), `Tdiff` and
  `Visck` in m2/s, `Prndtl`; the other water methods give 0 for these.
- `electroPropertiesSolvent(T, P, symbol)`: `epsilon`, its derivatives, and the Born functions `bornZ`, `bornY`, `bornQ`, `bornX`.

## A pressure of 0

P = 0 means the saturation pressure of the water at this T, for every record of the calculation, also a mineral or a gas. `engine.saturationPressure(T)` gives its value in Pa: from the water record of the database, else from the Wagner-Pruss curve. It exists only below the critical temperature of water, 647.096 K: at or above it, P = 0 is an error. A negative P, T <= 0 and NaN are errors too.

## Changes between calculations

A change of the preferences, the conventions or the data applies from the next calculation. A reference from `database.substance(...)` is old after a calculation: call it again.
