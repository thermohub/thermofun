# BirchMurnaghan47

Birch-Murnaghan equation of state.

Calculates V(T, P) from the third-order Birch-Murnaghan equation, with alpha(T) = a1 + a2 T + a3/T2 and K(T) linear in T.

Source: Birch (1947); code of M. Gottschalk.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_eos_birch_murnaghan_gott97` |
| Other names | `BirchMurnaghanGottschalk97`, `BirchMurnaghanGott97` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1/K | 1/K | yes |  | thermal expansion, constant term |
| a2 | 1/K2 | 1/K2 | no |  | thermal expansion, term a2 T |
| a3 | K | K | no |  | thermal expansion, term a3 / T2 |
| kt00 | MPa | Pa | yes |  | bulk modulus at the reference temperature |
| dkdt | MPa/K | Pa/K | no |  | temperature derivative of the bulk modulus |
| kp | 1 | 1 | yes |  | pressure derivative of the bulk modulus |
| kpp | 1/MPa | 1/Pa | no |  | second pressure derivative of the bulk modulus; 0: computed from kp |

## Example

From Birch (1947), Phys. Rev. 71, 809-824; the coefficients are illustrative values of the order of corundum (K = 252 GPa, K' = 4.3). No bundled database has this method yet.

```yaml
substances:
  Co:
    formula: Al2O3
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -1582300.0, H: -1675700.0, S: 50.92, Cp: 79.0, V: 2.558e-5}
    methods:
      - method: ConstantCp
      - method: BirchMurnaghan47
        parameters: {a1: 1.9e-5, kt00: 2.52e+11, kp: 4.3}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Co | 298.15 | 1 bar | V | 2.558e-05 | 1e-12 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (600, 1e+09)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Co")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -1.5823e+06  -1.6757e+06  50.92  79  2.558e-05
600  1e+09  -1.5813e+06  -1.6259e+06  106.66  79  2.5626e-05
```
