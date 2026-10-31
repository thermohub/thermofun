# HKF88

Revised HKF equation of state.

Calculates the properties of an aqueous species from the revised Helgeson-Kirkham-Flowers equation of state.

Source: Tanger and Helgeson (1988); Shock et al. (1992).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_hkf88_gems`, `solute_hkf88_reaktoro` |
| Other names | `HKFReaktoro`, `HKFGems` |

## Implementations

`reaktoro`, `gems`. The default is `reaktoro`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | cal/(mol.bar) | J/(mol.Pa) | yes |  | pressure coefficient a1 |
| a2 | cal/mol | J/mol | yes |  | pressure coefficient a2 |
| a3 | cal.K/(mol.bar) | J.K/(mol.Pa) | yes |  | pressure coefficient a3 |
| a4 | cal.K/mol | J.K/mol | yes |  | pressure coefficient a4 |
| c1 | cal/(mol.K) | J/(mol.K) | yes |  | temperature coefficient c1 |
| c2 | cal.K/mol | J.K/mol | yes |  | temperature coefficient c2 |
| omega | cal/mol | J/mol | yes |  | conventional Born coefficient |

## Example

From Al(OH)2+ in `aq17-thermofun.json`.

```yaml
- method: HKF88
  units: {energy: cal, pressure: bar}
  parameters:
    a1: 0.2494
    a2: -169.09
    a3: 6.4146
    a4: -27091
    c1: 16.7439
    c2: -10465
    omega: 53240
```
