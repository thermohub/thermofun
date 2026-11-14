# ConstantCp

Constant heat capacity.

Integrates the constant reference Cp from the reference S, H and G.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `standard_entropy_cp_integration` |
| Other names | `EntropyCpIntegration` |

## Example

From HCN@ in `cemdata18-thermofun.json`.

```yaml
- method: ConstantCp
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("cemdata18-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "HCN@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  1.1437e+05  1.0375e+05  131.3  0  0
373.15  1e+07  1.0452e+05  1.0375e+05  131.3  0  0
```
