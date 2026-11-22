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
| apparent_properties | 'BENSON_HELGESON' |  | the old name of energy_reference, in capitals (BENSON_HELGESON). It is kept for code that uses it; use energy_reference |
| concentration_scale | Molality | molality, mole_fraction, molarity | the concentration scale of the standard state of aqueous species: molality (default), mole_fraction or molarity |
| energy_reference | BensonHelgeson | benson_helgeson, berman_brown, ser, assigned_enthalpy, gibbs_helmholtz_formation, dft_formation_0K | the reference of the energies: what G and H of a substance are measured from. benson_helgeson (default): the apparent energies of formation from the elements at 298.15 K and 1 bar, as in geochemical databases; berman_brown: G differs from it by 298.15 times the entropies of the elements; ser, assigned_enthalpy, gibbs_helmholtz_formation, dft_formation_0K: see Conventions |
| standard_pressure | OneBar | 1bar, 1atm | the standard pressure of gases: 1bar (default) or 1atm |
| water_properties | 'GEOCHEMICAL' |  | the old name of water_reference, in capitals (GEOCHEMICAL). It is kept for code that uses it; use water_reference |
| water_reference | Geochemical | geochemical, steam_tables | the reference of the energies of water: geochemical (default, G of formation from the elements) or steam_tables (G = 0 and S = 0 for liquid water at its triple point) |
