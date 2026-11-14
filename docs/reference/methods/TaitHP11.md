# TaitHP11

Tait equation of Holland and Powell.

Calculates V(T, P) from the modified Tait equation with a thermal pressure, and adds the terms to G, H, S and Cp.

Source: Holland and Powell (2011).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `mv_eos_tait_hp11` |
| Other names | `HP11Tait` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| kappa0p | 1 | 1 | yes |  | pressure derivative of the bulk modulus |
| kappa0pp | 1/kbar | 1/Pa | yes |  | second pressure derivative of the bulk modulus |
| n_atoms | 1 | 1 | yes |  | number of atoms in the formula |
| alpha0 | 1/K | 1/K | yes |  | thermal expansion at the reference state |
| kappa0 | kbar | Pa | yes |  | bulk modulus at 298 K |

## Example

From Holland and Powell (2011), J. Metamorphic Geol. 29, 333-383, the form of the equation; the coefficients are illustrative values of the order of periclase. No bundled database has this method yet.

```yaml
substances:
  Per:
    formula: MgO
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -569300.0, H: -601600.0, S: 26.94, Cp: 37.8, V: 1.125e-5}
    methods:
      - method: ConstantCp
      - method: TaitHP11
        parameters: {kappa0p: 4.0, kappa0pp: -2.4e-11, n_atoms: 2, alpha0: 3.1e-5, kappa0: 1.6e+11}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Per | 298.15 | 1 bar | V | 1.125e-05 | 1e-09 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (600, 1e+09)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Per")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -5.693e+05  -6.016e+05  26.94  37.8  1.125e-05
600  1e+09  -5.7055e+05  -5.7912e+05  52.934  37.691  1.13e-05
```
