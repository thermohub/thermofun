# FernandezEtAl97

Dielectric constant of Fernandez et al..

Calculates the dielectric constant of water from the IAPWS formulation.

Source: Fernandez D.P., Goodwin A.R.H., Lemmon E.W., Levelt Sengers J.M.H., Williams R.C. (1997) J. Phys. Chem. Ref. Data 26, 1125-1166.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_fern97` |
| Other names | `Fernandez97Diel` |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Dielectric constant of water, IAPWS formulation: g = 1 + n12 r w^-1.2 + sum n_i r^I_i (Tc/T)^J_i, r = rho/rhoc, w = T/228 - 1. n: coefficients; I, J: exponents; Tc in K, rhoc in kg/m3. k, Na, alpha, epsilon, mu, M: the physical constants of the molecule and of vacuum, in SI, as in the paper.

Source: Fernandez D.P., Goodwin A.R.H., Lemmon E.W., Levelt Sengers J.M.H., Williams R.C. (1997) J. Phys. Chem. Ref. Data 26, 1125-1166.

Names: `n1` to `n12`, `I1` to `I11`, `J1` to `J11`, `Tc`, `rhoc`, `k`, `Na`, `alpha`, `epsilon0`, `mu`, `M`.

## Example

No bundled record uses this method.
