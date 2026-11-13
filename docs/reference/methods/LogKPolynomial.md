# LogKPolynomial

log K polynomial.

Calculates log K = a0 + a1 T + a2/T + a3 ln T + a4/T2 + a5 T2 + a6/sqrt(T), and dH, dS and dCp from its derivatives. Without a0 to a6, it calculates them from the dCp coefficients a to e and the reference dH and dS: this equation is the integral of dCp = a + b T + c/T2 + d/sqrt(T) + e T2.

The id `LogKNordstromMunoz88` is the same method: the same calculation and parameters, under a name of the legacy files (`logk_nordstrom_munoz88`). It has no page of its own.

Source: Nordstrom and Munoz (1988); Anderson and Crerar (1993).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_fpt_function` |
| Other names | `LogKAnalytic` |

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

From Plummer and Busenberg (1982), Geochim. Cosmochim. Acta 46, 1011-1040; the analytic expression of calcite in phreeqc.dat. No bundled database has this method yet.

PHREEQC writes the fourth term as A4 log10(T). ThermoFun has a3 ln(T), so a3 = A4 / ln(10) = 71.595 / 2.302585.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48}
    methods:
      - method: LogKPolynomial
        parameters: {a0: -171.9065, a1: -0.077993, a2: 2839.319, a3: 31.093313}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 298.15 | 1 bar | logK | -8.48 | 0.005 |
