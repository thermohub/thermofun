# ConstantVolume

Constant molar volume.

Adds V dP to G and H, with the reference volume.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_constant` |
| Other names | `ConMolVol` |

## Example

From Portlandite in `aq17-thermofun.json`.

```yaml
- method: ConstantVolume
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Portlandite")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -8.9701e+05  -9.8468e+05  83.4  87.505  3.306e-05
373.15  1e+07  -9.0373e+05  -9.7746e+05  103.98  95.696  3.306e-05
```
