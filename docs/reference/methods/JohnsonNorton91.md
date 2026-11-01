# JohnsonNorton91

Dielectric constant of Johnson and Norton.

Calculates the dielectric constant of water and the Born functions.

Source: Johnson and Norton (1991).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_jnort91_reaktoro`, `water_diel_jnort91_gems` |
| Other names | `JN91Reaktoro`, `JN91Gems` |

## Implementations

`reaktoro`, `gems`. The default is `reaktoro`. Use another only to reproduce the results of that code.

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Dielectric constant of water: eps = sum k_i(t) r^i, t = T/Tr, r = rho/Dr, with k0 = 1, k1 = a1/t, k2 = a2/t + a3 + a4 t, k3 = a5/t + a6 t + a7 t2, k4 = a8/t2 + a9/t + a10. Tr in K, Dr in kg/m3. Used by the Reaktoro implementation; the GEMS water code has its own copy (jn_a_* in WaterHGK84-gems.yaml).

Source: Johnson J.W., Norton D. (1991) Am. J. Sci. 291, 541-648.

Names: `a1` to `a10`, `Tr`, `Dr`.

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: JohnsonNorton91
```
