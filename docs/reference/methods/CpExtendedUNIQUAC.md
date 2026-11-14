# CpExtendedUNIQUAC

Heat capacity of Extended UNIQUAC.

Integrates Cp = a + b T + c / (T - Theta) and adds a pressure term.

Source: Thomsen and Rasmussen (1999).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Convention of its results | gibbs_helmholtz_formation |
| Legacy names | `cp_ft_equation_uniquac` |
| Other names | `ExtendedUNIQUAC`, `StandardThermoModelExtendedUNIQUAC` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | J/(mol.K) | J/(mol.K) | yes |  | constant term |
| b | J/(mol.K2) | J/(mol.K2) | yes |  | term b T |
| c | J/mol | J/mol | yes |  | term c / (T - Theta) |
| alpha | 1/bar | 1/Pa | yes |  | pressure term, first coefficient |
| beta | 1/bar2 | 1/Pa2 | yes |  | pressure term, second coefficient |
| Theta | K | K | no | 200 | temperature of the pole of the heat capacity |

## Example

From Thomsen and Rasmussen (1999), Chem. Eng. Sci. 54, 1787-1802, the form Cp = a + b T + c / (T - Theta); the coefficients are illustrative values. No bundled database has this method yet.

```yaml
substances:
  Na+:
    formula: Na+
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -261881.0, H: -240300.0, S: 58.45, Cp: 46.4, V: -1.2e-6}
    methods:
      - method: CpExtendedUNIQUAC
        parameters: {a: 40.0, b: 0.1, c: -2000.0, alpha: 0.0, beta: 0.0}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Na+ | 298.15 | 1 bar | Cp | 49.438 | 0.001 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Na+")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -2.6188e+05  -2.403e+05  58.343  49.438  -1.2e-06
373.15  1e+07  -2.6674e+05  -2.3592e+05  71.386  65.764  -1.2e-06
```
