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
| a1 | 1 | 1 | no |  | coefficient of the equation of state |
| a2 | 1 | 1 | no |  | coefficient of the equation of state |
| a3 | 1 | 1 | no |  | coefficient of the equation of state |
| a4 | 1 | 1 | no |  | coefficient of the equation of state |
| a5 | 1 | 1 | no |  | coefficient of the equation of state |
| a6 | 1 | 1 | no |  | coefficient of the equation of state |
| a7 | 1 | 1 | no |  | coefficient of the equation of state |
| a8 | 1 | 1 | no |  | coefficient of the equation of state |
| a9 | 1 | 1 | no |  | coefficient of the equation of state |
| a10 | 1 | 1 | no |  | coefficient of the equation of state |
| a11 | 1 | 1 | no |  | coefficient of the equation of state |
| a12 | 1 | 1 | no |  | coefficient of the equation of state |
| a13 | 1 | 1 | no |  | coefficient of the equation of state |
| a14 | 1 | 1 | no |  | coefficient of the equation of state |
| gamma | 1 | 1 | no |  | coefficient of the exponential term |
| Tc | 1 | 1 | no |  | critical temperature, K |
| Vc | 1 | 1 | no |  | critical volume, cm3/mol |
| R | 1 | 1 | no |  | gas constant, cm3 bar/(K mol) |
| g1kb_0 | 1 | 1 | no |  | coefficient of G(T, 1 kbar) of DEW, cal/mol, the highest power of T (degC) first |
| g1kb_1 | 1 | 1 | no |  | coefficient of G(T, 1 kbar) of DEW |
| g1kb_2 | 1 | 1 | no |  | coefficient of G(T, 1 kbar) of DEW |
| g1kb_3 | 1 | 1 | no |  | coefficient of G(T, 1 kbar) of DEW |
| g1kb_4 | 1 | 1 | no |  | coefficient of G(T, 1 kbar) of DEW |
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Example

No bundled record uses this method.
