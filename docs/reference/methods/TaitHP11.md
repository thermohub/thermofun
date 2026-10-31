# TaitHP11

Tait equation of Holland and Powell.

Calculates V(T, P) from the modified Tait equation with a thermal pressure, and adds the terms to G, H, S and Cp.

Source: Holland and Powell (2011).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_eos_tait_hp11` |
| Other names | `HP11Tait` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| kappa0p | 1 | 1 | yes |  | pressure derivative of the bulk modulus |
| kappa0pp | 1/kbar | 1/Pa | yes |  | second pressure derivative of the bulk modulus |
| n_atoms | 1 | 1 | yes |  | number of atoms in the formula |
| alpha0 | 1/K | 1/K | yes |  | thermal expansion at the reference state |
| kappa0 | kbar | Pa | yes |  | bulk modulus at 298 K |

## Example

No bundled record uses this method.
