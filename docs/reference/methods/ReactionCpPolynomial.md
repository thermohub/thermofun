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

No bundled record uses this method.
