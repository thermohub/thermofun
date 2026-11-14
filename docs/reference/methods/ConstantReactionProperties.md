# ConstantReactionProperties

Constant reaction properties.

Gives the reference reaction properties at every T and P.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Example

From Plummer and Busenberg (1982), as written in phreeqc.dat. No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48, dH: -9610.6}
    methods:
      - method: ConstantReactionProperties
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 323.15 | 1 bar | logK | -8.48 | 1e-09 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(323.15, 100000)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Calcite-dissolution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
323.15  100000  -8.48  0  -9610.6  0  0
```
