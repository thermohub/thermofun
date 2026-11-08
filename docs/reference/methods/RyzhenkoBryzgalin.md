# RyzhenkoBryzgalin

Model of Ryzhenko and Bryzgalin.

Calculates log K of a complex from an electrostatic model.

Source: Ryzhenko and Bryzgalin.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_eos_ryzhenko_gems` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| pK298 | 1 | 1 | yes |  | pK at 298.15 K |
| A | 1 | 1 | yes |  | coefficient A |
| B | 1 | 1 | yes |  | coefficient B |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, rho: g/cm3. They are not converted.

Log K of a complex from an electrostatic model, as a function of the ionization of water. pKw(T, rho) = -(a + b/T + c/T2 + d/T3 + (e + f/T + g/T2) log10(rho)), with T in K and rho in g/cm3. RHO298: the density of water at 298.15 K, g/cm3. zzH2O: the term of water in the model, a number. pK298, A and B belong to each reaction.

Source: Marshall W.L., Franck E.U. (1981) J. Phys. Chem. Ref. Data 10, 295-304 (a to g, the ionization of water); Borisov M.V., Shvarov Yu.V. (1992); Shvarov Yu.V., Bastrakov E.N. (1999) AGSO Record 1999/25 (the model, zzH2O = 1.0107); both as written in Dolejs D. (2013) Rev. Mineral. Geochem. 76, 35-79, Eqs. 43-44.

Names: `a`, `b`, `c`, `d`, `e`, `f`, `g`, `RHO298`, `zzH2O`.

## Example

No bundled record uses this method.
