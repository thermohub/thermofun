# Options of the engine

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`.

## EnginePreferences

| Option | Default | Valid words | Description |
|---|---|---|---|
| apply_pressure_correction_to_gas_props | True |  | a gas with a fluid method or IdealGas has RT ln(P/Pr) in G and -R ln(P/Pr) in S True/False (default: True); False gives the state at Pr |
| cancel_fugacity_coefficient_in_gas_props | False |  | remove the fugacity coefficient term RT ln(phi) of the fluids from G True/False (default: False); the residual terms of H and S stay |
| enable_memoize | True |  | enable/disable memoization of calculated properties (default: True) |
| fallback_to_reference_properties | False |  | use reference properties if no calculation functions are available True/False (default: False) |
| max_cache_size | 100000 |  | results kept in all the caches together, default 100000 (about 0.7 kB each); 0: no limit |
| method_implementation | {} |  | method id -> implementation, for example {'HKF88': 'reaktoro'}; it replaces the implementation of the records |
| propagate_parameter_errors | False |  | propagate the errors of the reference properties and of the coefficients of the models to the err of the calculated properties True/False (default: False) |
| property_source | Record | direct, reaction, reactants, record | where the properties of a record come from: PropertySource or the words record (default), direct, reaction, reactants |
| round_to_uncertainty | False |  | round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (default: False); derivatives are not rounded |
| solventState | 0 |  | not read by the calculations (old API): the state of water comes from its record or from the state argument of propertiesSolvent, default 0 |
| solventSymbol | 'H2O@' |  | symbol of solvent used in EoS of solute properties (e.g., H2O@ for HKF model), default H2O@ |
| uncertainty_significant_digits | 2 |  | number of significant digits of the (rounded up) uncertainties if round_to_uncertainty is True (default: 2) |

## EngineConventions

| Option | Default | Valid words | Description |
|---|---|---|---|
| apparent_properties | 'BENSON_HELGESON' |  |  |
| concentration_scale | Molality | molality, mole_fraction, molarity |  |
| energy_reference | BensonHelgeson | benson_helgeson, berman_brown, ser, assigned_enthalpy, gibbs_helmholtz_formation, dft_formation_0K |  |
| standard_pressure | OneBar | 1bar, 1atm |  |
| water_properties | 'GEOCHEMICAL' |  |  |
| water_reference | Geochemical | geochemical, steam_tables |  |
