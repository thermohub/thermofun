# ReactionCpPolynomial

Heat capacity of reaction.

Gives dCp = a + b T + c/T2 + d/sqrt(T) + e T2; the log K methods use it.

| | |
|---|---|
| Works on | reaction |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `dr_heat_capacity_ft` |
| Other names | `DrCpIntegration` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | J/(mol.K) | J/(mol.K) | yes |  | constant term |
| b | J/(mol.K2) | J/(mol.K2) | no |  | term b T |
| c | J.K/mol | J.K/mol | no |  | term c / T2 |
| d | J/(mol.K^0.5) | J/(mol.K^0.5) | no |  | term d / sqrt(T) |
| e | J/(mol.K3) | J/(mol.K3) | no |  | term e T2 |

## Example

From the closed form of a constant dCp (Dolejs 2013, Eqs. 36 and 38) with the log K and dH of calcite of Plummer and Busenberg (1982); dCp = 50 J/(mol K) is an illustrative value. No bundled database has this method yet.

The method gives dCp only. A method of log K, here `LogKVantHoff`, integrates it, so the two are given together.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48, dH: -9610.6}
    methods:
      - method: ReactionCpPolynomial
        parameters: {a: 50.0}
      - method: LogKVantHoff
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 298.15 | 1 bar | logK | -8.48 | 1e-09 |
| Calcite-dissolution | 373.15 | 1 bar | logK | -8.75732 | 1e-05 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (373.15, 100000)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Calcite-dissolution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
298.15  100000  -8.48  48404  -9610.6  -194.58  50
373.15  100000  -8.7573  62561  -5860.6  -183.36  50
```
