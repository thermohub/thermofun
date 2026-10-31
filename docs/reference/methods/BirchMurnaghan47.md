# BirchMurnaghan47

Birch-Murnaghan equation of state.

Calculates V(T, P) from the third-order Birch-Murnaghan equation, with alpha(T) = a1 + a2 T + a3/T2 and K(T) linear in T.

Source: Birch (1947); code of M. Gottschalk.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_eos_birch_murnaghan_gott97` |
| Other names | `BirchMurnaghanGottschalk97`, `BirchMurnaghanGott97` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1/K | 1/K | yes |  | thermal expansion, constant term |
| a2 | 1/K2 | 1/K2 | no |  | thermal expansion, term a2 T |
| a3 | K | K | no |  | thermal expansion, term a3 / T2 |
| kt00 | MPa | Pa | yes |  | bulk modulus at the reference temperature |
| dkdt | MPa/K | Pa/K | no |  | temperature derivative of the bulk modulus |
| kp | 1 | 1 | yes |  | pressure derivative of the bulk modulus |
| kpp | 1/MPa | 1/Pa | no |  | second pressure derivative of the bulk modulus; 0: computed from kp |

## Example

No bundled record uses this method.
