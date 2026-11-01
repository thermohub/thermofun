<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Migrate from the old API

**You need:** code that uses ThermoFun before version 1.0.

Calculations do not change their names: `ThermoEngine`, `ThermoBatch`, `thermoPropertiesSubstance` and the other calls stay as they are.

## Results that change

| Change | What to do |
|---|---|
| A gas with a fluid method or `IdealGas` now has `RT ln(P/Pr)` in G on every path. | A code that adds this term itself, such as GEMS, sets `apply_pressure_correction_to_gas_props = False` and `cancel_fugacity_coefficient_in_gas_props = True`. |
| A substance from a reaction with a pressure method now runs that method on what the reaction gives. | Nothing, unless your record had such a method and it was not used before. |
| On the steam tables, G and A of water now have the term `-Str (T - Ttr)`. | Nothing. The old values were wrong. |
| The `NEA_HGK` and `NEA_IAPWS` data of the triple point have new U and A. | Nothing. Equilibrium calculations do not read U or A. |
| R is 8.31451 J/(mol K) in Churakov-Gottschalk too, and ln(10) has all its digits. | Nothing. The differences are very small. |

## Calls that have a new form

The old calls still work, through `Compat/`. They will go in version 2.0.

| Old call | New form |
|---|---|
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
