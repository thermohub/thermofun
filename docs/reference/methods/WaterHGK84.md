# WaterHGK84

Water, Haar-Gallagher-Kell.

Calculates the properties of water from the HGK equation of state.

Source: Haar, Gallagher and Kell (1984); Levelt Sengers et al. (1983).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | water_solvent, gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_eos_hgk84_lvs83_gems`, `water_eos_hgk84_reaktoro` |
| Other names | `WaterHGK`, `WaterHGKGems` |

## Implementations

`reaktoro`, `gems`. The default is `reaktoro`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Example

From H2O in `heracles-thermofun.json`.

```yaml
- method: WaterHGK84
  parameters:
    state: 1
```
