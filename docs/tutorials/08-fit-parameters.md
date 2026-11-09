---
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors
jupytext:
  text_representation:
    extension: .md
    format_name: myst
    format_version: 0.13
kernelspec:
  display_name: Python 3
  language: python
  name: python3
---

# 8. Fit parameters

You learn to fit a parameter of a mineral to measured heat capacities. It takes 15 minutes.

**You need:** ThermoFun with autodiff, SciPy (`pip install thermofun[fitting]`), and the file `mineral-methods.yaml` from `examples/v2/` of the repository, in your folder.

## Make the measurements

This tutorial makes its own data, so that you can see the fit find the known value. Use your measurements in real work.

```{code-cell} ipython3
import thermofun as tf
from thermofun import fitting

truth = tf.ThermoEngine("mineral-methods.yaml")
measured = []
for T in (600.0, 800.0, 1000.0, 1100.0, 1200.0):                # K
    Cp = truth.thermoPropertiesSubstance(T, 1e5, "Calcite").heat_capacity_cp.val
    measured.append(fitting.Observation("Calcite", "Cp", T, 1e5, Cp, 0.5))   # the last value is the uncertainty, J/(mol K)
```

## Start from a wrong value

```{code-cell} ipython3
engine = tf.ThermoEngine("mineral-methods.yaml")
Smax = engine.parameter("Calcite", "LandauHP98", "Smax")
print(Smax.value(), "J/(mol K) in the file")
Smax.setValue(7.0)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert truth.parameter("Calcite", "LandauHP98", "Smax").value() == 10.0
```

## Look at the derivative

The fit uses exact derivatives. You can read them.

```{code-cell} ipython3
derivatives = engine.parameterSensitivities(800.0, 1e5, "Calcite", [("Calcite", "LandauHP98", "Smax")])
print(derivatives["Cp"]["Calcite/LandauHP98/Smax"])
```

```{code-cell} ipython3
:tags: [remove-cell]
assert derivatives["Cp"]["Calcite/LandauHP98/Smax"] > 0.0
```

## Fit

```{code-cell} ipython3
problem = fitting.Problem(engine, [("Calcite", "LandauHP98", "Smax")], measured)
result = problem.solve()
print(result.values, result.chi2)
```

```{code-cell} ipython3
:tags: [remove-cell]
assert abs(result.values["Calcite/LandauHP98/Smax"] - 10.0) < 1e-4
```

`result.covariance` gives the uncertainty of the parameter, if the uncertainties of the measurements are correct.

The details are in [Fit parameters to measured data](../how-to/fit-parameters.md).
