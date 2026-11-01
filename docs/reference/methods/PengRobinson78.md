# PengRobinson78

Peng-Robinson equation of state.

Adds the pressure terms of the Peng-Robinson equation of state (1978 form) to G, H and S, and sets V.

Source: Peng and Robinson (1976, 1978).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_peng_robinson78` |
| Other names | `PR78` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| N | 1 | 1 | no |  | empirical parameter of the attraction term |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: none: numbers. They are not converted.

Peng-Robinson equation of state, 1978 form. a = Oa R2 Tc2/Pc alpha, b = Ob R Tc/Pc, alpha = (1 + k (1 - sqrt(T/Tc)))2. k = k1_0 + k1_1 w - k1_2 w2 for omega w up to omega_switch, else k = k2_0 + k2_1 w - k2_2 w2 + k2_3 w3. All are numbers without units.

Source: Peng D.Y., Robinson D.B. (1976) Ind. Eng. Chem. Fundam. 15, 59-64; Robinson D.B., Peng D.Y. (1978) GPA Research Report 28.

Names: `k1_0` to `k1_2`, `k2_0` to `k2_3`, `omega_switch`, `Oa`, `Ob`.

## Example

No bundled record uses this method.
