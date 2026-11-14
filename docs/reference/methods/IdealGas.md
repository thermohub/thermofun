# IdealGas

Ideal gas.

Sets V = RT/P, adds RT ln(P/P0) to G and subtracts R ln(P/P0) from S.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid, component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_pvnrt` |
| Other names | `IdealGasVol` |

## Example

From CH4 in `cemdata18-thermofun.json`.

```yaml
- method: IdealGas
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("cemdata18-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "CH4")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -50659  -74810  186.26  35.745  0.02479
373.15  1e+07  -50666  -71962  156.48  40.118  0.00031026
```
