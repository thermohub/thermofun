# LogKVantHoff

log K of van 't Hoff.

Calculates log K = (dS - dH/T) / (R ln 10) with dH and dS that do not change with T (dCp = 0). Reads log K or dG, and dH or dS, at Tr (the third of log K, dH, dS follows from the other two); without dH and dS, log K stays constant. It is LogKPolynomial with two terms: a0 = dS / (R ln 10), a2 = -dH / (R ln 10).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_2_term_extrap` |
| Other names | `LogKExtrap2T` |

## Parameters

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

From Plummer and Busenberg (1982), as written in phreeqc.dat (log_k -8.48, delta_h -2.297 kcal). No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48, dH: -9610.6}
    methods:
      - method: LogKVantHoff
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 298.15 | 1 bar | logK | -8.48 | 1e-09 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Calcite-dissolution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
298.15  100000  -8.48  48404  -9610.6  -194.58  0
```
