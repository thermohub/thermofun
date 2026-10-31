# ReactionVolumePolynomial

Volume of reaction, T and P.

Calculates dV(T, P) from expansion and compressibility terms, and adds the terms to log K, dG and dH.

| | |
|---|---|
| Works on | reaction |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `dr_volume_fpt` |
| Other names | `DrVolumeFT` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1/K | 1/K | yes |  | thermal expansion, first coefficient |
| a2 | 1/K2 | 1/K2 | yes |  | thermal expansion, second coefficient |
| a3 | 1/K3 | 1/K3 | yes |  | thermal expansion, third coefficient |
| b1 | 1/bar | 1/Pa | yes |  | compressibility, first coefficient |
| b2 | 1/bar2 | 1/Pa2 | yes |  | compressibility, second coefficient |

## Example

No bundled record uses this method.
