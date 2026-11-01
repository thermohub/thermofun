<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Fit parameters

You learn to fit a parameter of a mineral to measured heat capacities. It takes 15 minutes.

**You need:** ThermoFun with autodiff, SciPy (`pip install thermofun[fitting]`), and the file `examples/v2/mineral-methods.yaml` of the repository.

## 1. Make the measurements

This tutorial makes its own data, so that you can see the fit find the known value. Use your measurements in real work.

```python
import thermofun as tf
from thermofun import fitting

truth = tf.ThermoEngine("examples/v2/mineral-methods.yaml")
measured = []
for T in (600.0, 800.0, 1000.0, 1100.0, 1200.0):                # K
    Cp = truth.thermoPropertiesSubstance(T, 1e5, "Calcite").heat_capacity_cp.val
    measured.append(fitting.Observation("Calcite", "Cp", T, 1e5, Cp, 0.5))   # the last value is the uncertainty, J/(mol K)
```

## 2. Start from a wrong value

```python
engine = tf.ThermoEngine("examples/v2/mineral-methods.yaml")
Smax = engine.parameter("Calcite", "LandauHP98", "Smax")
assert Smax.value() == 10.0                                     # J/(mol K)
Smax.setValue(7.0)
```

## 3. Look at the derivative

The fit uses exact derivatives. You can read them.

```python
derivatives = engine.parameterSensitivities(800.0, 1e5, "Calcite", [("Calcite", "LandauHP98", "Smax")])
assert derivatives["Cp"]["Calcite/LandauHP98/Smax"] > 0.0
```

## 4. Fit

```python
problem = fitting.Problem(engine, [("Calcite", "LandauHP98", "Smax")], measured)
result = problem.solve()
print(result.values, result.chi2)
assert abs(result.values["Calcite/LandauHP98/Smax"] - 10.0) < 1e-4
```

`result.covariance` gives the uncertainty of the parameter, if the uncertainties of the measurements are correct.

**Next:** [Batch calculations](05-batch-calculations.md). The details are in [Fit parameters to measured data](../how-to/fit-parameters.md).
