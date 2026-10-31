# LogKNordstromMunoz88

log K of Nordstrom and Munoz.

Calculates log K from the same equation as LogKPolynomial, in the form of Nordstrom and Munoz.

Source: Nordstrom and Munoz (1988).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_nordstrom_munoz88` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a0 | 1 | 1 | yes |  | constant term |
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

From Sn(Cl)+ in `mines16-thermofun.json`.

```yaml
- method: LogKNordstromMunoz88
  parameters:
    a0: 1.803
    a1: -0.002585
    a5: 4.424e-06
    b: -0.098979
    e: 0.000508181
```
