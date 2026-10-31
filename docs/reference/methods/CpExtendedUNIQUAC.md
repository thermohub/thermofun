# CpExtendedUNIQUAC

Heat capacity of Extended UNIQUAC.

Integrates Cp = a + b T + c / (T - Theta) and adds a pressure term.

Source: Thomsen and Rasmussen (1999).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Convention of its results | gibbs_helmholtz_formation |
| Legacy names | `cp_ft_equation_uniquac` |
| Other names | `ExtendedUNIQUAC`, `StandardThermoModelExtendedUNIQUAC` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | J/(mol.K) | J/(mol.K) | yes |  | constant term |
| b | J/(mol.K2) | J/(mol.K2) | yes |  | term b T |
| c | J/mol | J/mol | yes |  | term c / (T - Theta) |
| alpha | 1/bar | 1/Pa | yes |  | pressure term, first coefficient |
| beta | 1/bar2 | 1/Pa2 | yes |  | pressure term, second coefficient |
| Theta | K | K | no | 200 | temperature of the pole of the heat capacity |

## Example

No bundled record uses this method.
