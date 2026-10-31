# PRSV86

Peng-Robinson-Stryjek-Vera equation of state.

Adds the pressure terms of the PRSV equation of state to G, H and S, and sets V.

Source: Stryjek and Vera (1986); Proust and Vera (1989).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_prsv` |
| Other names | `PRSV` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| k1 | 1 | 1 | no |  | interaction parameter k1 |
| k2 | 1 | 1 | no |  | interaction parameter k2 |
| k3 | 1 | 1 | no |  | interaction parameter k3 |

## Example

From CH4 in `mines16-thermofun.json`.

```yaml
- method: PRSV86
  units: {pressure: bar}
  parameters:
    Tc: 190.555
    Pc: 45.95
    omega: 0.01045
    k1: -0.00159
```
