<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Conventions

The same substance can have different values for G, because G has no absolute zero. A convention says how the values are defined.

A value is defined by its **standard state**, its **reference temperature** and its **energy reference**. A record or a dataset gives
the standard state and the energy reference in two blocks:

| Block and field | Words | Default |
|---|---|---|
| `standard_state.pressure` | `1bar`, `1atm` | `1bar` |
| `standard_state.concentration_scale` | `molality`, `molarity`, `mole_fraction` | `molality` |
| `energy_reference.elements` | `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, ... | `benson_helgeson` |
| `energy_reference.water` | `helgeson_kirkham_1974`, `codata_1989`, `steam_tables` | `helgeson_kirkham_1974` |

**The engine has one inside convention.** Each record says what it is in, and the engine converts it to the canonical convention. So species with different conventions can be in one reaction. Results go out in the convention that you ask for.

**How the conventions differ.** With S_el the sum of the entropies of the elements of the formula at 298.15 K and 1 bar (Dolejš 2013, Eqs. 8, 24):

| Change | G | S | H, Cp, V |
|---|---|---|---|
| `berman_brown` to `benson_helgeson` | + 298.15 S_el | no change | no change |
| `molality` to `mole_fraction` | + RT ln(1000/Mw), Mw in g/mol | - R ln(1000/Mw) | no change |
| `molality` to `molarity` | - RT ln(rho/1000), rho in kg/m3 | follows | H follows rho(T) |
| 1 bar to 1 atm (gases) | + RT ln(1.01325) | - R ln(1.01325) | no change |

Two of these are approximations. `dft_formation_0K` takes a formation energy at 0 K from a DFT calculation as the enthalpy of formation at 298.15 K, with no zero-point or thermal term: an estimate, typically within a few kJ/mol per atom, and the reader warns (CNV-008). `gibbs_helmholtz_formation` uses T S_el(Tr) for the elements at every T, which is exact only if their heat capacities were zero.

U and A follow H, not G: U = H - P V and A = U - T S. With `benson_helgeson`, A is therefore not G - P V: the two differ by 298.15 S_el. This is the "apparent" Helmholtz and internal energy of Berman (1988) (Dolejš 2013, Eqs. 21-26), the same in every energy reference.

**Water: two anchors and the steam tables.** An equation of state of water gives energies on its own scale, with zero at the
triple point (273.16 K). The anchor ties that scale to the values of formation: the values of liquid water at the triple
point, Gtr, Htr, Str. There are two anchors, and they are data of water, not conventions:

| `energy_reference.water` | G of liquid water at 25 °C, 1 bar | Datasets |
|---|---|---|
| `helgeson_kirkham_1974` (default) | -237181 J/mol | Helgeson and Kirkham (1974); HKF and SUPCRT92 data |
| `codata_1989` | -237140 J/mol | CODATA 1989 key values; NEA TDB data |

The species of a dataset are consistent with its own water inside every reaction with H2O. So the anchor is declared on the
dataset (or on the water record), is used inside the calculation, and is never converted to the other one: asking the engine
for the other anchor is an error. For `codata_1989` the engine takes the triple-point values of the equation of state in use:
the `NEA_HGK` set for WaterHGK84 and WaterLVS83, the `NEA_IAPWS` set for WaterIAPWS95 (both in `WaterTriplePoint.yaml`, from
the CODATA values at 298.15 K and the difference to the triple point that each equation gives).

The **steam tables** are a scale, not an anchor: S = 0 and U = 0 for liquid water at its triple point. They are converted both
ways, from the anchor of the dataset. G on the two scales differs by a constant and a term in T, because S differs by a
constant:

G(anchor) = G(steam) + Gtr - Str (T - Ttr)

A water record written on the steam scale declares `energy_reference: {water: steam_tables}`; results on the steam scale are
asked for with the conventions of the engine.

[Choose conventions](../how-to/choose-conventions.md) gives the steps.
