<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Fit parameters to measured data

This page tells you how to fit the parameters of a record to measured properties, and how to read the uncertainty of the result.

**You need:** ThermoFun with autodiff, and SciPy (`pip install thermofun[fitting]`). All numbers are in SI: V in m3/mol.

`pytests/test_fitting.py` runs every example of this page.

## 1. Name a parameter

A parameter is three words: the symbol of the record, the method, and the name of the parameter.

| You want | Method | Parameter |
|---|---|---|
| A reference value of a substance | `""` | `G`, `H`, `S`, `Cp`, `V` |
| A reference value of a reaction | `""` | `logK`, `dG`, `dH`, `dS`, `dCp`, `dV` |
| A parameter of a method | the method, for example `HKF88` | `a1` |
| A coefficient of the second T segment | `CpPolynomial` | `a0[1]` |
| A change at the first phase transition | `CpPolynomial` | `dS[0]`, `dH[0]`, `dV[0]` |

## 2. Get the derivatives

```python
import json
import thermofun as tf

mineral = {"class": "component", "aggregate_state": "crystal", "formula": "SiO2", "charge": 0,
           "reference_conditions": {"T": 298.15, "P": 1e5},
           "standard_state_properties": {"G": -856000.0, "H": -910830.0, "S": 50.0, "Cp": 44.9, "V": 2.27e-5},
           "methods": [{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 2273.15, "parameters": {
               "a0": 100.0, "a1": 0.0, "a2": 0.0, "a3": 0.0, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "a8": 0.0, "a9": 0.0}}]}]}
data = {"thermohub": "2.0.0-draft.1", "elements": {"Si": {"molar_mass": 0.0280855}, "O": {"molar_mass": 0.0159994}}, "substances": {"Qtz": mineral}}
engine = tf.ThermoEngine(tf.Database(json.dumps(data)))

derivatives = engine.parameterSensitivities(500.0, 1e5, "Qtz", [("Qtz", "", "S"), ("Qtz", "CpPolynomial", "a0")])
assert derivatives["G"]["Qtz/S"] == -(500.0 - 298.15)
```

The result has the properties first and the parameters second, so `pandas.DataFrame(derivatives)` gives a table. For a reaction, use
`reactionParameterSensitivities`.

The derivatives follow the same path as the values: see [Choose where properties come from](add-data.md#10-choose-where-properties-come-from). A substance from its defining reaction
has derivatives by the reaction and by the other species. A substance from its methods has none by them. An unknown parameter is an error, and not a derivative of 0.

A call has two parts, which can be different records: the record whose properties you want, and the record that holds the parameter:

```
engine.parameterSensitivities(T, P, "Na+", [("H2O@", "SverjenskyEtAl14", "b3")])
#                  how the properties of Na+  change with this parameter of H2O@
```

Here `b3` belongs to the dielectric model of `H2O@`. `Na+` is in the call because HKF uses the dielectric constant: `b3` changes the dielectric constant of `H2O@`, then the Born functions, then G of `Na+`.

`b3` is a built-in constant of the method, with its value in `ThermoFun/Methods/ParameterSets`. The difference between a parameter and a constant:

| | A parameter of a record | A built-in constant of a method |
|---|---|---|
| Example | `a1` of HKF on `Na+` | `eta` of HKF, `b3` of Sverjensky |
| `engine.parameters(symbol)` | lists it | does not list it: name it to use it |
| The derivative | exact, with autodiff | a central difference of the whole calculation: less exact, and slower |
| Which records it reaches | those that depend on the record | those that depend on the record, also a solute through the water |

Constants are not listed because a model can have hundreds of them, and a fit that goes over `engine.parameters(symbol)` should not change the table of a water model. An integer of a table, such as an exponent of the water code, has no derivative.

## 3. Change a parameter

A handle changes one value in the database of the engine. The engine calculates again only the results that depend on this record.

```python
a0 = engine.parameter("Qtz", "CpPolynomial", "a0")
a0.setValue(110.0)
assert engine.thermoPropertiesSubstance(500.0, 1e5, "Qtz").heat_capacity_cp.val == 110.0
```

The handle needs its engine. `engine.parameters("Qtz")` gives the handles of all the values of a record that you can fit.

## 4. Fit

```python
from thermofun import fitting

# measured: here calculated with known values, to see the fit find them
truth = tf.ThermoEngine(tf.Database(json.dumps(data)))
measured = []
for T in (300.0, 500.0, 700.0, 900.0):
    r = truth.thermoPropertiesSubstance(T, 1e5, "Qtz")
    measured.append(fitting.Observation("Qtz", "Cp", T, 1e5, r.heat_capacity_cp.val, 0.5))
    measured.append(fitting.Observation("Qtz", "G", T, 1e5, r.gibbs_energy.val, 50.0))

engine = tf.ThermoEngine(tf.Database(json.dumps(data)))
engine.parameter("Qtz", "CpPolynomial", "a0").setValue(80.0)   # a wrong start
problem = fitting.Problem(engine, [("Qtz", "", "G"), ("Qtz", "", "S"), ("Qtz", "CpPolynomial", "a0")], measured)
result = problem.solve()

assert abs(result.values["Qtz/CpPolynomial/a0"] - 100.0) < 1e-6
assert result.chi2 < 1e-6
```

The weight of an observation is its uncertainty. `Observation.property` is one of `G`, `H`, `S`, `Cp`, `V`, `U`, `A` of a substance, or `logK`, `lnK`, `dG`, `dH`, `dS`, `dCp`, `dV` of a reaction.

`result.covariance` is `inverse(J^T J)`, with `J` the weighted Jacobian at the solution. It means something only if the uncertainties of the observations are right: look at `result.chi2` against `result.degrees_of_freedom`.

## 5. Keep the result

```python
import os, tempfile

fitted = result.database()
path = os.path.join(tempfile.mkdtemp(), "fitted.json")
fitted.save(path)

engine = tf.ThermoEngine(tf.Database(path))
engine.preferences.propagate_parameter_errors = True
assert engine.thermoPropertiesSubstance(500.0, 1e5, "Qtz").gibbs_energy.err > 0.0
```

The saved records hold the fitted values, their uncertainties, and a `covariances` block. With `propagate_parameter_errors`, each result gets `err = sqrt(J S J^T)` from them.

A block belongs to one record. If a fit has parameters of two records, the covariance between them has no place in the file: `result.covariance_dropped` gives the number of values left out, and `result.covariance` still has them.
