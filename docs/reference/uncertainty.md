<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Uncertainties and rounding

Every result has an uncertainty `err` beside its value `val`. A record can give an uncertainty for a value, and the engine carries it to every result.

## What `err` means

`err` is a first-order propagation of the uncertainties of the inputs: u(y)^2 = J S J^T. J holds the exact [derivatives](derivatives.md) of y by each uncertain value. S is the covariance matrix of those values. Without `covariances`, S is diagonal: the values are independent. The propagation is linear, so a large uncertainty of a nonlinear parameter is not covered.

Every uncertainty is kept as a standard uncertainty u (k = 1): an expanded one is divided by its coverage factor, the half-width of a uniform or triangular distribution by √3 or √6, and a legacy `error` is read as u. So `err` of a result is a standard uncertainty. Multiply it by k yourself for an expanded one, for example 2 for about 95 %.

| Source | How it counts |
|---|---|
| Reference values | Always. G(T) = G(Tr) - S(Tr) (T - Tr) + ... has (T - Tr) u(S). With `propagate_parameter_errors`, they count together with the parameters and the `covariances`. |
| A reaction from reactants | The errors are weighted by the coefficients: u(dG)^2 is the sum of nu^2 u^2. u(ln K) = u(dG)/(R T), and u(log K) = u(ln K)/ln 10. |
| Other properties | From U = H - P V, A = U - T S and S = (H - G)/T. |
| Parameters of methods | Only with `propagate_parameter_errors`. The record gives an `errors` array or an `uncertainty` next to the value of the parameter. |

Values of different records are independent. If a derivative is not known, `err` is `NaN` and the status says that the uncertainty is not complete: you never get a value that is too small and looks complete.

With `propagate_parameter_errors`, one more calculation is done for each value that has an uncertainty. A parameter of one record can be correlated with a parameter of another record only through `covariances`.

ThermoFun does not make uncertainties. Weighted means and extrapolations (for example the specific ion interaction theory) belong to data evaluation. The rules of propagation are those of the NEA TDB guidelines (Wanner and Östhols, TDB-3): independent quantities, weighted by the stoichiometric coefficients. A reaction measured directly has a smaller uncertainty than the same reaction from the formation data of its species, so use the reference values of the reaction (`thermoPropertiesReaction`) when the database has them.

How to use them: [Propagate uncertainties](../how-to/propagate-uncertainties.md).

## Rounding

With `round_to_uncertainty` (off by default), the engine rounds each result that has an uncertainty greater than 0. The derivatives and the results without an uncertainty are not rounded. It rounds the uncertainty up to `uncertainty_significant_digits`, and the value to the same decimals, as in NEA TDB-3. A 5 with nothing after it rounds to the even digit. The calculation itself keeps all digits.

```python
import thermofun as tf

print(tf.round_to_uncertainty(25.4478, 1.0834, 2))
print(tf.round_half_even(2.5, 0))
```

Output:

```text
(25.4, 1.1)
2.0
```

<!-- check
```python
assert tf.round_to_uncertainty(25.4478, 1.0834, 2) == (25.4, 1.1)
assert tf.round_half_even(2.5, 0) == 2.0
```
-->
