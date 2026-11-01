# SverjenskyEtAl14

Dielectric constant of Sverjensky et al..

Calculates the dielectric constant of water at high pressure (DEW model). The published coefficients are for t in degC and the density in g/cm3.

Source: Sverjensky D.A., Harrison B., Azzolini D. (2014) Geochim. Cosmochim. Acta 129, 125-145.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_sverj14` |
| Other names | `Sverjensky14Diel` |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Dielectric constant of water at high pressure (Deep Earth Water model): eps = exp(u) rho^w, u = b1 t + b2 sqrt(t) + b3, w = a1 t + a2 sqrt(t) + a3, with t in degC and rho in g/cm3. The coefficients are those of the paper.

Source: Sverjensky D.A., Harrison B., Azzolini D. (2014) Geochim. Cosmochim. Acta 129, 125-145.

Names: `a1` to `a3`, `b1` to `b3`.

## Example

No bundled record uses this method.
