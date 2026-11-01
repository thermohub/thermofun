# SoaveRedlichKwong72

Soave-Redlich-Kwong equation of state.

Adds the pressure terms of the SRK equation of state to G, H and S, and sets V.

Source: Soave (1972); Soave (1993).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_soave_redlich_kwong` |
| Other names | `SRK` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| N | 1 | 1 | no |  | empirical parameter of the attraction term |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Soave-Redlich-Kwong equation of state. a = Oa R2 Tc2/Pc alpha, b = Ob R Tc/Pc, alpha = (1 + m (1 - sqrt(T/Tc)))2, m = m0 + m1 w - m2 w2 with the omega w. All are numbers without units.

Source: Soave G. (1972) Chem. Eng. Sci. 27, 1197-1203.

Names: `m0` to `m2`, `Oa`, `Ob`.

## Example

No bundled record uses this method.
