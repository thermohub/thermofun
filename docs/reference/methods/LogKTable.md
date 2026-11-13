# LogKTable

Table of log K.

Interpolates a table of log K(T), with Lagrange polynomials or a cubic spline.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_lagrange_interp` |
| Other names | `LogKLagrange` |

## Example

From values of the equation of Plummer and Busenberg (1982) for calcite at 0, 25, 50, 75 and 100 degC. No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48}
    methods:
      - method: LogKTable
        interpolation: lagrange
        order: 3
        table:
          temperatures: [273.15, 298.15, 323.15, 348.15, 373.15]
          logK: [-8.3814, -8.4798, -8.6628, -8.9266, -9.2665]
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 310.15 | 1 bar | logK | -8.5573 | 0.002 |
