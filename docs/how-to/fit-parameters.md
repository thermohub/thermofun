<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Fit parameters to measured data

```{admonition} Under development
:class: warning
Fitting is under development. It will move to a separate library, ThermoFit; ThermoFun will keep the sensitivities and the contributions of the methods. The interface of `thermofun.fitting` can change.
```

**Related:** [Tutorial 7: Fit parameters](../tutorials/07-fit-parameters.md) · [HKF88](../reference/methods/HKF88.md) · [Derivatives with autodiff](../reference/derivatives.md) · [Uncertainties](../reference/uncertainty.md)

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
import thermofun as tf

mineral = {"class": "component", "aggregate_state": "crystal", "formula": "SiO2", "charge": 0,
           "reference_conditions": {"T": 298.15},
           "standard_state_properties": {"G": -856000.0, "H": -910830.0, "S": 50.0, "Cp": 44.9, "V": 2.27e-5},
           "methods": [{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 2273.15, "parameters": {
               "a0": 100.0, "a1": 0.0, "a2": 0.0, "a3": 0.0, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "a8": 0.0, "a9": 0.0}}]}]}
data = {"thermohub": "2.0.0-draft.1", "elements": {"Si": {"molar_mass": 0.0280855}, "O": {"molar_mass": 0.0159994}}, "substances": {"Qtz": mineral}}
engine = tf.ThermoEngine(tf.Database(data))

derivatives = engine.parameterSensitivities(500.0, 1e5, "Qtz", [("Qtz", "", "S"), ("Qtz", "CpPolynomial", "a0")])
```

<!-- check
```python
assert derivatives["G"]["Qtz/S"] == -(500.0 - 298.15)
```
-->

The result has the properties first and the parameters second, so `pandas.DataFrame(derivatives)` gives a table. For a reaction, give
its symbol: the results are then logK, lnK, dG, dH, dS, dCp and dV.

A substance defined by a reaction often has the symbol of the reaction. The symbol then names two records, and `kind` says
which one you want: `"substance"` (the default) or `"reaction"`, as in `engine.route(symbol, kind)`. The parameter needs no
kind: `logK` is a value of a reaction, `G` a value of a substance.

```python
data["substances"]["SiO2@"] = {"class": "aqueous_solute", "aggregate_state": "aqueous", "formula": "SiO2", "charge": 0, "defining_reaction": "SiO2@"}
data["reactions"] = {"SiO2@": {"equation": "Qtz = SiO2@", "reaction_properties": {"logK": -4.0, "dH": 22000.0}, "methods": [{"method": "LogKVantHoff"}]}}
engine = tf.ThermoEngine(tf.Database(data))

of_substance = engine.parameterSensitivities(373.15, 1e5, "SiO2@", [("SiO2@", "", "logK")])
of_reaction = engine.parameterSensitivities(373.15, 1e5, "SiO2@", [("SiO2@", "", "logK")], kind="reaction")
print("dG/dlogK of the substance", round(of_substance["G"]["SiO2@/logK"], 3), "J/mol")
print("dlogK/dlogK of the reaction", round(of_reaction["logK"]["SiO2@/logK"], 6))
```

Output:

```text
dG/dlogK of the substance -7143.907 J/mol
dlogK/dlogK of the reaction 1.0
```

The first is -RT ln 10 at 373.15 K: G of SiO2@ is G of Qtz minus RT ln 10 log K.

The methods of log K (LogKVantHoff, LogKConstantCp, ...) calculate from dH and dS at Tr, and dCp. logK (or dG) is read only
to give the one of dH and dS that the record lacks. A record that gives logK, dH and dS has derivatives by dH and dS, and 0
by logK: give logK and dH, without dS, to fit logK.

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

Model parameters are not listed because a model can have hundreds of them, and a fit that goes over `engine.parameters(symbol)` should not change the table of a water model. An integer of a table, such as an exponent of the water code, has no derivative.

## 3. Change a parameter

A handle changes one value in the database of the engine. The engine calculates again only the results that depend on this record.

```python
a0 = engine.parameter("Qtz", "CpPolynomial", "a0")
a0.setValue(110.0)
print(engine.thermoPropertiesSubstance(500.0, 1e5, "Qtz").heat_capacity_cp.val)  # prints 110.0
```

<!-- check
```python
assert engine.thermoPropertiesSubstance(500.0, 1e5, "Qtz").heat_capacity_cp.val == 110.0
```
-->

The handle needs its engine. `engine.parameters("Qtz")` gives the handles of all the values of a record that you can fit.

## 4. Fit

```python
from thermofun import fitting

# measured: here calculated with known values, to see the fit find them
true_engine = tf.ThermoEngine(tf.Database(data))
T = [300.0, 500.0, 700.0, 900.0]
measured = (fitting.calculated(true_engine, "Qtz", "Cp", T, 1e5, uncertainty=0.5)
            + fitting.calculated(true_engine, "Qtz", "G", T, 1e5, uncertainty=50.0))

engine = tf.ThermoEngine(tf.Database(data))
engine.parameter("Qtz", "CpPolynomial", "a0").setValue(80.0)   # a wrong start
problem = fitting.Problem(engine, [("Qtz", "", "G"), ("Qtz", "", "S"), ("Qtz", "CpPolynomial", "a0")], measured)
result = problem.solve()
```

<!-- check
```python
assert abs(result.values["Qtz/CpPolynomial/a0"] - 100.0) < 1e-6
assert result.chi2 < 1e-6
```
-->

`fitting.observations(symbol, property, T, P, values, uncertainties, unit, T_unit)` makes the observations of one property at many points, from your numbers and their unit; `fitting.calculated(engine, symbol, property, T, P, uncertainty)` makes them with an engine, as here. One `fitting.Observation(symbol, property, T, P, value, uncertainty)` is one point, in SI.

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
```

<!-- check
```python
assert engine.thermoPropertiesSubstance(500.0, 1e5, "Qtz").gibbs_energy.err > 0.0
```
-->

The saved records hold the fitted values, their uncertainties, and a `covariances` block. With `propagate_parameter_errors`, each result gets `err = sqrt(J S J^T)` from them.

A block belongs to one record. If a fit has parameters of two records, the covariance between them has no place in the file: `result.covariance_dropped` gives the number of values left out, and `result.covariance` still has them.

## 6. Fit with constraints

Some fits must keep a value or a relation exactly, whatever the data say. Two kinds of constraint do this:

| Constraint | Holds | Example |
|---|---|---|
| `fitting.PropertyConstraint(symbol, property, T, P, value)` | a property at T, P | Cp and V at 25 °C from a calorimetric and a density measurement |
| `fitting.Relation({parameter: coefficient, ...}, value)` | a linear relation among parameters | a4 = −4.134 a2 − 27790 cal of HKF (Shock and Helgeson 1988) |

A parameter that enters the model far from linearly can be **searched**: `Problem(..., search=parameter)` tries values of
it one at a time, and solves the other parameters for each value. This is how HKF parameters are fitted: omega in the
search, all the others solved. A fit of all of them at once can go astray from a poor start.

The example is G of UO2+2 along the saturation pressure, from 10 to 300 °C, with the constraints of the program OptimB
(Shvarov 2015): Cp and V at 25 °C and the relation of a2 and a4. G and S stay at their values; a1, a4, c1, c2 and omega
are fitted. The start values are estimates from correlations.

```python
CAL = 4.184   # J/cal
uo2 = {"formula": "UO2+2", "charge": 2, "class": "aqueous_solute", "aggregate_state": "aqueous",
       "standard_state_properties": {"G": -227761 * CAL, "S": -23.64 * CAL},
       "methods": [{"method": "HKF88", "units": {"energy": "cal", "pressure": "bar"}, "parameters": {
           "a1": 0.3088, "a2": -24.03, "a3": 5.844, "a4": -27690.7, "c1": 25.37, "c2": -9894.5, "omega": 144618.0}}]}
water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous",
         "standard_state_properties": {"G": -237181.0, "H": -285881.0, "S": 69.92},
         "methods": [{"method": "WaterHGK84"}, {"method": "JohnsonNorton91"}]}
engine = tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "UO2+2": uo2}}))

# G of UO2+2 at the saturation pressure (P = 0), Hovey et al. (1989)
measured = fitting.observations("UO2+2", "G", P=0.0, unit="kJ/mol", T_unit="C",
    T=[10, 25, 40, 50, 55, 100, 150, 200, 250, 300],
    values=[-954.45, -952.95, -951.48, -950.52, -950.04, -945.78, -941.02, -936.02, -930.6, -924.6],
    uncertainties=[0.5, 0.5, 0.5, 1.0, 0.5, 1.0, 1.0, 1.0, 2.0, 2.0])

constraints = [
    fitting.PropertyConstraint("UO2+2", "Cp", 298.15, 1e5, 10.04 * CAL),   # Cp at 25 °C, J/(mol K)
    fitting.PropertyConstraint("UO2+2", "V", 298.15, 1e5, 6.1e-6),         # V at 25 °C, m3/mol
    fitting.Relation({("UO2+2", "HKF88", "a4"): 1.0, ("UO2+2", "HKF88", "a2"): 4.134}, -27790 * CAL),
]
free = [("UO2+2", "HKF88", "a1"), ("UO2+2", "HKF88", "a4"), ("UO2+2", "HKF88", "c1"), ("UO2+2", "HKF88", "c2"),
        ("UO2+2", "HKF88", "omega")]
problem = fitting.Problem(engine, free, measured, constraints, search=("UO2+2", "HKF88", "omega"))
result = problem.solve(search_step=1000 * CAL)

print(result.success)
print("omega", round(result.values["UO2+2/HKF88/omega"] / CAL), "cal/mol")
print("c1", round(result.values["UO2+2/HKF88/c1"] / CAL, 2), "cal/(mol K)")
print("weighted rms", round(result.weighted_rms / 1000, 4), "kJ/mol")
r = engine.thermoPropertiesSubstance(298.15, 1e5, "UO2+2")
print("Cp", round(r.heat_capacity_cp.val / CAL, 4), "cal/(mol K), V", round(r.volume.val * 1e6, 4), "cm3/mol")
```

Output:

```text
True
omega 97378 cal/mol
c1 11.66 cal/(mol K)
weighted rms 0.0134 kJ/mol
Cp 10.04 cal/(mol K), V 6.1 cm3/mol
```

<!-- check
```python
assert all(abs(x) < 1e-6 for x in result.constraint_residuals)
assert result.degrees_of_freedom == 10 + 3 - 5
assert abs(result.values["UO2+2/HKF88/a4"] + 4.134 * engine.parameter("UO2+2", "HKF88", "a2").value() + 27790 * CAL) < 1e-6
```
-->

Cp and V at 25 °C are those given, and a4 follows a2. What to read in the result:

- `result.weighted_rms` is the typical deviation of the fit, in the unit of the observations:
  sqrt(chi2 / sum of 1/uncertainty²). With right uncertainties it is below the largest of them.
- `result.constraint_residuals` gives, for each constraint, the calculated minus the given value: 0 within rounding.
- `result.degrees_of_freedom` is the number of observations plus constraints minus parameters. The covariance keeps
  to what the constraints leave free: a combination of parameters that a relation fixes has no variance.

The search takes `search_step` (the first step, here 1000 cal/mol as in OptimB; by default 1 % of the start value) and
`search_tol` (when to stop; by default 1/100 of the step). The step gets smaller near the minimum.
`search_fixed_step=True` keeps it, as OptimB does: the result then differs a little from the minimum.

**Values from another program.** Cp and V at 25 °C of HKF contain the Born functions of water, which depend on the
dielectric model. OptimB, and fits of the SUPCRT era, use fixed values (Q = 5.903e-7 1/bar, X = −3.09e-7 1/K²,
Shock and Helgeson 1988); ThermoFun calculates them from the water methods of the engine. To impose Cp and V as such a
program does, add the difference times omega with `terms`:

```python
born = engine.electroPropertiesSolvent(298.15, 1e5, "H2O@")
omega = ("UO2+2", "HKF88", "omega")
cp = fitting.PropertyConstraint("UO2+2", "Cp", 298.15, 1e5, 10.04 * CAL, terms={omega: -298.15 * (born.bornX.val + 3.09e-7)})
v = fitting.PropertyConstraint("UO2+2", "V", 298.15, 1e5, 6.1e-6, terms={omega: born.bornQ.val - 5.903e-12})
```

<!-- check
```python
same = fitting.Problem(engine, free, measured, [cp, v, constraints[2]], search=omega).solve(search_step=1000 * CAL)
assert same.success and abs(same.values["UO2+2/HKF88/omega"] / CAL - 96871) < 5
```
-->

With them, omega is 96871 cal/mol: OptimB gives 96374 with its own water model.

## 7. Fit from a template and a task

Section 6 set up a fit by hand: the free parameters, the constraints, the searched parameter and its step. What is the
same for every fit of one method goes in a **fit template**. What belongs to one fit goes in a **task**: the record, the
given values and the data.

ThermoFun has one fit template, `HKF88`: the file `HKF88.yaml` in the folder `fit_templates` of the installed package
(`fitting.TEMPLATE_FOLDER`). `fitting.catalog()` lists the fit templates (a row each: `name`, `method`, `summary`,
`presets`, `source`), and `fitting.template(name)` gives one as a dict. The fit of section 6 takes its pieces from here:

| Section 6, by hand | Section 7 |
|---|---|
| `free = [a1, a4, c1, c2, omega]` | template: preset `psat_only` |
| `PropertyConstraint(..., "Cp", 298.15, 1e5, 10.04 * CAL)` | template: constraint `Cp_reference`; task: `given: {Cp: ...}` |
| `PropertyConstraint(..., "V", 298.15, 1e5, 6.1e-6)` | template: constraint `V_reference`; task: `given: {V: ...}` |
| `Relation({a4: 1.0, a2: 4.134}, -27790 * CAL)` | template: constraint `a2_a4_SH88` |
| `search=omega`, `search_step=1000 * CAL` | template: `omega: {search: {step: 1000}}` |
| the Born functions of another program (`terms=`) | template: convention `born_reference`; task: `conventions` |
| `fitting.observations(...)` | task: `data` |
| the record | task: `record` |

The part of `HKF88.yaml` that this task uses (the file also has the parameters G, S, a2, a3, two more constraints, the
rules for the warnings and a second preset):

```yaml
fit_template:
  name: HKF88
  method: HKF88
  parameters:
    a1:    {unit: cal/(mol*bar), display_scale: 10}
    a4:    {unit: cal*K/mol, display_scale: 1.0e-4}
    c1:    {unit: cal/(mol*K)}
    c2:    {unit: cal*K/mol, display_scale: 1.0e-4}
    omega: {unit: cal/mol, display_scale: 1.0e-5, search: {step: 1000, tol: 0.01}}   # step and tol in cal/mol
  conventions:
    born_reference:
      default: solvent
      choices:
        solvent: {}
        SH88_fixed: {Q: 5.903e-12, X: -3.09e-7}
  constraints:
    Cp_reference:
      property: Cp
      value: given.Cp
      terms: {omega: "-Tr*(solvent.bornX - born_reference.X)"}
    V_reference:
      property: V
      value: given.V
      terms: {omega: "solvent.bornQ - born_reference.Q"}
    a2_a4_SH88:
      relation: {a4: 1, a2: 4.134}
      value: "-27790*cal"
  presets:
    psat_only:
      free: [a1, a4, c1, c2, omega]
      constraints: [Cp_reference, V_reference, a2_a4_SH88]
```

With the default convention `solvent`, the terms with the Born functions are 0, and Cp and V are those of section 6. With
`SH88_fixed` they are the terms of "Values from another program" above.

<!-- check
```python
import os
assert os.path.isfile(os.path.join(fitting.TEMPLATE_FOLDER, "HKF88.yaml"))
template = fitting.template("HKF88")
assert template["presets"]["psat_only"] == {"free": ["a1", "a4", "c1", "c2", "omega"], "constraints": ["Cp_reference", "V_reference", "a2_a4_SH88"]}
assert template["parameters"]["omega"] == {"unit": "cal/mol", "display_scale": 1.0e-5, "search": {"step": 1000, "tol": 0.01}}
assert template["constraints"]["a2_a4_SH88"]["relation"] == {"a4": 1, "a2": 4.134}
assert template["constraints"]["Cp_reference"]["terms"] == {"omega": "-Tr*(solvent.bornX - born_reference.X)"}
assert template["conventions"]["born_reference"]["choices"]["SH88_fixed"] == {"Q": 5.903e-12, "X": -3.09e-7}
```
-->

The task, with the data of section 6:

```python
task = fitting.Task(tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "UO2+2": uo2}})), {
    "template": "HKF88", "record": "UO2+2", "preset": "psat_only",
    "given": {"Cp": "10.04 cal/(mol*K)", "V": "6.1 cm3/mol"},
    "data": {"property": "G", "unit": "kJ/mol", "columns": ["T_C", "P_bar", "value", "uncertainty"],
             "rows": [[10, 0, -954.45, 0.5], [25, 0, -952.95, 0.5], [40, 0, -951.48, 0.5], [50, 0, -950.52, 1.0], [55, 0, -950.04, 0.5],
                      [100, 0, -945.78, 1.0], [150, 0, -941.02, 1.0], [200, 0, -936.02, 1.0], [250, 0, -930.6, 2.0], [300, 0, -924.6, 2.0]]}})
print(task.free, task.constraints_on, task.warnings)
result = task.solve()
shown = task.report(result)
print("omega", round(shown["omega"]["value"], 4), shown["omega"]["unit"], "x", shown["omega"]["display_scale"])
```

Output:

```text
['a1', 'a4', 'c1', 'c2', 'omega'] ['Cp_reference', 'V_reference', 'a2_a4_SH88'] []
omega 0.9738 cal/mol x 1e-05
```

<!-- check
```python
assert abs(result.values["UO2+2/HKF88/omega"] / CAL - 97378) < 1
```
-->

The preset `psat_only` frees a1, a4, c1, c2 and omega and switches on the three constraints. What a task can change:

| Key | Does |
|---|---|
| `preset` | the free parameters and the constraints that are on |
| `free` | the free parameters, instead of those of the preset |
| `switches` | constraints on or off, for example `{"V_reference": "off", "c2_from_Cp_SSH97": "on"}` |
| `conventions` | for example `{"born_reference": "SH88_fixed"}`: Cp and V as OptimB imposes them |
| `start` | start values, in the units of the template |
| `data` | `rows` or `file` (CSV with a header); columns `T_C` or `T_K`, `P_bar`, `P_MPa` or `P_Pa` (0: Psat), `value`, and `uncertainty` or `weight` (1/uncertainty; 0 leaves the point out) |

A task is also a YAML or JSON file with the same keys under `fit_task`: `fitting.Task(engine, "uo2.yaml")`.
`task.warnings` lists what the rules of the template find before the fit (for example a1-a4 all free with data at one
pressure) and values outside the plausible ranges after it. `task.report(result)` gives the values in the units of the
template, times `display_scale`: a1×10, a2×10⁻², a4×10⁻⁴, c2×10⁻⁴, omega×10⁻⁵, as in SUPCRT and OptimB.

To write your own template, copy `HKF88.yaml` and give `Task` its path as `template`. The constraints are expressions
linear in the parameters, in SI, for example `value: "2037*given.Cp - 30346*cal"`; the names they may use are listed at
the top of `HKF88.yaml`.
