# ReactionConstantVolume

Constant volume of reaction.

Adds dV dP to dG and dH, and the same term to log K.

| | |
|---|---|
| Works on | reaction |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `dr_volume_constant` |
| Other names | `DrVolumeConst` |

## Example

From M075SH in `cemdata18-thermofun.json`.

```yaml
- method: ReactionConstantVolume
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("cemdata18-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "M075SH")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
298.15  100000  -28.634  1.6344e+05  54033  -366.97  -595.05
373.15  1e+07  -27.331  1.9525e+05  8495.7  -500.49  -595.05
```
