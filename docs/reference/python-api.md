<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Python API

The Python package (`import thermofun as tf`, built with pybind11 from `python/src/thermofun`) exposes the public
C++ API of ThermoFun. This page lists what is available; the units are those of the C++ API (K, Pa, J/mol).

## Data and calculation

| Python | C++ | Notes |
|---|---|---|
| `Database`, `Element`, `Substance`, `Reaction` | same | records, `getX`, `addX`, `setX`, `addMapX`, `setMapX`, `containsX`, JSON strings |
| `ThermoEngine` | same | `thermoPropertiesSubstance`, `thermoPropertiesReaction`, `thermoPropertiesReactionFromReactants`, `propertiesSolvent`, `electroPropertiesSolvent` (by symbol or by object), `preferences`, `setPreferences`, `conventions`, `database()` |
| `ThermoBatch`, `Output`, `BatchPreferences` | same | batch calculations, `setUnits`, `setDigits`, `setPropertyUnit`, ... |
| `ThermoPropertiesSubstance`, `ThermoPropertiesReaction`, `PropertiesSolvent`, `ElectroPropertiesSolvent` | same | members are `ThermoScalar` |
| `ThermoParametersSubstance`, `ThermoParametersReaction` | same | parameters of the models, `coefficient_errors` |
| `ThermoScalar`, `Temperature`, `Pressure`, `ThermoVariables`, `Status` | `Reaktoro_::...` | `val`, `ddt`, `ddp`, `err`, `sta`; `+ - * / **` with the propagation of the derivatives, errors and statuses |

## Enumerations and constants (`GlobalVariables.h`)

`MethodGenEoS_Thrift`, `MethodCorrT_Thrift`, `MethodCorrP_Thrift`, `SubstanceClass`, `AggregateState`,
`SubstanceThermoCalculationType`, `RectionThermoCalculationType`, `ReactionComponentType`, `ConcentrationScales`,
`SubstanceTPMethodType`, `ReactionTPMethodType` (needed by the setters and getters of `Substance` and `Reaction`, for example
`s.setSubstanceClass(tf.SubstanceClass.AQSOLUTE)`), the dictionaries `enum_method_substance` and `enum_method_reaction`, and the
constants `R_CONSTANT`, `NA_CONSTANT`, `F_CONSTANT`, `e_CONSTANT`, `k_CONSTANT`, `cal_to_J`, `C_to_K`, `lg_to_ln`, `ln_to_lg`,
`H2O_mol_to_kg`, `bar_to_Pa`, `H2OMolarMass`. The bindings are generated from the header with
`python3 python/tools/generate_enums.py` (run it again if an enumeration changes).

## Models

A method can be used alone, with no database and no engine. The module `thermofun.methods` makes it from its id and its parameters in SI:

- `substanceMethod(id, parameters=..., symbol=...)` gives a method with `evaluate(T, P, reference, solvent)`;
- `solventMethod(id, implementation=...)` gives an equation of state of water with `properties(T, P, state)`;
- `dielectricMethod(id, implementation=...)` gives a dielectric model with `properties(T, P, solvent, state)`;
- `reactionMethod(id, ...)` gives a method of a reaction with `calculate(T, P, incoming, solvent)`;
- `catalog()` lists all methods. The pages in [Methods](methods/index.md) describe them.

The pressure of the solvent models is an input (the saturation line, `P = 0`, is available through `ThermoEngine`).

## Utilities

`convert_units`, `units_convertible`, `parseElement`, `parseSubstance`, `parseReaction`, `readConventions`,
`availableSubstanceTPMethods`, `availableReactionTPMethods`, `availablePropertiesSubstance`, `availablePropertiesReaction`,
`get_water_triple_data`, `update_loggers`, `clear_loggers`, `round_to_uncertainty`, `round_half_even`.

## Not exposed

Internal or private members only: the autodiff helper types (`real`, `Pass`, `ElectroPropertiesSubstance`, `FunctionG`),
the private members of `ThermoBatch` and `Reaction` (reached through `Output` and the engine), `Element.toElementKey` (needs the
types of the ChemicalFun library), the CSV writers of `Common/OutputToCSV.h` (use `ThermoBatch` and `Output`) and the
OpenMP helpers. Errors are raised as `RuntimeError`.

Tests: `pytests/test_python_api.py`, `pytests/test_errors.py`, `pytests/test_autodiff.py`.
