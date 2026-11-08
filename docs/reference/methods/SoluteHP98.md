# SoluteHP98

Aqueous solute of Holland and Powell.

Calculates the properties of an aqueous species from the density model of Holland and Powell. Implementation consistent (the default): S, H and Cp are the exact derivatives of G. Implementation published: the formulas of the paper, in which T' is a constant and S, Cp, H are not the derivatives of G. G and V are the same in both.

Source: Holland and Powell (1998).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_holland_powell98` |
| Other names | `HP98Sol` |

## Implementations

`consistent`, `published`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| b | kJ/(mol.K2) | J/(mol.K2) | yes |  | coefficient b |

## Example

No bundled record uses this method.
