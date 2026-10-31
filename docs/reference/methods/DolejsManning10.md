# DolejsManning10

Density model of Dolejs and Manning.

Calculates dG = a + b T + c T ln T + d T2 + e T ln(density of water), and log K from it.

Source: Dolejs and Manning (2010).

| | |
|---|---|
| Works on | reaction |
| Kind | base method |
| Classes | any |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `logk_dolejs_manning10` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a | kJ/mol | J/mol | yes |  | enthalpy term |
| b | J/(mol.K) | J/(mol.K) | yes |  | entropy term |
| c | J/(mol.K) | J/(mol.K) | no |  | heat capacity, constant term |
| d | J/(mol.K2) | J/(mol.K2) | no |  | heat capacity, linear term |
| e | J/(mol.K) | J/(mol.K) | yes |  | density term |

## Example

No bundled record uses this method.
