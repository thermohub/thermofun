<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Uncertainties and rounding

`err` is a first-order propagation: u(y)^2 = J S J^T. J holds the exact derivatives of y by each uncertain value. S is the covariance matrix of those values. Without `covariances`, S is diagonal.

It keeps the meaning of the inputs. With 95 % uncertainties in the data, a result is at 95 % too.

| Source | How it counts |
|---|---|
| Reference values | Always. G(T) = G(Tr) - S(Tr) (T - Tr) + ... has (T - Tr) u(S). |
| A reaction from reactants | The errors are weighted by the coefficients. u(ln K) = u(dG)/(R T), and u(log K) = u(ln K)/ln 10. |
| Other properties | From U = H - P V and A = U - T S. |
| Parameters of methods | Only with `propagate_parameter_errors`. |

Values of different records are independent. If a derivative is not known, `err` is `NaN` and the status says that the uncertainty is not complete. Without autodiff, `propagate_parameter_errors` is an error.

ThermoFun does not make uncertainties. Weighted means and extrapolations belong to data evaluation.

## Rounding

With `round_to_uncertainty`, the engine rounds each result that has an uncertainty. It rounds the uncertainty up to `uncertainty_significant_digits`, and the value to the same decimals, as in NEA TDB-3. A 5 with nothing after it rounds to the even digit. The calculation itself keeps all digits.

```python
import thermofun as tf

assert tf.round_to_uncertainty(25.4478, 1.0834, 2) == (25.4, 1.1)
assert tf.round_half_even(2.5, 0) == 2.0
```
