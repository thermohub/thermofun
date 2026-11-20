# LogKZeroEntropy

log K with zero entropy of reaction.

Calculates log K = -dG / (RT ln 10) with dS = 0, so dH = dG, the reference value. Reads log K or dG at Tr. It is LogKPolynomial with one term: a2 = -dG(Tr) / (R ln 10).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_1_term_extrap1` |
| Other names | `LogKExtrap1T1` |

## Parameters

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

From the definition of the method (dS = 0, so log K = log K(Tr) Tr / T); the log K of calcite of Plummer and Busenberg (1982) at 25 degC. No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48}
    methods:
      - method: LogKZeroEntropy
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 298.15 | 1 bar | logK | -8.48 | 1e-09 |
| Calcite-dissolution | 373.15 | 1 bar | logK | -6.77559 | 1e-05 |

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
298.15  100000  -8.48  48404  48404  0  0
373.15  100000  -6.7756  48404  48404  0  0
```
