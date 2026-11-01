# HelgesonKirkham74

Dielectric constant of Helgeson and Kirkham.

Calculates the dielectric constant of water and the Born functions from a polynomial in T and the density. HKF coefficients of Tanger and Helgeson (1988) and Sverjensky et al. (1997) were fitted with it.

Source: Helgeson and Kirkham (1974); Tanger and Helgeson (1988).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Dielectric constant of water, from which the HKF coefficients of Tanger and Helgeson (1988) were fitted: eps = sum over i, j of e_i_j T^i rho^j, with T in K and rho in g/cm3. The paper prints the first row of Table E-1 as e(0,1) to e(0,5): these are e_0_0 to e_0_4 of eq. E-1, which gives eps = 78.46 at 298.15 K. Tanger and Helgeson used the equations of Bradley and Pitzer (1979) below 100 degC and above 1 kbar; this method does not.

Source: Helgeson H.C., Kirkham D.H. (1974) Am. J. Sci. 274, 1089-1198; Tanger J.C., Helgeson H.C. (1988) Am. J. Sci. 288, 19-98, Appendix E.

Names: `e_0_0` to `e_0_4`, `e_1_0` to `e_1_3`, `e_2_0` to `e_2_2`, `e_3_0`, `e_3_1`, `e_4_0`.

## Example

No bundled record uses this method.
