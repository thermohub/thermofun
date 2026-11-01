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
| A0_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_5 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_6 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_7 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_8 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_9 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_10 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_11 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_12 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_13 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_14 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_15 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_16 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_17 | 1 | 1 | no |  | see Methods/ParameterSets |
| A0_18 | 1 | 1 | no |  | see Methods/ParameterSets |
| A1_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| A1_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| A1_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| A1_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| A1_5 | 1 | 1 | no |  | see Methods/ParameterSets |
| yc_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| yc_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| yc_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| yc_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_5 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_6 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_7 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_8 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_9 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_10 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_11 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_12 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_13 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_14 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_15 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_16 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_17 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_18 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_19 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_20 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_21 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_22 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_23 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_24 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_25 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_26 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_27 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_28 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_29 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_30 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_31 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_32 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_33 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_34 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_35 | 1 | 1 | no |  | see Methods/ParameterSets |
| ki_36 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_5 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_6 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_7 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_8 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_9 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_10 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_11 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_12 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_13 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_14 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_15 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_16 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_17 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_18 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_19 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_20 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_21 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_22 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_23 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_24 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_25 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_26 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_27 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_28 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_29 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_30 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_31 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_32 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_33 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_34 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_35 | 1 | 1 | no |  | see Methods/ParameterSets |
| li_36 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_5 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_6 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_7 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_8 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_9 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_10 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_11 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_12 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_13 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_14 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_15 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_16 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_17 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_18 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_19 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_20 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_21 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_22 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_23 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_24 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_25 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_26 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_27 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_28 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_29 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_30 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_31 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_32 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_33 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_34 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_35 | 1 | 1 | no |  | see Methods/ParameterSets |
| A3_36 | 1 | 1 | no |  | see Methods/ParameterSets |
| mi_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| mi_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| mi_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| mi_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| ni_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| ni_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| ni_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| ni_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| alpha_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| alpha_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| alpha_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| alpha_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| beta_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| beta_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| beta_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| beta_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| ri_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| ri_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| ri_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| ri_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| ti_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| ti_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| ti_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| ti_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| A4_1 | 1 | 1 | no |  | see Methods/ParameterSets |
| A4_2 | 1 | 1 | no |  | see Methods/ParameterSets |
| A4_3 | 1 | 1 | no |  | see Methods/ParameterSets |
| A4_4 | 1 | 1 | no |  | see Methods/ParameterSets |
| referenceTemperature | 1 | 1 | no |  | K |
| referenceDensity | 1 | 1 | no |  | kg/m3 |
| referenceHelmholtz | 1 | 1 | no |  | J/kg |
| A20 | 1 | 1 | no |  | coefficient |
| z0 | 1 | 1 | no |  | coefficient |
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Example

From H2O in `heracles-thermofun.json`.

```yaml
- method: WaterHGK84
  parameters:
    state: 1
```
