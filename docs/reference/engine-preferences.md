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
| physical_constants | codata1986 | codata2018, codata1986 | the constants of physics of the calculation: codata1986 (the default: most stored G and log K values were derived with its R) or codata2018 (tf.methods.physicalConstantsNames()). Fitted equations keep the R of their fit. A change drops the cached results |
| propagate_parameter_errors | False |  | also propagate the uncertainties of the parameters of the methods, with those of the reference values and the covariances; without it, only the reference values True/False (default: False) |
| property_source | Record | direct, reaction, reactants, record | where the properties of a record come from: PropertySource or the words record (default), direct, reaction, reactants |
| round_to_uncertainty | False |  | round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (default: False); derivatives are not rounded |
| solventState | 0 |  | not read by the calculations (old API): the state of water comes from its record or from the state argument of propertiesSolvent, default 0 |
| solventSymbol | 'H2O@' |  | symbol of solvent used in EoS of solute properties (e.g., H2O@ for HKF model), default H2O@ |
| uncertainty_significant_digits | 2 |  | number of significant digits of the rounded uncertainties if round_to_uncertainty is True (default: 2): to the nearest, a half up (NEA TDB-3 sect. 4) |

## EngineConventions

A convention says what the zero of an energy is, and what a standard state is. The same substance has different G and H in different conventions. The engine converts each record from the convention of the record to the one asked for here, so the results are in the convention of your choice. They change only the zero and the scale, not the physics. See [Conventions](conventions.md).

| Option | Default | Valid words | Description |
|---|---|---|---|
| apparent_properties | 'BENSON_HELGESON' |  | the 0.x name of energy_reference.elements, in capitals (BENSON_HELGESON) |
| energy_reference.elements | BensonHelgeson | benson_helgeson, berman_brown, ser, assigned_enthalpy, gibbs_helmholtz_formation, dft_formation_0K | benson_helgeson, berman_brown, ser, assigned_enthalpy, gibbs_helmholtz_formation or dft_formation_0K (EnergyReference or the word) |
| energy_reference.ions | None | hydrogen_ion, none | hydrogen_ion (the default: H+ is 0, the electron from the standard hydrogen electrode) or none (the records give them); Ions or the word |
| energy_reference.water | None | helgeson_kirkham_1974, codata_1989, steam_tables | helgeson_kirkham_1974 or codata_1989 (the anchor: data of water used inside), or steam_tables (the scale of the values of a water record); WaterReference or the word |
| standard_state.concentration_scale | Molality | molality, mole_fraction, molarity | the concentration scale of solutes: molality, molarity or mole_fraction (ConcentrationScale or the word) |
| standard_state.pressure | 0 | 1bar, 1atm, any pressure in Pa | the standard pressure p° in Pa; set it with 1bar, 1atm (a word or StandardPressure) or any pressure in Pa |
| water_properties | 'GEOCHEMICAL' |  | the 0.x name of energy_reference.water, in capitals: GEOCHEMICAL (the anchor of the dataset) or STEAM_TABLES |
