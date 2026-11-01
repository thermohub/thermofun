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

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

The universal constants of the perturbation equation of state: its reference is the Weeks-Chandler-Andersen fluid. P1 to P10 and hs_a3, hs_a9, hs_a12: the hard sphere diameter. AA, A4 to A6, BB, B4 to B6, A00 to A34: the WCA free energy, with rm = 2^(1/6). fi1_N, fi2_N: the polynomials of the induced term, highest place last. kd0 to kd5: the correction of the dipole term. dipole: the conversion of the dipole moment, 1.38048E-4 as the constant of Boltzmann in the units of the model. NA = 0.6023: the number of Avogadro in the units of the model. PISIX and TWOPI: pi/6 and 2 pi with the digits of the code. All are numbers in the reduced units of the model. The coefficients of each substance are in its record.

Source: Churakov S.V., Gottschalk M. (2003) Geochim. Cosmochim. Acta 67, 2397-2414 and 2415-2425.

Names: `PISIX`, `TWOPI`, `rm`, `P1`, `PP2`, `P3` to `P10`, `AA1` to `AA3`, `A4` to `A6`, `BB1` to `BB3`, `B4` to `B6`, `A00`, `A01`, `A10` to `A12`, `A21` to `A23`, `A31` to `A34`, `fi1_0` to `fi1_9`, `fi2_0` to `fi2_9`, `hs_a3`, `hs_a9`, `hs_a12`, `NA`, `dipole`, `kd0` to `kd5`.

## Example

No bundled record uses this method.
