# VolumeDorogokupets88

Molar volume of Dorogokupets et al..

Calculates V(T, P) as VolumeBerman88, with another form of the coefficients.

Source: Dorogokupets et al. (1988).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_equation_dorogokupets88` |
| Other names | `Dorogokupets88Vol` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1/K | 1/K | yes |  | thermal expansion, first coefficient |
| a2 | 1/K2 | 1/K2 | yes |  | thermal expansion, second coefficient |
| a3 | 1/K3 | 1/K3 | yes |  | thermal expansion, third coefficient |
| b1 | 1/bar | 1/Pa | yes |  | compressibility, first coefficient |
| b2 | 1/bar2 | 1/Pa2 | yes |  | compressibility, second coefficient |

## Example

From Dorogokupets, Sokolova, Danilov and Litasov (1988), as given in GEMS; the coefficients are illustrative values of the order of a silicate. No bundled database has this method yet.

```yaml
substances:
  Q:
    formula: SiO2
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -856288.0, H: -910700.0, S: 41.46, Cp: 44.6, V: 2.2688e-5}
    methods:
      - method: ConstantCp
      - method: VolumeDorogokupets88
        parameters: {a1: 2.4e-5, a2: 1.0e-8, a3: 0.0, b1: -2.0e-11, b2: 1.0e-22}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Q | 298.15 | 1 bar | V | 2.2688e-05 | 1e-12 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (600, 1e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Q")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -8.5629e+05  -9.107e+05  41.46  44.6  2.2688e-05
600  1e+08  -8.7177e+05  -8.95e+05  72.582  44.6  2.2828e-05
```
