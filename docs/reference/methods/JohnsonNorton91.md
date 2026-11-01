# JohnsonNorton91

Dielectric constant of Johnson and Norton.

Calculates the dielectric constant of water and the Born functions.

Source: Johnson and Norton (1991).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_jnort91_reaktoro`, `water_diel_jnort91_gems` |
| Other names | `JN91Reaktoro`, `JN91Gems` |

## Implementations

`reaktoro`, `gems`. The default is `reaktoro`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1 | 1 | no |  | coefficient of k_i(t) |
| a2 | 1 | 1 | no |  | coefficient of k_i(t) |
| a3 | 1 | 1 | no |  | coefficient of k_i(t) |
| a4 | 1 | 1 | no |  | coefficient of k_i(t) |
| a5 | 1 | 1 | no |  | coefficient of k_i(t) |
| a6 | 1 | 1 | no |  | coefficient of k_i(t) |
| a7 | 1 | 1 | no |  | coefficient of k_i(t) |
| a8 | 1 | 1 | no |  | coefficient of k_i(t) |
| a9 | 1 | 1 | no |  | coefficient of k_i(t) |
| a10 | 1 | 1 | no |  | coefficient of k_i(t) |
| Tr | 1 | 1 | no |  | reference temperature, K |
| Dr | 1 | 1 | no |  | reference density, kg/m3 |

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: JohnsonNorton91
```
