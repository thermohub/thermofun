# WaterIAPWS95

Water, IAPWS-95.

Calculates the properties of water from the IAPWS-95 formulation.

Source: Wagner and Pruss (2002).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | water_solvent, gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_eos_iapws95_gems`, `water_eos_iapws95_reaktoro` |
| Other names | `WaterWP95`, `WaterIAPWS95Gems` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: WaterIAPWS95
```
