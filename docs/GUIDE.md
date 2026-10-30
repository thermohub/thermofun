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

Change one option, for example `engine.preferences.propagate_parameter_errors = True`, or set all of them with
`engine.preferences = prefs` (Python names are the same except `solventState` and `solventSymbol`).
A change applies from the next calculation: the engine then clears its cached results.

| Option | Default | When it matters |
|---|---|---|
| `enable_memoize` | true | Caches results of repeated `(T, P, symbol)` calls. Leave on unless you are debugging a model. |
| `max_cache_size` | 1e6 | Number of cached entries; the least recently used is dropped first. 0 = unlimited. |
| `solvent_state` | 0 | Solvent state used by the solute models: 0 liquid, 1 vapour. |
| `solvent_symbol` | `H2O@` | The solvent record used to calculate solutes (for example HKF). Change it if your database names water differently. |
| `fallback_to_reference_properties` | false | When a substance has no calculation method or data are missing, return the reference properties stored in the record instead of leaving the property undefined. |
| `apply_pressure_correction_to_gas_props` | false | Adds the fugacity/pressure correction of the fluid equation of state to `H`, `S`, `V` of gases (`G` is kept, see the note in [autodiff.md](autodiff.md)). Reactions calculated from reactants switch it on themselves. |
| `cancel_fugacity_coefficient_in_gas_props` | true | Fluids (CORK, PRSV, CG, SRK, PR78, STP): the term `RT ln(phi)` is removed from `G`, so `G` is the standard-state value `G(T, 1 bar)`. Set to false to keep it: `G` is then `G(T, 1 bar) + RT ln(phi)` (up to 1.3 kJ/mol for CO at 1000 bar), and for steam from the HGK water equation of state the ideal-gas pressure term `RT ln(P/1 bar)` is removed from `G` in the same way. The terms are 0 at 1 bar for an ideal gas. |
| `propagate_parameter_errors` | false | Also propagates the uncertainties of model coefficients. Costs two extra calculations per parameter with an error. |
| `round_to_uncertainty` | false | Rounds results to the decimals of their uncertainty (NEA TDB rules). |
| `uncertainty_significant_digits` | 2 | Digits of the rounded-up uncertainty when rounding is on (the TDB tables use one or two). |

**Conventions** (`engine.conventions`): `apparent_properties` (`BENSON_HELGESON` default, or `BERMAN_BROWN`)
converts the properties of substances (not the water solvent) to the Berman-Brown convention if set so; `water_properties` (`GEOCHEMICAL` default,
or `STEAM_TABLES`) converts the properties of the water solvent to the steam-tables reference state if set so.
Change one field in place, or set all of them with `engine.conventions = conventions` or
`engine.setConventions(conventions)`. A change applies from the next calculation.

**Changes of the data** apply from the next calculation too: `appendData`, and `add...`, `set...` or
`substance(...)`, `reaction(...)`, `element(...)` on `engine.database()`. If you keep a reference from
`substance(...)` and change the record after a calculation, call `substance(...)` again before the next one.

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
A reaction with `logk_ft_coeffs` and a named extrapolation method (`logk_1_term_extrap0`..`logk_3_term_extrap`) is calculated from its reference values; the function is used only if no such method is named. If both give a different logK at the reference temperature (more than 0.01), a warning is logged once for the reaction.
A `cp_ft_equation` substance without Cp coefficients uses a constant Cp equal to its `sm_heat_capacity_p`
(zero if that is empty), and a warning is logged once for it.


Keys of the coefficients that the substance models read (each `{"values": [...], "units": [...]}`):

| Key | Used by |
|---|---|
| `m_volume_fpt_coeffs` | Volume equations of Berman (1988) and Dorogokupets et al. (1988): five coefficients (1/K, 1/K², 1/K³, 1/bar, 1/bar²). |
| `eos_birch_murnaghan_coeffs` | Birch-Murnaghan volume (12 coefficients). |
| `eos_churakov_gottschalk_coeffs` | Churakov-Gottschalk fluid (CG) equation of state. |
| `eos_gas_crit_props` | Critical parameters of the other fluid equations of state. |

A record without `aggregate_state` has the state "other". A water solvent record with `standard_entropy_cp_integration` gets its properties from the reference values and the constant Cp. A gas defined by a reaction and without a volume method has the ideal-gas volume `RT/P`.

| You want to | Do |
|---|---|
| List the models you can put in a record | `tf.availableSubstanceTPMethods()`, `tf.availableReactionTPMethods()`. |
| List the properties and units | `tf.availablePropertiesSubstance()`, `tf.availablePropertiesReaction()`. |
| Add or replace records | `Database.addSubstance`/`setSubstance`/`addMapSubstances`/`setMapSubstances`, and the same for reactions and elements. `set...` overwrites with a warning; `setMap...` overwrites silently. |
| Give an uncertainty | Add `"errors": [...]` next to `values` of a reference property or of a coefficient entry. A coefficient entry has at most 1000 values (the models use up to a few tens; the longest in the test databases has 12): a longer one is refused when the record is read. |

### Extra standard property models

These substance models give the standard properties of a substance at a temperature and a pressure. Each model is a `TPMethods` entry.

| Method (code) | Coefficients | What it does |
|---|---|---|
| `mv_eos_tait_hp11` (39) | `eos_tait_hp11_coeffs`: `kappa0'`, `kappa0''` (1/kbar), number of atoms; the expansivity `m_expansivity` and the bulk modulus `m_compressibility` (kbar) of the record | Tait equation of state of Holland and Powell (2011). It gives the volume and the integral of `V dP` from the reference pressure. The entropy, enthalpy and heat capacity follow. Use it after a `cp_ft_equation` entry. G at the reference T and P is the reference G. |
| `cp_ft_equation_uniquac` (44) | `eos_extended_uniquac_coeffs`: `a`, `b`, `c`, `alpha`, `beta`, `Theta` (default 200 K) | Extended UNIQUAC. The heat capacity is `Cp = a + b T + c/(T - Theta)`. G and H follow from the reference G and H of the record. The pressure term `R T (alpha + beta dP) dP` of G gives the volume and the entropy. |
| `cp_nasa_polynomials` (45) | one entry per temperature interval: `limitsTP` and `nasa_polynomial_coeffs` (`a1`..`a7`, `b1`, `b2`) | NASA polynomials. G, H and S use the NASA convention (`G = H - T S`). They do not refer to the reference values of the record. Outside all intervals, the model uses the nearest interval and sets a message. |
| `standard_thermo_interpolation` (46) | `tp_table`: `temperatures` (K), `pressures` (Pa), `gibbs_energy`, `enthalpy`, `volume` (m3/mol), `heat_capacity_cp`, each as a list `value[i + j*nT]` for temperature `i` and pressure `j`; `reference_pressure` (Pa, default 1e5) | Bilinear interpolation in T and P. Outside the table, the model uses the nearest value and sets a message. `G` gets the term `V (P - reference_pressure)`. The entropy is `(H - G)/T`. |

The test `extra-models` compares all four models with the reference values in `tests/extra_models/extra_reference.csv`.

## Autodiff — derivatives

The models calculate with [autodiff](https://autodiff.github.io/) in two passes (temperature seeded, then
pressure seeded), so `ddt` and `ddp` of **every** property are exact, including third-order terms such as the
derivatives of `densityPP`. Cost: about two model evaluations per call. If you do not need the derivatives, the library can be
built without autodiff (below). Churakov–Gottschalk mixture calculations retain finite differences. 

### Building without autodiff

`cmake -DTFUN_USE_AUTODIFF=OFF` builds the models with plain numbers (`Common/Real.hpp`): autodiff is not
needed (not found, not downloaded, not linked), and the calculations are faster (about 1.3 to 1.8 times in a
quick measurement). The default is ON. In the plain build every `ddt` and `ddp` is 0, and the values, errors and
statuses are identical to the autodiff build (compared on 48,000 values in 8 test databases), except for the
properties whose **value** needs a derivative; they are `notdefined`:

| Property | Why |
|---|---|
| `heat_capacity_cv` of substances (and `reaction_heat_capacity_cv` of the reactions that use them) | `Cv = Cp - T V alpha^2/beta` needs `dV/dT` and `dV/dP`. The Cv of the water solvent models is analytical and kept. |
| `densityTP` of the HGK water | It is `d(densityT)/dP`. `densityT`, `densityP`, `densityTT` and `densityPP` are analytical and kept. |

(With `fallback_to_reference_properties` on, an undefined property is replaced by the reference value of the record when it has one, as for any property that cannot be calculated, so `Cv` is then the reference `Cv`.) `thermofun.with_autodiff` is `False` in the Python package of this build, and the derivative tests are skipped.
Projects that use ThermoFun through CMake (`find_package(ThermoFun)`) get the setting from the installed target,
so they build with plain numbers too.

To only avoid the **download** of autodiff in the default build, install it first (`install-dependencies.sh`,
`conda install autodiff -c conda-forge`, or `-Dautodiff_DIR=<path>`).

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

If the calculation fails for a perturbed parameter, the affected `err` is `NaN` and the message of `sta` says the
uncertainty is incomplete. Not covered: correlations (no covariance matrices) and the solvent models. The assignment of uncertainties
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
| `TFUN_USE_AUTODIFF` | ON | OFF builds the models with plain numbers: no autodiff dependency, faster, all derivatives are 0 (see [Building without autodiff](#building-without-autodiff)). |
| `TFUN_BUILD_TESTS` | OFF | Builds the C++ tests (`ctest`): memoization, autodiff, model derivatives, interface, analytical derivatives. |
| `BUILD_SHARED_LIBS`, `BUILD_STATIC_LIBS` | ON | Library type. |

autodiff is found by CMake or downloaded when missing (see the README). Python tests: `python -m pytest` from
the repository root (`pytests/`).

