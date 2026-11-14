# SoluteHP98

Aqueous solute of Holland and Powell.

Calculates the properties of an aqueous species from the density model of Holland and Powell. Implementation consistent (the default): S, H and Cp are the exact derivatives of G. Implementation published: the formulas of the paper, in which T' is a constant and S, Cp, H are not the derivatives of G. G and V are the same in both.

Source: Holland and Powell (1998).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_holland_powell98` |
| Other names | `HP98Sol` |

## Implementations

`consistent`, `published`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| b | kJ/(mol.K2) | J/(mol.K2) | yes |  | coefficient b |

## Example

From Holland and Powell (1998), J. Metamorphic Geol. 16, 309-343, the density model of the aqueous species; b is an illustrative value. No bundled database has this method yet.

```yaml
substances:
  Na+:
    formula: Na+
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -261881.0, H: -240300.0, S: 58.45, Cp: 46.4, V: -1.2e-6}
    methods:
      - method: SoluteHP98
        parameters: {b: 0.1}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Na+ | 298.15 | 1 bar | G | -261881 | 1 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (473.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Na+")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -2.6188e+05  -2.403e+05  58.45  46.4  -1.2e-06
473.15  1e+07  -2.7365e+05  -2.3372e+05  75.616  60.178  4.9181e-06
```
