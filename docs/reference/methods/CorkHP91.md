# CorkHP91

Compensated Redlich-Kwong equation of state.

Adds the pressure terms of the CORK equation of state to G, H, S and Cp, and sets V.

Source: Holland and Powell (1991).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_comp_redlich_kwong_hp91` |
| Other names | `CORK` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| k1 | 1 | 1 | no |  | interaction parameter k1 |
| k2 | 1 | 1 | no |  | interaction parameter k2 |
| k3 | 1 | 1 | no |  | interaction parameter k3 |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, P: kbar, V: kJ/kbar. They are not converted.

Compensated Redlich-Kwong equation of state, the constants of its three forms, in kJ, kbar and K as in the formulas. cs: any fluid, by corresponding states from Tc and Pc: a = a0 Tc^2.5/Pc + a1 Tc^1.5 T/Pc, b = b0 Tc/Pc, c = c0 Tc/Pc^1.5 + c1 T/Pc^1.5, d = d0 Tc/Pc2 + d1 T/Pc2. co2: a = a0 + a1 T + a2 T2, c = c0 + c1 T, d = d0 + d1 T above p0 in kbar. h2o: a is a polynomial in (T0 - T), with a1 to a3 for the liquid and a7 to a9 for the vapour below T0, and in (T - T0) with a4 to a6 above T0 (Eq. 6 of the paper; the implementation gems has (T0 - T) between T0 and Tsuper); co2: a and the virial terms are those of Perple_X (a from R. Powell) and of Holland and Powell (1998), not of Table 1 of 1991; Tsuper: the T above which H2O is one fluid; psat0, psat2, psat3, psat5: Psat in kbar, a polynomial in T of powers 0, 2, 3, 5.

Source: Holland T.J.B., Powell R. (1991) Contrib. Mineral. Petrol. 109, 265-273.

Names: `cs_a0`, `cs_a1`, `cs_b0`, `cs_c0`, `cs_c1`, `cs_d0`, `cs_d1`, `co2_a0` to `co2_a2`, `co2_b`, `co2_c0`, `co2_c1`, `co2_d0`, `co2_d1`, `co2_e0`, `co2_p0`, `h2o_a0` to `h2o_a9`, `h2o_b`, `h2o_c0`, `h2o_c1`, `h2o_d0`, `h2o_d1`, `h2o_e0`, `h2o_p0`, `h2o_psat0`, `h2o_psat2`, `h2o_psat3`, `h2o_psat5`, `h2o_T0`, `h2o_Tsuper`.

## Example

From CH4 in `aq17-thermofun.json`.

```yaml
- method: CorkHP91
  units: {pressure: bar}
  parameters:
    Tc: 190.6
    Pc: 46
    omega: 0
```
