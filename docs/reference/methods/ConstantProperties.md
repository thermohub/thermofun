# ConstantProperties

Constant properties.

Gives the reference properties at every T and P.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Example

From Quartz of aq17. No bundled database has this method yet.

The reference values at every T and P. For a substance whose data are known only at 25 degC, near 25 degC.

```yaml
substances:
  Quartz:
    formula: SiO2
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -856433.0, H: -910830.0, S: 41.5, Cp: 44.89092, V: {value: 2.2688, unit: J/bar}}
    methods:
      - method: ConstantProperties
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Quartz | 373.15 | 100 bar | G | -856433 | 1e-06 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Quartz")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
373.15  1e+07  -8.5643e+05  -9.1083e+05  41.5  44.891  2.2688e-05
```
