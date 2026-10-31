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

No bundled record uses this method.
