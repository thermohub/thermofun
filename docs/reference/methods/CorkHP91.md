# CorkHP91

Compensated Redlich-Kwong equation of state.

Adds the pressure terms of the CORK equation of state to G, H and S, and sets V.

Source: Holland and Powell (1991).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_comp_redlich_kwong_hp91` |
| Other names | `CORK` |

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

From CH4 in `aq17-thermofun.json`.

```yaml
- method: CorkHP91
  units: {pressure: bar}
  parameters:
    Tc: 190.6
    Pc: 46
    omega: 0
```
