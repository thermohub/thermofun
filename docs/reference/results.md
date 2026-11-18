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
| `sta` | The status and a message: `notdefined`, `read`, `assigned`, `calculated`, `initialized`. |
| `outside_range` | True if a model calculated the value outside its range (of T, P, the density of water, its T intervals or its table). The status stays `calculated`; the message (`sta[1]`) starts with "Outside the range of the model:" and says which range. In C++: `ThermoFun::outsideRange(x)` (`Common/OutsideRange.h`). |

`ThermoScalar` has `+ - * /` and `**`. They carry the derivatives, the errors and the status.

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
