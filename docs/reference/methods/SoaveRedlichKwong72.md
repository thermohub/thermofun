# SoaveRedlichKwong72

Soave-Redlich-Kwong equation of state.

Adds the pressure terms of the SRK equation of state to G, H and S, and sets V.

Source: Soave (1972); Soave (1993).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_soave_redlich_kwong` |
| Other names | `SRK` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| N | 1 | 1 | no |  | empirical parameter of the attraction term |

## Example

No bundled record uses this method.
