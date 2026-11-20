# DolejsManning10

Density model of Dolejs and Manning.

Calculates dG = a + b T + c T ln T + d T2 + e T ln(density of water), and log K from it.

Source: Dolejs and Manning (2010).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_dolejs_manning10` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | kJ/mol | J/mol | yes |  | enthalpy term |
| b | J/(mol.K) | J/(mol.K) | yes |  | entropy term |
| c | J/(mol.K) | J/(mol.K) | no |  | heat capacity, constant term |
| d | J/(mol.K2) | J/(mol.K2) | no |  | heat capacity, linear term |
| e | J/(mol.K) | J/(mol.K) | yes |  | density term |

## Example

From the form of the method (Dolejs and Manning 2010, Geofluids 10, 20-40), dG = a + b T + c T ln T + d T2 + e T ln(density of water); the coefficients are illustrative values. No bundled database has this method yet.

```yaml
reactions:
  Quartz-solution:
    equation: Qz = SiO2@
    reaction_properties: {logK: -4.0}
    methods:
      - method: DolejsManning10
        parameters: {a: 25000.0, b: -50.0, e: -20.0}
```

`pytests/test_method_examples.py` checks that it calculates, at 573.15 K and 1000 bar, 673.15 K and 5000 bar. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(573.15, 1e+08), (673.15, 5e+08)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Quartz-solution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
573.15  1e+08  0.13004  -1426.9  15906  30.242  -67.674
673.15  5e+08  0.59917  -7721.7  18909  39.562  -22.467
```
