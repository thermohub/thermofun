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

All the tables of the water code of GEMS, in the units of its formulas (see Methods/Water/WaterHGK-JNgems.cpp). name_N is place N of the array of the code. ac, nc, el, bc, ad: HGK equation of state; hc, hb, li: its limits and bounds; tt: triple point; cr, co: near-critical equation of Levelt Sengers et al. (1983); psat: saturation pressure; ideal: ideal gas of Woolley (1979); visc, cond: viscosity and thermal conductivity of Sengers and Kamgar-Parsi (1984). The dielectric constant of this code takes its values from JohnsonNorton91.yaml.

Source: Haar, Gallagher and Kell (1984); Levelt Sengers et al. (1983); Johnson and Norton (1991).

Implementation `reaktoro`.

Helmholtz function of water, Reaktoro implementation. name_N is place N of the array of the code. A0 to A4: coefficients of its four parts; yc, ki, li, mi, ni, alpha, beta, ri, ti, A, z: their exponents and shape constants. referenceTemperature, referenceDensity, referenceHelmholtz: in SI. Ttr: the temperature in K that the model takes as the triple point. Saturation and critical values: see WaterSaturation.yaml.

Source: Haar L., Gallagher J.S., Kell G.S. (1984) NBS/NRC Steam Tables.

Values that the Reaktoro implementations of HGK and IAPWS-95 share. Saturation of water: sat_a1 to sat_a6 for the pressure, sat_b1 to sat_b6 for the density of the liquid, sat_c1 to sat_c6 for the density of the vapour; numbers without units. Tcr in K, Pcr in Pa, Dcr in kg/m3: the critical point. M in kg/mol: the molar mass of water. A pressure of 0 means the saturation pressure at T. The code of GEMS has its own saturation equation (psat_a in WaterHGK84-gems.yaml).

Source: Wagner W., Pruss A. (1993) J. Phys. Chem. Ref. Data 22, 783-787; Wagner W., Pruss A. (2002) J. Phys. Chem. Ref. Data 31, 387-535.

Names: `A0_1` to `A0_18`, `A1_1` to `A1_5`, `yc_1` to `yc_4`, `ki_1` to `ki_36`, `li_1` to `li_36`, `A3_1` to `A3_36`, `mi_1` to `mi_4`, `ni_1` to `ni_4`, `alpha_1` to `alpha_4`, `beta_1` to `beta_4`, `ri_1` to `ri_4`, `ti_1` to `ti_4`, `A4_1` to `A4_4`, `sat_a1` to `sat_a6`, `sat_b1` to `sat_b6`, `sat_c1` to `sat_c6`, `Tcr`, `Pcr`, `Dcr`, `M`, `Ttr`, `referenceTemperature`, `referenceDensity`, `referenceHelmholtz`, `A20`, `z0`, `ac_wm`, `ac_gascon`, `ac_tz`, `ac_aa`, `ac_uref`, `ac_sref`, `ac_zb`, `ac_dzb`, `ac_yb`, `nc_nc`, `el_g1`, `el_g2`, `el_gf`, `el_b1`, `el_b2`, `el_b1t`, `el_b2t`, `el_b1tt`, `el_b2tt`, `hc_tcHGK`, `hc_dcHGK`, `hc_pcHGK`, `hb_Ttop`, `hb_Tbtm`, `hb_Ptop`, `hb_Pbtm`, `hb_Dtop`, `hb_Dbtm`, `li_sDli1`, `li_sPli1`, `li_sDli37`, `li_sPli37`, `li_sDIB30`, `li_Tli13`, `li_Pli13`, `li_Dli13`, `li_TnIB30`, `li_DnIB30`, `tt_Utri`, `tt_Stri`, `tt_Htri`, `tt_Atri`, `tt_Gtri`, `tt_Ttr`, `tt_Ptripl`, `tt_Dltrip`, `tt_Dvtrip`, `cr_Tc`, `cr_rhoC`, `cr_Pc`, `cr_Pcon`, `cr_Ucon`, `cr_Scon`, `cr_dPcon`, `nc_g_1` to `nc_g_40`, `nc_ii_1` to `nc_ii_40`, `nc_jj_1` to `nc_jj_40`, `bc_bp_1` to `bc_bp_10`, `bc_bq_1` to `bc_bq_10`, `ad_atz_1` to `ad_atz_4`, `ad_adz_1` to `ad_adz_4`, `ad_aat_1` to `ad_aat_4`, `ad_aad_1` to `ad_aad_4`, `co_a_1` to `co_a_20`, `co_q_1` to `co_q_20`, `co_x_1` to `co_x_11`, `psat_a_1` to `psat_a_8`, `ideal_c_1` to `ideal_c_18`, `visc_a_1` to `visc_a_4`, `visc_b_1` to `visc_b_42`, `cond_aL_1` to `cond_aL_4`, `cond_au_1` to `cond_au_4`, `cond_bL_1` to `cond_bL_30`, `cond_bu_1` to `cond_bu_30`.

## Example

From H2O in `heracles-thermofun.json`.

```yaml
- method: WaterHGK84
  parameters:
    state: 1
```
