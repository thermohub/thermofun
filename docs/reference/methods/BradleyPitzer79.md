# BradleyPitzer79

Dielectric constant of Bradley and Pitzer.

Calculates the dielectric constant of water and the Born functions from T and P, with no density of water.

Source: Bradley and Pitzer (1979).

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

Dielectric constant of water as a function of T and P, with no density: D = D1000 + C ln((B + P)/(B + 1000)), D1000 = U1 exp(U2 T + U3 T2), C = U4 + U5/(U6 + T), B = U7 + U8/T + U9 T, with T in K and P in bar. Tmin, Tmax in K, Pmax in bar: its range, up to Pmax_low below Tlow. Outside of it the method warns. Tanger and Helgeson (1988) used it below about 70 degC and above 1 kbar.

Source: Bradley D.J., Pitzer K.S. (1979) J. Phys. Chem. 83, 1599-1603, Table I.

Names: `U1` to `U9`, `Tmin`, `Tmax`, `Tlow`, `Pmax_low`, `Pmax`.

## Example

No bundled record uses this method.
