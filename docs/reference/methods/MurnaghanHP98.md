# MurnaghanHP98

Murnaghan equation of Holland and Powell.

Calculates V(T, P) from the Murnaghan equation, and adds the terms to G, H, S and Cp.

Source: Holland and Powell (1998).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_eos_murnaghan_hp98` |
| Other names | `HP98Min` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| alpha0 | 1/K | 1/K | yes |  | thermal expansion at the reference state |
| kappa0 | kbar | Pa | yes |  | bulk modulus at 298 K |

## Example

From Akermanite in `aq17-thermofun.json`.

```yaml
- method: MurnaghanHP98
  units: {pressure: kbar}
  parameters:
    alpha0: 5.08e-05
    kappa0: 1420
```
