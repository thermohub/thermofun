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
