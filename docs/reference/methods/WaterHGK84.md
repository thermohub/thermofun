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

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Implementation `gems`.

All the tables of the water code of GEMS, in the units of its formulas (see Methods/Water/WaterHGK-JNgems.cpp). name_N is place N of the array of the code. ac, nc, el, bc, ad: HGK equation of state; hc, hb, li: its limits and bounds; tt: triple point; cr, co: near-critical equation of Levelt Sengers et al. (1983); psat: saturation pressure; ideal: ideal gas of Woolley (1979); visc, cond: viscosity and thermal conductivity of Sengers and Kamgar-Parsi (1984); jn: dielectric constant of Johnson and Norton (1991). A record cannot replace these values: the GEMS code reads this table once for all its calculations.

Source: Haar, Gallagher and Kell (1984); Levelt Sengers et al. (1983); Johnson and Norton (1991).

Implementation `reaktoro`.

Helmholtz function of water, Reaktoro implementation. name_N is place N of the array of the code. A0 to A4: coefficients of its four parts; yc, ki, li, mi, ni, alpha, beta, ri, ti, A, z: their exponents and shape constants. referenceTemperature, referenceDensity, referenceHelmholtz: in SI.

Source: Haar L., Gallagher J.S., Kell G.S. (1984) NBS/NRC Steam Tables.

Names: `A0_1` to `A0_18`, `A1_1` to `A1_5`, `yc_1` to `yc_4`, `ki_1` to `ki_36`, `li_1` to `li_36`, `A3_1` to `A3_36`, `mi_1` to `mi_4`, `ni_1` to `ni_4`, `alpha_1` to `alpha_4`, `beta_1` to `beta_4`, `ri_1` to `ri_4`, `ti_1` to `ti_4`, `A4_1` to `A4_4`, `referenceTemperature`, `referenceDensity`, `referenceHelmholtz`, `A20`, `z0`.

## Example

From H2O in `heracles-thermofun.json`.

```yaml
- method: WaterHGK84
  parameters:
    state: 1
```
