# MarshallFranck81

Ion product of water of Marshall and Franck.

Calculates log K = a + b/T + c/T2 + d/T3 + (e + f/T + g/T2) log10(density of water).

Source: Marshall and Franck (1981).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_marshall_frank78` |
| Other names | `FrantzMarshall`, `MarshallFranck78` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | 1 | 1 | yes |  | constant term |
| b | K | K | yes |  | term b / T |
| c | K2 | K2 | yes |  | term c / T2 |
| d | K3 | K3 | yes |  | term d / T3 |
| e | 1 | 1 | yes |  | density term, constant |
| f | K | K | yes |  | density term, f / T |
| g | K2 | K2 | yes |  | density term, g / T2 |

## Example

From Marshall and Franck (1981), J. Phys. Chem. Ref. Data 10, 295-304; coefficients as in Dolejs (2013), Eq. 44. No bundled database has this method yet.

The ion product of water. The method reads the density of water from the water record of the database.

```yaml
reactions:
  Water-dissociation:
    equation: H2O@ = H+ + OH-
    reaction_properties: {logK: -13.995}
    methods:
      - method: MarshallFranck81
        parameters: {a: -4.098, b: -3245.2, c: 2.2362e+5, d: -3.984e+7, e: 13.957, f: -1262.3, g: 8.5641e+5}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Water-dissociation | 298.15 | saturation | logK | -13.995 | 0.005 |
