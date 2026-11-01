# WaterZhangDuan05

Water, Zhang and Duan.

Calculates the properties of water at high pressure from the equation of Zhang and Duan.

Source: Zhang and Duan (2005).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_pvt_zhang_duan05` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| dew_gibbs | 1 | 1 | no | 0 | 1: the Gibbs energy of the model DEW (P >= 1 kbar) |
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Water at high pressure and temperature. a1 to a14, gamma: coefficients of the equation of state; Tc in K, Vc in cm3/mol, R in cm3 bar/(K mol), as in the formulas. g1kb_0 to g1kb_4: G(T, 1 kbar) of the DEW model, a polynomial in T (degC) in cal/mol, highest power first. Tmax in K, Pmin and Pmax in Pa: the range of the model; outside of it the calculation stops with an error.

Source: Zhang Z., Duan Z. (2005) Phys. Earth Planet. Inter. 149, 335-354; Sverjensky et al. (2014).

Names: `a1` to `a14`, `gamma`, `Tc`, `Vc`, `R`, `g1kb_0` to `g1kb_4`, `Tmax`, `Pmax`, `Pmin`.

## Example

No bundled record uses this method.
