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
| g1kb_0 | 1 | 1 | no | 2.68807e-09 | coefficient of G(T, 1 kbar) of DEW, cal/mol, T in degC, the highest power first |
| g1kb_1 | 1 | 1 | no | 6.31631e-07 | coefficient of G(T, 1 kbar) of DEW, cal/mol, T in degC, the highest power first |
| g1kb_2 | 1 | 1 | no | -0.0193724 | coefficient of G(T, 1 kbar) of DEW, cal/mol, T in degC, the highest power first |
| g1kb_3 | 1 | 1 | no | -16.9451 | coefficient of G(T, 1 kbar) of DEW, cal/mol, T in degC, the highest power first |
| g1kb_4 | 1 | 1 | no | -55769.3 | coefficient of G(T, 1 kbar) of DEW, cal/mol, T in degC, the highest power first |
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Example

No bundled record uses this method.
