# Options of the engine

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`.

## EnginePreferences

| Option | Default | Valid words | Description |
|---|---|---|---|
| apply_pressure_correction_to_gas_props | False |  | apply pressure/fugacity correction to gas standard properties True/False (default: False) |
| cancel_fugacity_coefficient_in_gas_props | True |  | remove the fugacity coefficient term RT ln(phi) of the fluids from G, giving standard-state G(T, 1 bar) True/False (default: True); False keeps it and removes RT ln(P/1 bar) from G of steam (HGK) |
| enable_memoize | True |  | enable/disable memoization of calculated properties (default: True) |
| fallback_to_reference_properties | False |  | use reference properties if no calculation functions are available True/False (default: False) |
| max_cache_size | 1000000 |  | size of memoization cache, default 1e6 (if 0 unlimited cache) |
| method_implementation | {} |  | method id -> implementation, for example {'HKF88': 'reaktoro'}; it replaces the implementation of the records |
| propagate_parameter_errors | False |  | propagate the errors of the reference properties and of the coefficients of the models to the err of the calculated properties True/False (default: False) |
| property_source | Record | direct, reaction, reactants, record | where the properties of a record come from: PropertySource or the words record (default), direct, reaction, reactants |
| round_to_uncertainty | False |  | round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (default: False); derivatives are not rounded |
| solventState | 0 |  | solvent aggregation state (0 liquid, 1 gas/vapour), default 0 |
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
