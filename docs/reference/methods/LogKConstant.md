# LogKConstant

Constant log K.

Keeps log K at the reference value at every T: dH = dS = dCp = 0. Reads log K or dG at Tr. It is LogKPolynomial with one term: a0 = log K(Tr).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_1_term_extrap0` |
| Other names | `LogKExtrap1T0` |

## Parameters

None. The method reads only the reference values of the reaction, and makes its terms of log K from them (see above).

Not read: `a0` to `a6`, `a`, `b`, `c`, `d`, `e`. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated them; a file can keep them, and they change nothing.

## Example

From Plummer and Busenberg (1982), log K of calcite at 25 degC. No bundled database has this method yet.

Without dH, dS and dCp a record keeps log K at every T. That is reasonable near 25 degC only.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48}
    methods:
      - method: LogKConstant
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 323.15 | 1 bar | logK | -8.48 | 1e-09 |
