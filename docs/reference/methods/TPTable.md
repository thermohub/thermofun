# TPTable

Table of T and P.

Interpolates a table of G, H, V and Cp on a grid of T and P.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `standard_thermo_interpolation` |
| Other names | `TPTableInterpolation`, `StandardThermoModelInterpolation` |

## Example

From the table of the how-to "Add data" (section 5.6), values chosen for the example. No bundled database has this method yet.

```yaml
substances:
  T:
    formula: Ca
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -1000, H: -900, S: 10, Cp: 20, V: 1.0e-5}
    methods:
      - method: TPTable
        table:
          temperatures: [298.15, 400, 500, 600]
          pressures: [1.0e5]
          G: [-1000, -1100, -1200, -1300]
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| T | 400 | 1 bar | G | -1100 | 1e-06 |
| T | 500 | 1 bar | G | -1200 | 1e-06 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(400, 100000), (500, 100000)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "T")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
400  100000  -1100  0  0  0  1e-05
500  100000  -1200  0  0  0  1e-05
```
