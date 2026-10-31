# PengRobinson78

Peng-Robinson equation of state.

Adds the pressure terms of the Peng-Robinson equation of state (1978 form) to G, H and S, and sets V.

Source: Peng and Robinson (1976, 1978).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_peng_robinson78` |
| Other names | `PR78` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| N | 1 | 1 | no |  | empirical parameter of the attraction term |

## Example

No bundled record uses this method.
