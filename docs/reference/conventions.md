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
| `standard_state.pressure` | `1bar`, `1atm`, or any pressure | `1bar` |
| `standard_state.concentration_scale` | `molality`, `molarity`, `mole_fraction` | `molality` |
| `energy_reference.elements` | `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, ... | `benson_helgeson` |
| `energy_reference.water` | `helgeson_kirkham_1974`, `codata_1989`, `steam_tables` | `helgeson_kirkham_1974` |
| `energy_reference.ions` | `hydrogen_ion`, `none` | `hydrogen_ion` |

**What G, H and S of a record mean.** The keys of `standard_state_properties` are the same in every energy reference; their
meaning at the reference temperature Tr follows it. NEA data are `benson_helgeson` as published (dfG°m, dfH°m,
S°m); `dfG` and `dfH` are read as G and H of `benson_helgeson`.

| `energy_reference.elements` | G at Tr | H at Tr | S |
|---|---|---|---|
| `benson_helgeson` | dfG° (of formation from the elements) | dfH° | absolute (third law) |
| `gibbs_helmholtz_formation` | dfG° | dfH° | dfS° (of formation) |
| `berman_brown`, `ser`, `assigned_enthalpy`, `dft_formation_0K` | dfH° - Tr S° | dfH° | absolute |

Cp and V are absolute in every energy reference.

**The concentration scales of solutes.** The standard state of a solute is the hypothetical ideal-dilute solute at a
standard composition (NEA TDB-5, Table 1 and 3.1; IUPAC GB4 4.11.1):

| `concentration_scale` | Composition | Standard value | Depends on T and P |
|---|---|---|---|
| `molality` | amount of solute per **mass of solvent**, mol/kg ("0.1 molal", "0.1 m") | m° = 1 mol/kg | no |
| `molarity` | amount of solute per **volume of the solution**, mol/dm3 ("0.1 molar", "0.1 M"); IUPAC: amount concentration | c° = 1 mol/dm3 | yes, through the density of water |
| `mole_fraction` | amount of solute per total amount, x = n/sum(n); IUPAC: amount fraction | x° = 1 | no |

A change to or from `molarity` therefore needs the water record of the database (CNV-002).

**The engine has one inside convention.** Each record says what it is in, and the engine converts it to the canonical convention. So species with different conventions can be in one reaction. Results go out in the convention that you ask for.

**How the conventions differ.** With S_el the sum of the entropies of the elements of the formula at 298.15 K and 1 bar (Dolejš 2013, Eqs. 8, 24):

| Change | G | S | H, Cp, V |
|---|---|---|---|
| `berman_brown` to `benson_helgeson` | + 298.15 S_el | no change | no change |
| `molality` to `mole_fraction` | + RT ln(1000/Mw), Mw in g/mol | - R ln(1000/Mw) | no change |
| `molality` to `molarity` | - RT ln(rho/1000), rho in kg/m3 | follows | H follows rho(T) |
| 1 bar to 1 atm (gases) | + RT ln(1.01325) | - R ln(1.01325) | no change |

Two of these are approximations. `dft_formation_0K` takes a formation energy at 0 K from a DFT calculation as the enthalpy of formation at 298.15 K, with no zero-point or thermal term: an estimate, typically within a few kJ/mol per atom, and the reader warns (CNV-008). `gibbs_helmholtz_formation` is the convention of the Extended UNIQUAC tables: they give dfG° and dfH° at 25 °C and a Cp of the species, and get G at other T from the Gibbs-Helmholtz equation, d(G/T)/dT = -H/T², applied to these values of formation. So the element term is T S_el at every T, which is exact only if the heat capacities of the elements were zero: it equals `benson_helgeson` at Tr, and the two drift apart by (T - Tr) S_el.

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

**The hydrogen ion and the electron.** With `ions: hydrogen_ion` (the default, NEA TDB-5 1.6 and 1.7.e), all properties of
H+(aq) are 0 at every T and P, whatever the energy reference of the elements. The electron (an aqueous solute with the formula
`-` or `e-`) is defined by the standard hydrogen electrode, H+ + e- = 1/2 H2(g) with drG° = 0 at every T:

G(e-, T) = 1/2 G°(H2, g, T, p°) - G°(H+, T) = 1/2 G°(H2, g, T, p°)

and the same for H, S and Cp. The gas is at its standard pressure, so the electron does not depend on P (V = 0). At 25 °C this
gives S = 65.34 J/(mol K) and Cp = 14.42 J/(mol K), the values of the NEA tables. The engine takes H2(g) from the database
(validate warns, CNV-011, if there is none; the record of the electron is then used). With `ions: none` on a record or the
dataset, the records give the values of H+ and e-. Results are always on `hydrogen_ion`.

[Choose conventions](../how-to/choose-conventions.md) gives the steps.
