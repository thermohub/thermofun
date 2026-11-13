# ConstantReactionProperties

Constant reaction properties.

Gives the reference reaction properties at every T and P.

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Example

From Plummer and Busenberg (1982), as written in phreeqc.dat. No bundled database has this method yet.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48, dH: -9610.6}
    methods:
      - method: ConstantReactionProperties
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 323.15 | 1 bar | logK | -8.48 | 1e-09 |
