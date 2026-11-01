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

IAPWS-95 formulation of water, Reaktoro implementation. name_N is place N of the array of the code. no, gammao: ideal gas part; c, d, t, n: residual part; alpha, beta, gamma, epsilon, a, b, A, B, C, F, E: its Gaussian and non-analytic terms. sat_a1 to sat_a6: saturation pressure of Wagner and Pruss (1993), used by all the water methods that take P = 0 as saturation, so a record cannot replace them.

Source: Wagner W., Pruss A. (2002) J. Phys. Chem. Ref. Data 31, 387-535.

Names: `no_1` to `no_9`, `gammao_1` to `gammao_5`, `c_1` to `c_55`, `d_1` to `d_55`, `t_1` to `t_55`, `n_1` to `n_57`, `alpha_1` to `alpha_3`, `beta_1` to `beta_3`, `gamma_1` to `gamma_3`, `epsilon_1` to `epsilon_3`, `a_1`, `a_2`, `b_1`, `b_2`, `A_1`, `A_2`, `B_1`, `B_2`, `C_1`, `C_2`, `F_1`, `F_2`, `E_1`, `E_2`, `sat_a1` to `sat_a6`.

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: WaterIAPWS95
```
