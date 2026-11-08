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

`ThermoScalar` has `+ - * /` and `**`. They carry the derivatives, the errors and the status.

## A substance

`thermoPropertiesSubstance(T, P, symbol)`, with T in K and P in Pa.

| Property | Unit |
|---|---|
| `gibbs_energy`, `enthalpy`, `internal_energy`, `helmholtz_energy` | J/mol |
| `entropy`, `heat_capacity_cp`, `heat_capacity_cv` | J/(mol K) |
| `volume` | J/bar. 1 J/bar is 1e-5 m3/mol. |

## A reaction

`thermoPropertiesReaction(T, P, symbol)`. The symbol can be an equation. The properties are `reaction_` and the names above, and `log_equilibrium_constant` and `ln_equilibrium_constant`.

## Water

- `propertiesSolvent(T, P, symbol)`: `density` in kg/m3, its derivatives, `Alpha`, `Beta`, `dAldT`, `dynamic_viscosity`, `surface_tension` and others.
- `electroPropertiesSolvent(T, P, symbol)`: `epsilon`, its derivatives, and the Born functions `bornZ`, `bornY`, `bornQ`, `bornX`.

## A pressure of 0

P = 0 means the saturation pressure of the water at this T, for every record of the calculation, also a mineral or a gas. `engine.saturationPressure(T)` gives its value in Pa: from the water record of the database, else from the Wagner-Pruss curve. It exists only below the critical temperature of water, 647.096 K: at or above it, P = 0 is an error. A negative P, T <= 0 and NaN are errors too.

## Changes between calculations

A change of the preferences, the conventions or the data applies from the next calculation. A reference from `database.substance(...)` is old after a calculation: call it again.
