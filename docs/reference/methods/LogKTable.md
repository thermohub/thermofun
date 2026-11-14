# LogKTable

Table of log K.

Interpolates a table of log K(T), with Lagrange polynomials or a cubic spline.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_lagrange_interp` |
| Other names | `LogKLagrange` |

## Example

From values of the equation of Plummer and Busenberg (1982) for calcite at 0, 25, 50, 75 and 100 degC. No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48}
    methods:
      - method: LogKTable
        interpolation: lagrange
        order: 3
        table:
          temperatures: [273.15, 298.15, 323.15, 348.15, 373.15]
          logK: [-8.3814, -8.4798, -8.6628, -8.9266, -9.2665]
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 310.15 | 1 bar | logK | -8.5573 | 0.002 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(310.15, 100000)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Calcite-dissolution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
310.15  100000  -8.5573  50811  -13372  -206.94  -331.52
```
