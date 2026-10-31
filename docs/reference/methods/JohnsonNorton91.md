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

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: JohnsonNorton91
```
