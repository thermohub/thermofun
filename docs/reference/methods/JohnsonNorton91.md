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

Units of the values: T: K, rho: kg/m3. They are not converted.

Dielectric constant of water: eps = sum k_i(t) r^i, t = T/Tr, r = rho/Dr, with k0 = 1, k1 = a1/t, k2 = a2/t + a3 + a4 t, k3 = a5/t + a6 t + a7 t2, k4 = a8/t2 + a9/t + a10. Tr in K, Dr in kg/m3. Both implementations use these values.

Source: Johnson J.W., Norton D. (1991) Am. J. Sci. 291, 541-648.

Names: `a1` to `a10`, `Tr`, `Dr`.

## Example

From H2O@ in `aq17-thermofun.json`.

```yaml
- method: JohnsonNorton91
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -2.3718e+05  -2.8583e+05  69.927  75.328  1.8069e-05
373.15  1e+07  -2.429e+05  -2.8004e+05  86.724  75.547  1.8709e-05
```
