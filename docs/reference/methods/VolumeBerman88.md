# VolumeBerman88

Molar volume of Berman.

Calculates V(T, P) from expansion and compressibility terms, and adds the terms to G, H and S.

Source: Berman (1988).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_equation_berman88` |
| Other names | `Berman88Vol` |

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
