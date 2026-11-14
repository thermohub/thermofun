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

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Implementation `reaktoro`.

Units of the values: T: K, rho: kg/m3. They are not converted.

IAPWS-95 formulation of water, Reaktoro implementation. name_N is place N of the array of the code. no, gammao: ideal gas part; c, d, t, n: residual part; alpha, beta, gamma, epsilon, a, b, A, B, C, F, E: its Gaussian and non-analytic terms. Ttr: the temperature in K that the model takes as the triple point. Saturation and critical values: see WaterSaturation.yaml.

Source: Wagner W., Pruss A. (2002) J. Phys. Chem. Ref. Data 31, 387-535.

Units of the values: T: K, P: Pa, rho: kg/m3, M: kg/mol. They are not converted.

Values that the Reaktoro implementations of HGK and IAPWS-95 share. Saturation of water: sat_a1 to sat_a6 for the pressure, sat_b1 to sat_b6 for the density of the liquid, sat_c1 to sat_c6 for the density of the vapour; numbers without units. Tcr in K, Pcr in Pa, Dcr in kg/m3: the critical point. M in kg/mol: the molar mass of water. A pressure of 0 means the saturation pressure at T. The code of GEMS has its own saturation equation (psat_a in WaterHGK84-gems.yaml).

Source: Wagner W., Pruss A. (1993) J. Phys. Chem. Ref. Data 22, 783-787; Wagner W., Pruss A. (2002) J. Phys. Chem. Ref. Data 31, 387-535.

Names: `sat_a1` to `sat_a6`, `sat_b1` to `sat_b6`, `sat_c1` to `sat_c6`, `Tcr`, `Pcr`, `Dcr`, `M`, `Ttr`, `no_1` to `no_9`, `gammao_1` to `gammao_5`, `c_1` to `c_55`, `d_1` to `d_55`, `t_1` to `t_55`, `n_1` to `n_57`, `alpha_1` to `alpha_3`, `beta_1` to `beta_3`, `gamma_1` to `gamma_3`, `epsilon_1` to `epsilon_3`, `a_1`, `a_2`, `b_1`, `b_2`, `A_1`, `A_2`, `B_1`, `B_2`, `C_1`, `C_2`, `F_1`, `F_2`, `E_1`, `E_2`.

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: WaterIAPWS95
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -2.3718e+05  -2.8583e+05  69.927  75.328  1.8069e-05
373.15  1e+07  -2.429e+05  -2.8004e+05  86.724  75.547  1.8709e-05
```
