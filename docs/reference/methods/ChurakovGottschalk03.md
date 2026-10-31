# ChurakovGottschalk03

Equation of state of Churakov and Gottschalk.

Adds the pressure terms of the perturbation equation of state to G, H and S, and sets V.

Source: Churakov and Gottschalk (2003a, b).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_churakov_gottschalk` |
| Other names | `CGF` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| sigma0 | 1 | 1 | yes |  | sigma = sigma0 + sigma1 exp(sigma2 T), angstrom |
| eps0 | 1 | 1 | yes |  | epsilon = eps0 + eps1 exp(eps2 T), K |
| mu0 | 1 | 1 | yes |  | dipole = mu0 + mu1 / (T + mu2), debye |
| pol0 | 1 | 1 | yes |  | polarizability = pol0 + pol1 / (T + pol2), angstrom3 |
| sigma1 | 1 | 1 | yes |  | see sigma0, angstrom |
| sigma2 | 1 | 1 | yes |  | see sigma0, 1/K |
| eps1 | 1 | 1 | yes |  | see eps0, K |
| eps2 | 1 | 1 | yes |  | see eps0, 1/K |
| mu1 | 1 | 1 | yes |  | see mu0, debye K |
| mu2 | 1 | 1 | yes |  | see mu0, K |
| pol1 | 1 | 1 | yes |  | see pol0, angstrom3 K |
| pol2 | 1 | 1 | yes |  | see pol0, K |

## Example

No bundled record uses this method.
