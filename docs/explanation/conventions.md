<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Conventions

The same substance can have different values for G, because G has no absolute zero. A convention says how the values are defined.

| Field | Words | Default |
|---|---|---|
| `energy_reference` | `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, ... | `benson_helgeson` |
| `concentration_scale` | `molality`, `mole_fraction`, `molarity` | `molality` |
| `water_reference` | `geochemical`, `steam_tables` | `geochemical` |
| `standard_pressure` | 1 bar, 1 atm | 1 bar |

**The engine has one inside convention.** Each record says what it is in, and the engine converts it to the canonical convention. So species with different conventions can be in one reaction. Results go out in the convention that you ask for.

**How the conventions differ.** With S_el the sum of the entropies of the elements of the formula at 298.15 K and 1 bar (Dolejš 2013, Eqs. 8, 24):

| Change | G | S | H, Cp, V |
|---|---|---|---|
| `berman_brown` to `benson_helgeson` | + 298.15 S_el | no change | no change |
| `molality` to `mole_fraction` | + RT ln(1000/Mw), Mw in g/mol | - R ln(1000/Mw) | no change |
| `molality` to `molarity` | - RT ln(rho/1000), rho in kg/m3 | follows | H follows rho(T) |
| 1 bar to 1 atm (gases) | + RT ln(1.01325) | - R ln(1.01325) | no change |

U and A follow H, not G: U = H - P V and A = U - T S. With `benson_helgeson`, A is therefore not G - P V: the two differ by 298.15 S_el. This is the "apparent" Helmholtz and internal energy of Berman (1988) (Dolejš 2013, Eqs. 21-26), the same in every energy reference.

**Water on two scales.** The steam tables set S = 0 and G = 0 for liquid water at its triple point, at 273.16 K. The geochemical scale uses the values of formation, with an absolute S. G on the two scales differs by a constant and a term in T, because S differs by a constant:

G(geochemical) = G(steam) + Gtr - Str (T - Ttr)

The values of the triple point come from a reference at 298.15 K and 1 bar, and from the difference to the triple point that the equation of state gives. `WaterTriplePoint.yaml` has three sets. The `NEA_HGK` and `NEA_IAPWS` sets use the CODATA 1989 values.

[Choose conventions](../how-to/choose-conventions.md) gives the steps.
