# WaterIAPWS95Gas

Water vapour, gas equation of IAPWS-95.

Calculates the properties of metastable water vapour (above the saturation pressure, below 10 MPa, up to 55 kg/m3) from the gas equation of Wagner and Pruss (2002), Eq. 3.2. IAPWS-95 is not reliable there away from the saturated vapour line; the authors advise this equation, as IAPWS-IF97 does. Used in this region only, for a vapour record (state gas); elsewhere the next equation of state of the record, for example WaterIAPWS95, is used, and it gives the saturation.

Source: Wagner and Pruss (2002) J. Phys. Chem. Ref. Data 31, 387-535, Sects. 3, 6.3.1 and 7.3.2.3.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Contributes to | G sets, H sets, S sets, Cp sets, V sets |
| Classes | water_solvent, gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Model parameters

Parameters of the model, the same for every record. A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, rho: kg/m3. They are not converted.

Gas equation of Wagner and Pruss (2002), Eq. 3.2 and Table 3.1: the Helmholtz function of water for the vapour up to 55 kg/m3 (about 0.17 times the critical density), made from the ideal-gas part of IAPWS-95 and 7 residual terms n delta^d tau^t. The numbers n, d, t are place N of the arrays, as in WaterIAPWS95.yaml: place 1 is not used, places 2 to 8 are the terms 1 to 7 of Table 3.1. The ideal-gas part, the critical point and the saturation come from WaterIAPWS95.yaml and WaterSaturation.yaml.

Source: Wagner W., Pruss A. (2002) J. Phys. Chem. Ref. Data 31, 387-535, Sect. 3.

Names: `d_1` to `d_8`, `t_1` to `t_8`, `n_1` to `n_8`.

## Example

No bundled record uses this method.
