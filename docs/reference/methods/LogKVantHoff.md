# LogKVantHoff

log K of van 't Hoff.

Calculates log K with dH and dS that do not change with T (dCp = 0). The coefficients of log K(T), if given, only check the reference value.

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

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a0 | 1 | 1 | no |  | constant term |
| a1 | 1/K | 1/K | no |  | term a1 T |
| a2 | K | K | no |  | term a2 / T |
| a3 | 1 | 1 | no |  | term a3 ln T |
| a4 | K2 | K2 | no |  | term a4 / T2 |
| a5 | 1/K2 | 1/K2 | no |  | term a5 T2 |
| a6 | K^0.5 | K^0.5 | no |  | term a6 / sqrt(T) |
| a | J/(mol.K) | J/(mol.K) | no |  | constant term |
| b | J/(mol.K2) | J/(mol.K2) | no |  | term b T |
| c | J.K/mol | J.K/mol | no |  | term c / T2 |
| d | J/(mol.K^0.5) | J/(mol.K^0.5) | no |  | term d / sqrt(T) |
| e | J/(mol.K3) | J/(mol.K3) | no |  | term e T2 |

## Example

No bundled record uses this method.
