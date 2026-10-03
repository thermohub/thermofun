# Python interface

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

The calculation models can be used directly with the `Substance` or `Reaction` that holds their parameters:

- substances: `ThermoModelsSubstance`, `EmpiricalCpIntegration`, `EntropyCpIntegration`, `WaterIdealGasWoolley`, `HPLandau`,
  `MinMurnaghanEOSHP98`, `MinBerman88`, `MinBMGottschalk`, `ConMolVol`, `IdealGasLawVol`, `GasCORK`, `GasPRSV`, `GasCGF`,
  `GasSRK`, `GasPR78`, `GasSTP`, `SoluteHKFgems`, `SoluteHKFreaktoro`, `SoluteHollandPowell98`, `SoluteAnderson91`,
  `SoluteAkinfievDiamondEOS`;
- solvent: `WaterHGK`, `WaterHGKreaktoro`, `WaterWP95reaktoro`, `WaterZhangDuan2005` (`propertiesSolvent`,
  `thermoPropertiesSubstance`), `WaterJNreaktoro`, `WaterJNgems`, `WaterElectroSverjensky2014`,
  `WaterElectroFernandez1997` (`electroPropertiesSolvent`);
- reactions: `ReactionDolejsManning10`, `ReactionFrantzMarshall`, `ReactionRyzhenkoBryzgalin`, `Reaction_LogK_fT`,
  `Reaction_Vol_fT`, `ReactionFromReactantsProperties`.

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
