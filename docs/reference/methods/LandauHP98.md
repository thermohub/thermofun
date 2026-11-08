# LandauHP98

Landau transition of Holland and Powell.

Adds the Landau term of a phase transition to G, H, S and Cp, and sets V. Implementation consistent (the default): the volume term is integrated from the reference pressure, and has its Cp. Implementation gems: from P = 0, without that Cp.

Source: Holland and Powell (1998).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `landau_holland_powell98` |
| Other names | `HP98Landau` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc0 | Cel | K | yes |  | critical temperature at 1 bar |
| Smax | J/(mol.K) | J/(mol.K) | yes |  | maximum entropy of the transition |
| Vmax | J/(mol.bar) | m3/mol | yes |  | maximum volume of the transition |
| alpha0 | 1/K | 1/K | no |  | thermal expansion at the reference state |
| kappa0 | kbar | Pa | no |  | bulk modulus at 298 K |

## Example

From Albite in `aq17-thermofun.json`.

```yaml
- method: LandauHP98
  units: {pressure: kbar}
  parameters:
    Tc0: 676.85
    Smax: 16
    Vmax: 0.124
    alpha0: 4.56e-05
    kappa0: 593
```
