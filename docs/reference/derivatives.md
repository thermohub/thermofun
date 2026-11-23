<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Derivatives

Every property that ThermoFun gives has its derivatives by temperature and by pressure: `ddt` and `ddp` of a [result](results.md). They are exact, not finite differences. The models are written with the library [autodiff](https://autodiff.github.io/), which calculates the derivative together with the value, so there is no step size to choose.

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
calcite = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(calcite.gibbs_energy.ddt, -calcite.entropy.val)   # dG/dT is -S
```

Output:

```text
-135.56702246071714 -135.5670224607171
```

## What the derivatives satisfy

The derivatives agree with the other properties, as thermodynamics says:

| Relation | Meaning |
|---|---|
| dG/dT = -S | the entropy is the derivative of G |
| dG/dP = V | the volume is the derivative of G (V in m3/mol, P in Pa) |
| dH/dT = Cp, T dS/dT = Cp | the heat capacity is the derivative of H and S |
| dS/dP = -dV/dT | the Maxwell relation |

A test checks these for every record of the bundled databases. For water the second derivatives are in the result too (`densityT`, `densityTT`, `densityTP`, `epsilonP`, ...), because the models of aqueous species need them.

## Limits

- **The published model decides.** A few models do not satisfy a relation, because the published formulas do not: the volume of the PRSV and CORK gases is that of the real fluid while G and H carry the fugacity term, some minerals with a Murnaghan volume, and the pressure derivative of H of the Akinfiev H2@. The results are as the publications give them.
- **Cv.** For aqueous ions the relation Cv = Cp - T V alpha^2/beta has no meaning (their partial molar volumes grow with P), so Cv = Cp is kept.
- **At a branch of a model.** A model can have a branch at one point: quartz has a constant volume exactly at the reference state, and CORK has a branch at 5 kbar. The derivatives there are those of the branch.
- **Near the critical point of water** the derivatives of the equation of state are more consistent than finite differences of the values.

## The cost

A calculation with derivatives takes about twice as long as one without: a model runs once for T and once for P. The results are cached by the engine, so a repeated calculation costs nothing.

## For C++ code

`val`, `ddt` and `ddp` of a `ThermoScalar` can be used as `x.val` (ThermoFun) and as `x.val()` (autodiff, Reaktoro). A project that uses ThermoFun with CMake needs `find_package(autodiff)`, which `ThermoFunConfig.cmake` does. ThermoFun is always built with autodiff.

The uncertainty of a result, and its rounding, are in [Uncertainties and rounding](uncertainty.md).
