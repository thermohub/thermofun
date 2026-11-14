# LandauHP11

Landau transition of Holland and Powell (2011).

Adds the Landau term of a phase transition to G, H, S, Cp and V, with the order parameter at the reference state.

Source: Holland and Powell (2011).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Other names | `HP11Landau` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc0 | Cel | K | yes |  | critical temperature at the reference pressure |
| Smax | J/(mol.K) | J/(mol.K) | yes |  | maximum entropy of the transition |
| Vmax | J/(mol.bar) | m3/mol | yes |  | maximum volume of the transition |

## Example

From Holland and Powell (2011), data set ds62, quartz; the other values of the record are Quartz of aq17. No bundled database has this method yet.

```yaml
substances:
  Quartz:
    formula: SiO2
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -856433.0, H: -910830.0, S: 41.5, Cp: 44.89092, V: {value: 2.2688, unit: J/bar}}
    methods:
      - method: ConstantCp
      - method: LandauHP11
        parameters: {Tc0: {value: 847.0, unit: K}, Smax: 4.95, Vmax: {value: 0.1188, unit: J/(mol.bar)}}
```

`pytests/test_method_examples.py` checks that it calculates, at 773.15 K and 1000 bar. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(773.15, 1e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Quartz")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
773.15  1e+08  -8.8834e+05  -8.8818e+05  86.578  51.539  5.5258e-07
```
