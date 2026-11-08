<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Migrate from the old API

**You need:** code that uses ThermoFun before version 1.0.

Calculations do not change their names: `ThermoEngine`, `ThermoBatch`, `thermoPropertiesSubstance` and the other calls stay as they are.

## Gases: G has new terms by default

**Warning:** from version 1.0, G of a gas with a fluid method or `IdealGas` has `RT ln(P/Pr)` and `RT ln(phi)` by default, on every path. Before 1.0 the default was G(T, Pr). A code that adds these terms itself, such as GEMS, counts them twice if it keeps the defaults, and no error tells it.

The change is large at high pressure. aq17 at 473.15 K and 1 kbar: CH4 +28.9 kJ/mol, CO +29.6, CO2 +26.5, H2 +29.1. `RT ln(P/Pr)` alone is 27.2 kJ/mol.

To keep the old values, call `useStandardStateOfGases()` on the preferences of the engine. It sets `apply_pressure_correction_to_gas_props = False` and `cancel_fugacity_coefficient_in_gas_props = True`.

```python
import thermofun as tf

preferences = tf.EnginePreferences().useStandardStateOfGases()  # G(T, Pr) for a gas, as before 1.0
assert preferences.apply_pressure_correction_to_gas_props is False
assert preferences.cancel_fugacity_coefficient_in_gas_props is True
```

In C++: `engine.preferences().useStandardStateOfGases();`.

## Volumes are in m3/mol

**Warning:** from version 1.0, `volume` and `reaction_volume` are in m3/mol, SI as the pressure in Pa, so dG/dP = V. Before 1.0 they were in J/bar. 1 J/bar is 1e-5 m3/mol, so a value is 100000 times smaller, and a code that converted it with a factor gets wrong numbers without an error. For cm3/mol: multiply by 1e6 (it was 10).

```python
import thermofun as tf

assert tf.volume_unit == "m3/mol"   # absent before 1.0: the volumes were in J/bar
```

In C++, `THERMOFUN_VOLUME_SI` is defined from 1.0 on. `ThermoBatch` gives m3/mol unless you ask for another unit, for example `batch.setPropertiesUnits(["volume"], ["J/bar"])`.

## Other results that change

| Change | What to do |
|---|---|
| P = 0 is the saturation pressure also for minerals and gases (before: 0 Pa). | Nothing; `engine.saturationPressure(T)` gives the pressure in Pa, also in Python. |
| P = 0 at or above 647.096 K, a negative P, T <= 0, and NaN are refused with an error. | Give a pressure above the critical temperature of water: P = 0 means the saturation pressure, which ends there. |
| A substance from a reaction with a pressure method now runs that method on what the reaction gives. | Nothing, unless your record had such a method and it was not used before. |
| On the steam tables, G and A of water now have the term `-Str (T - Ttr)`. | Nothing. The old values were wrong. |
| The `NEA_HGK` and `NEA_IAPWS` data of the triple point have new U and A. | Nothing. Equilibrium calculations do not read U or A. |
| R is 8.31451 J/(mol K) in Churakov-Gottschalk too, and ln(10) has all its digits. | Nothing. The differences are very small. |

## Calls that have a new form

The old calls still work, through `Compat/`. They will go in version 2.0.

| Old call | New form |
|---|---|
| `ThermoFun::v2::...` in C++ | `ThermoFun::model::...`; the old name is an alias until 2.0 |
| An include of a header other than `ThermoFun/ThermoFun.h` | `#include <ThermoFun/ThermoFun.h>`: only the public headers are installed |
| `setMethodGenEoS`, `setMethod_T`, `setMethod_P` | The `methods` list of a record |
| `setThermoParameters`, `thermoParameters` | The `parameters` of a method, by name |
| `setThermoReferenceProperties` | `standard_state_properties` |
| `setReaction` of a substance | `defining_reaction` |
| Keys such as `sm_gibbs_energy`, `TPMethods` | The v2 format: see [The legacy format](../reference/legacy-format.md) |
| `uncertainCoefficients()` | `uncertainty` of a value, and `covariances` |

**Check:** compare your results before and after, and read the changes above if they differ.

```python
import thermofun as tf

substance = tf.Substance()
substance.setSymbol("X")
assert substance.symbol() == "X"                                # an old setter still works
```
