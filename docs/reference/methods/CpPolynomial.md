# CpPolynomial

Heat capacity polynomial.

Integrates Cp = a0 + a1 T + a2/T2 + a3/sqrt(T) + a4 T2 + a5 T3 + a6 T4 + a7/T3 + a8/T + a9 sqrt(T) over T segments, with phase transitions.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | yes |
| Has code | yes |
| Legacy names | `cp_ft_equation` |
| Other names | `CpIntegration` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a0 | J/(mol.K) | J/(mol.K) | yes |  | constant term |
| a1 | J/(mol.K2) | J/(mol.K2) | no |  | term a1 T |
| a2 | J.K/mol | J.K/mol | no |  | term a2 / T2 |
| a3 | J/(mol.K^0.5) | J/(mol.K^0.5) | no |  | term a3 / sqrt(T) |
| a4 | J/(mol.K3) | J/(mol.K3) | no |  | term a4 T2 |
| a5 | J/(mol.K4) | J/(mol.K4) | no |  | term a5 T3 |
| a6 | J/(mol.K5) | J/(mol.K5) | no |  | term a6 T4 |
| a7 | J.K2/mol | J.K2/mol | no |  | term a7 / T3 |
| a8 | J/mol | J/mol | no |  | term a8 / T |
| a9 | J/(mol.K^1.5) | J/(mol.K^1.5) | no |  | term a9 sqrt(T) |

## Example

From Akermanite in `aq17-thermofun.json`.

```yaml
- method: CpPolynomial
  segments:
    - t_min: 273.15
      t_max: 2273.15
      parameters:
        a0: 385.4
        a1: 0.003209
        a2: -247500
        a3: -2889.9
```
