# ThermoFun usage and options — a short guide

ThermoFun calculates the standard-state thermodynamic properties of substances and reactions, and the
properties of the solvent (water), at a given temperature `T` and pressure `P`, from a database of
records in JSON (a ThermoDataSet). Every result carries its derivatives with respect to `T` and `P`, an
uncertainty and a status. This guide lists what you can call and set, and when each setting matters.
The names are the same in C++ and Python.

## Using ThermoFun

| Class | Use it for |
|---|---|
| `Database` | Loads, inspects and edits the records (`Element`, `Substance`, `Reaction`). |
| `ThermoEngine` | One calculation at one `T`, `P`: properties as `ThermoScalar` values. Units are SI: K, Pa, J/mol. |
| `ThermoBatch` | Many `T`-`P` pairs, substances and properties at once, with units and rounding of your choice, written to a table or a CSV file. Inputs and outputs are in K and Pa unless you change the units (for example `setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])`). |

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")            # a file, or a JSON string
prop = engine.thermoPropertiesSubstance(373.15, 1e8, "H2O@")   # K, Pa, symbol
print(prop.gibbs_energy.val, prop.gibbs_energy.ddt, prop.gibbs_energy.ddp, prop.gibbs_energy.err)

reac = engine.thermoPropertiesReaction(298.15, 1e5, "my-reaction")   # a reaction symbol of your database
print(reac.log_equilibrium_constant.val)

batch = tf.ThermoBatch(engine)                              # uses the preferences of the engine
batch.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
batch.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
batch.thermoPropertiesSubstance([[25, 1], [100, 100]], ["Al+3", "OH-"],
                                ["gibbs_energy", "entropy"]).toCSV("results.csv")
```

C++ is the same (`ThermoFun::ThermoEngine`, `ThermoFun::ThermoBatch`, see the README for examples). Python
raises `RuntimeError` for errors.

## What you get back — `ThermoScalar`

Each property (`gibbs_energy`, `enthalpy`, `entropy`, `volume`, `heat_capacity_cp`, `heat_capacity_cv`,
`internal_energy`, `helmholtz_energy`; for reactions `reaction_gibbs_energy`, ..., `ln_equilibrium_constant`,
`log_equilibrium_constant`) is a `ThermoScalar`:

| Member | Meaning |
|---|---|
| `val` | The value. |
| `ddt`, `ddp` | Derivatives with respect to `T` and to `P`. |
| `err` | Uncertainty, propagated from the errors in the database (see [Errors](#errors-uncertainties)). 0 when none are given. |
| `sta` | Status and message: `notdefined`, `read`, `assigned`, `calculated`, `initialized`. |

**Check `sta`.** A property is `notdefined` when a value it depends on is missing; its `val` is then not
meaningful. `ThermoScalar` supports `+ - * /` and `**` with the derivatives, errors and statuses propagated
(Python and C++), so you can combine properties and keep the derivatives and the uncertainty.

`ThermoEngine` also gives the solvent: `propertiesSolvent` (density, `Alpha`, `Beta`, ... and their
derivatives) and `electroPropertiesSolvent` (dielectric constant `epsilon` and its derivatives, Born functions).

**Pressure `P = 0`** in the engine (C++ and Python) means "at the saturation pressure of the solvent at this `T`".

## Engine preferences — `ThermoEngine.preferences`

Set before calculating, for example `engine.preferences.propagate_parameter_errors = True`
(Python names are the same except `solventState` and `solventSymbol`).

| Option | Default | When it matters |
|---|---|---|
| `enable_memoize` | true | Caches results of repeated `(T, P, symbol)` calls. Leave on unless you are debugging a model. |
| `max_cache_size` | 1e6 | Number of cached entries; the least recently used is dropped first. 0 = unlimited. |
| `solvent_state` | 0 | Solvent state used by the solute models: 0 liquid, 1 vapour. |
| `solvent_symbol` | `H2O@` | The solvent record used to calculate solutes (for example HKF). Change it if your database names water differently. |
| `fallback_to_reference_properties` | false | When a substance has no calculation method or data are missing, return the reference properties stored in the record instead of leaving the property undefined. |
| `apply_pressure_correction_to_gas_props` | false | Adds the fugacity/pressure correction of the fluid equation of state to `H`, `S`, `V` of gases (`G` is kept, see the note in [autodiff.md](autodiff.md)). Reactions calculated from reactants switch it on themselves. |
| `propagate_parameter_errors` | false | Also propagates the uncertainties of model coefficients. Costs two extra calculations per parameter with an error. |
| `round_to_uncertainty` | false | Rounds results to the decimals of their uncertainty (NEA TDB rules). |
| `uncertainty_significant_digits` | 2 | Digits of the rounded-up uncertainty when rounding is on (the TDB tables use one or two). |

**Conventions** (`engine.conventions`): `apparent_properties` (`BENSON_HELGESON` default, or `BERMAN_BROWN`)
converts the properties of substances (not the water solvent) to the Berman-Brown convention if set so; `water_properties` (`GEOCHEMICAL` default,
or `STEAM_TABLES`) converts the properties of the water solvent to the steam-tables reference state if set so.

## Batch calculations — `ThermoBatch`

| Method | What it does |
|---|---|
| `thermoPropertiesSubstance(...)`, `thermoPropertiesReaction(...)` | One `T`/`P`, a list of `T`-`P` pairs, or a list of `T` and a list of `P`; one or many symbols and properties. Returns an `Output`. |
| `setPropertyUnit`, `setPropertyDigit`, `setPropertyUnitDigit`, `setPropertiesUnits`, `setPropertiesDigits`, `setUnits`, `setDigits` | Output unit and number of decimals, per property (`propertyUnits()`, `propertyDigits()` show the defaults). Also applies to the inputs `temperature` and `pressure`. |
| `setTemperatureIncrement(min, max, step)`, `setPressureIncrement(...)` | A grid of `T` and `P` instead of a list. |
| `setSolventSymbol(symbol)` | Same as for the engine. |
| `setBatchPreferences(prefs)` | The CSV and table options below. |

`Output` gives `toCSV(file)`, a table of values, or `toThermoScalar()` for one value with its derivatives.

| `BatchPreferences` | Default | When it matters |
|---|---|---|
| `isFixed`, `isFloating`, `isScientific` | fixed | Number format of the CSV file. |
| `separator` | `,` | CSV separator. |
| `fileName`, `solventFileName` | `tpresults.csv`, `tpSolventResults.csv` | Output files. |
| `outputSolventProperties` | false | Also write the solvent properties. |
| `substancePropertiesFromReaction` | false | For substances defined by a reaction. |
| `reactionPropertiesFromReactants` | false | Calculate the reaction from its reactants instead of its own data. |
| `loopOverTPpairsFirst`, `loopTemperatureThenPressure` | true | Order of the loops in the table. |
| `writeNaNifNotDefinedValue` | true | Write `NaN` for undefined values. If false, the (probably wrong) number is written. |

## The database

A record is a JSON object. A substance has reference properties (`sm_gibbs_energy`, `sm_enthalpy`, `sm_entropy_abs`,
`sm_heat_capacity_p`, `sm_volume`, ...) and `TPMethods` that say how to calculate it at other `T` and `P`
(for example `cp_ft_equation`, `solute_hkf88_gems`, `mv_constant`, fluid equations of state); a reaction has
reactants, its own reference properties and `TPMethods`. A substance with no `TPMethods` is calculated from
the reaction that defines it. Each model reads its coefficients as `{"values": [...], "units": [...]}`.

| You want to | Do |
|---|---|
| List the models you can put in a record | `tf.availableSubstanceTPMethods()`, `tf.availableReactionTPMethods()`. |
| List the properties and units | `tf.availablePropertiesSubstance()`, `tf.availablePropertiesReaction()`. |
| Add or replace records | `Database.addSubstance`/`setSubstance`/`addMapSubstances`/`setMapSubstances`, and the same for reactions and elements. `set...` overwrites with a warning; `setMap...` overwrites silently. |
| Give an uncertainty | Add `"errors": [...]` next to `values` of a reference property or of a coefficient entry. |

## Autodiff — derivatives

The models calculate with [autodiff](https://autodiff.github.io/) in two passes (temperature seeded, then
pressure seeded), so `ddt` and `ddp` of **every** property are exact, including third-order terms such as the
derivatives of `densityPP`. There is no option to turn it off; ignore `ddt`/`ddp` if you do not need them. Cost:
about two model evaluations per call.

Useful as checks: `dG/dT = -S`, `dG/dP = V`, `dH/dT = Cp`. For code that extends ThermoFun
(how to write a model with `real`, what changed, known limits) see [autodiff.md](autodiff.md).

## Errors (uncertainties)

`err` is a first-order propagation of **independent** errors:
`err_y = sqrt(sum (dy/dx_i * err_i)^2)`. It keeps the convention of the inputs: with 95 % NEA TDB
uncertainties in the database, results are at 95 % too.

| Source | How it enters `err` |
|---|---|
| Reference properties (`errors` of `sm_gibbs_energy`, ...) | Always. `G(T) = G298 - S298 (T - Tr) + ...` carries `(T - Tr) * err_S298`. |
| Reactions from reactants | `sqrt(sum nu_i^2 err_i^2)`, weighted by the stoichiometric coefficients. `ln K`: `err_G / (R T)`; `log K`: divided by `ln 10`. |
| Derived properties | `U = H - P V`, `A = U - T S`, ... analytically. |
| Coefficients of the models | Only with `propagate_parameter_errors = True`: each coefficient with an error is varied by `+-` its error and half the difference of the results is added in quadrature. |

Not covered: correlations (no covariance matrices) and the solvent models. The assignment of uncertainties
(weighted means, SIT extrapolation) is part of data evaluation and is not done here. Details and the
NEA TDB-3 worked example: [autodiff.md, section 5](autodiff.md#5-errors-uncertainties).

## Rounding

With `round_to_uncertainty = True` the engine (and so `ThermoBatch`) rounds each result that has `err > 0`:
the uncertainty **up** to `uncertainty_significant_digits` digits, and the value to the same number of
decimals, as in NEA TDB-3 (a 5 followed by nothing rounds to the even digit). Derivatives and results without
an error are not rounded, and the calculation itself is always done in full precision. The same rules are
available as functions:

```python
tf.round_to_uncertainty(25.4478, 1.0834, 2)   # (25.4, 1.1)
tf.round_half_even(2.5, 0)                    # 2.0
```

## Building and testing

| CMake option | Default | |
|---|---|---|
| `TFUN_BUILD_PYTHON` | ON | Builds the Python package `thermofun`. |
| `TFUN_BUILD_TESTS` | OFF | Builds the C++ tests (`ctest`): memoization, autodiff, model derivatives, interface, analytical derivatives. |
| `BUILD_SHARED_LIBS`, `BUILD_STATIC_LIBS` | ON | Library type. |

autodiff is found by CMake or downloaded when missing (see the README). Python tests: `python -m pytest` from
the repository root (`pytests/`).

