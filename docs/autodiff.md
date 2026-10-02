# ThermoFun and autodiff

ThermoFun calculates the derivatives of the thermodynamic properties with respect to temperature and pressure with
[autodiff](https://autodiff.github.io/) (`autodiff::real`, header-only), as Reaktoro 2 does (issue
[#28](https://github.com/thermohub/thermofun/issues/28)). This page describes how it is used, what it means for
code that uses ThermoFun, what was fixed on the way, and what is known and left.

## 1. Design

- **Calculation.** The models calculate with `autodiff::real` (`using real = autodiff::real`, `Common/Real.hpp`).
  `autodiff::real` carries one derivative along one seeded direction, so a model is evaluated in two passes
  (`Reaktoro_::Pass`): one with the temperature seeded, one with the pressure seeded. `twoPass(T, P, f)`
  (`ThermoProperties.h`) runs `f(pass)` in both passes and combines the results into properties with the value
  `val` and the derivatives `ddt` and `ddp`.
  - Inputs that come from earlier results are lifted into a pass with `pass(x)` / `lift(pass, props)`.
  - Constants (database reference values, reference-state quantities) are passed with `constant(x)` or a
    `Pass(T, P, Wrt::None)`: a constant must not carry a derivative.
- **Results.** The properties are `Reaktoro_::ThermoScalar` instances (`Common/ThermoScalar.hpp`) with `val`, `ddt`,
  `ddp`, `err` and `sta`, as before (see section 2). The property structs (`ThermoPropertiesSubstance`,
  `ThermoPropertiesReaction`, `PropertiesSolvent`, `ElectroPropertiesSolvent`) are structs of `ThermoScalar`; the
  same structs with `real` (`...AD`) are used inside the models.
- **Errors and statuses** are not part of the calculation: each model sets them explicitly after it, with
  `propagateFrom(inputs...)` (not defined if any input is not defined, otherwise calculated; the error is the
  quadrature sum of the errors of the inputs) or keeps those of its input.
- **Cost.** About twice per model call (two passes).
- **Build.** autodiff is found with `find_package(autodiff)` or fetched with `FetchContent` (commit `b0a4fef`:
  v1.1.2 plus the change that makes Eigen optional). It is listed in `install-dependencies.sh`,
  `environment.devenv.yml` and `ThermoFunConfig.cmake.in`.

## 2. The interface for other codes is not changed

Codes that use ThermoFun are not affected:

- `Common/ThermoScalar.hpp` (`Reaktoro_::ThermoScalar`, `Temperature`, `Pressure` with their arithmetic, `Status`),
  `Common/ScalarTypes.hpp` and `ThermoVariables` are as before; the engine and batch API is unchanged; the Python
  class `ThermoScalar` and its attributes are unchanged.
- The members `val`, `ddt` and `ddp` can be used in both styles: `x.val` (ThermoFun) and `x.val()` (autodiff and
  Reaktoro); `val(x)` also works. They are of the type `Reaktoro_::Num`, which converts to and from `double`
  (`x.val = 3.0`, `x.val += 1`, `double d = x.val`, `double& r = x.val`). Not possible with a wrapper: `&x.val`
  is a `Num*` (not a `double*`), `std::max(x.val, 1.0)` needs `std::max<double>`, and `real r = x.val;` needs
  `real r(x.val);`.
- What did change: the low-level model functions (for example `thermoPropertiesHPLandau`) take `real` instead of
  `Temperature`/`Pressure`, and the public headers include autodiff, so projects that use ThermoFun with CMake need
  `find_package(autodiff)` (done by `ThermoFunConfig.cmake`).

## 3. Models calculated with autodiff

All models, including the GEMS implementations that had zero or incomplete derivatives:
- the HGK/LVS water and the Johnson-Norton dielectric constant of GEMS (`WaterHGK-JNgems`);
- the equations of state of fluids: CORK, PRSV, SRK, PR78, STP and Churakov-Gottschalk (`Gases/s_solmod`).

## 4. Derivative bugs fixed (present before)

1. Akinfiev-Diamond (for example CO2@): the reference-state increments carried temperature derivatives.
2. `ln K = -dGr/(R T)` and `dS = (dH - dG)/T` used `T` as a plain number (the derivative of `1/T` was dropped).
3. The Holland-Powell Landau volume was not `dG/dP` of its own Gibbs energy (about 0.07 % for Quartz). With
   `dG/dQ = 0` (Holland and Powell 1992, Am. Min. 77:53), `V = dG/dP` at fixed Q:
   `V = v_bis (1 + 4P/(1000 kT))^(-1/4) + Vmax (Q^6/3 - Q^2)`. **V, U and A change by up to 7e-4 relative** for the
   Landau minerals; the Quartz reference volume in the tests was updated.
4. Empirical Cp integration with several temperature intervals, standard entropy and Cp integration, Birch-Murnaghan:
   constants (interval bounds, reference temperatures) were treated as temperatures.
5. The pressure of the reaction volume correction was a plain number: `dG/dP` of substances calculated from such
   reactions was 0.
6. HKF (GEMS) g function: the exponent lost its temperature derivative.
7. Zhang-Duan water: the pressure derivatives were scaled twice and the finite-difference points were not tied to `T` and `P`.
8. `ReactionDolejsManning10` called the Frantz-Marshall function (reading past the end of its 5 coefficients). It now
   calls the Dolejs-Manning function. **Behaviour change**; the units of its first coefficient were not checked
   against the paper.
9. Holland-Powell 98 aqueous solute: `T'` (= `T` up to 500 K) was a constant, so `dG/dT` was not the derivative of
   the Gibbs energy below 500 K. `T'` now carries the derivative there. The published `S` and `Cp` were not the
    derivatives of the published `G` (the `ln(rho/rho298)/T'` term of `S` had the opposite sign; `Cp` lacked
    `2 alpha/T'`) and `H` was `G + T S298`. `S`, `V`, `Cp` and `H` are now the exact derivatives of `G`:
    `S = -dG/dT`, `Cp = T dS/dT`, `H = G + T S`. **Behaviour change**: `S`, `Cp`, `H` (and `U`, `A`) of HP98 solutes
    differ from the published formulas (G and V are unchanged).
10. `Reaction_Vol_fT` (reaction volume as a polynomial of T and P, coefficients in 1/K, 1/K^2, 1/K^3, 1/bar, 1/bar^2)
    was called by the engine for the volume pressure methods but returned an empty result (which replaced the
    properties), and its dead implementation mixed Pa and bar. It is implemented and wired:
    `V = Vst (1 + a0 dT + a1 dT^2 + a2 dT^3 + a3 dP + a4 dP^2)`, `dG = int V dP`, `dS = -d(dG)/dT`,
    `dH = dG - T d(dG)/dT`, `dCp = -T d2(dG)/dT2`, `ln K = -G/(RT)`. No database uses it yet (tested in
    `model-derivatives`). **Behaviour change** for reactions that select it (before: empty properties).
11. `ThermoParametersSubstance::isothermal_compresibility` and `isobaric_expansivity` were not initialized (undefined
    values for records without them, for example Boehmite with the Murnaghan model); they are 0 now.
12. `ReactionFromReactantsProperties` returned an empty result; the combination of the reactants (formerly in
    `ThermoEngine`) is now this class, and the engine calls it (results unchanged).

## 5. Errors (uncertainties)

The error `err` of a property is a first-order (linear) propagation of independent standard errors,
`err_y = sqrt(sum (dy/dx_i err_i)^2)`; the status rule (not defined if an input is not defined) is as before.

- **Convention.** The propagation is linear, so the result has the convention of the input: with the uncertainties of the
  NEA TDB (95 % confidence level, "Guidelines for the assignment of uncertainties", TDB-3, Wanner 1999) as `errors`, the
  calculated `err` are at the 95 % level too; with standard errors they are standard errors. It follows the TDB-3 rules
  for propagation of errors (Eqs. 17-22): independent quantities (covariances disregarded), `sigma_X^2 = sum (dX/dY_i
  sigma_Yi)^2`, reaction sums weighted by the stoichiometric coefficients (Example 7: `2(-277.4 +- 4.9) - (-467.3 +- 6.2) = -87.5
  +- 11.6` kJ/mol), `sigma(ln K) = sigma(Delta_r G)/(R T)` and `sigma(log10 K) = sigma(ln K)/ln 10` (tested in
  `pytests/test_errors.py`). The TDB-3 assignment of uncertainties (weighted means, SIT extrapolation) is part of the
  data evaluation and not done here; as TDB-3 notes, data of a reaction measured directly have smaller
  uncertainties than those calculated from the formation data of its species, so use the reaction's own reference
  properties (`thermoPropertiesReaction`) when they are in the database.
- **Always** (analytic): reactions from reactants (`sqrt(sum nu_i^2 err_i^2)`), `ln K = -G/(R T)` (`err_G/(R T)`, `log K` divided by
  ln 10), `U = H - P V`, `A = U - T S`, `S = (H - G)/T`, the reaction volume and pressure corrections, the Gibbs energy of
  substances from reference properties (`G(T) = G298 - S298 (T - Tr) + ...`: `(T - Tr) err_S298`) and gases with the
  pressure correction. Before, these used the unweighted quadrature sum of the errors of the inputs. The errors of the
  reference properties come from the `errors` of the database records.
- **Optional, `preferences.propagate_parameter_errors = True`** (Python, C++ `EnginePreferences`): the uncertainties of the
  reference properties and of the **coefficients of the models** are propagated through the whole calculation. The
  optional `errors` array next to the `values` of a coefficient entry of the record (`eos_hkf_coeffs`,
  `m_heat_capacity_ft_coeffs`, `m_phase_trans_props`, `m_landau_phase_trans_props`, `logk_ft_coeffs`,
  `dr_heat_capacity_ft_coeffs`, `dr_volume_fpt_coeffs`, `dr_marshall_franck_coeffs`, `dr_dolejs_manning10_coeffs`, ...) is read
  with the same units. For every parameter with an error `s` the property is calculated at `p + s` and `p - s` with a copy of
  the database, and `err = sqrt(sum ((y(p+s) - y(p-s))/2)^2)` for every property. The parameters of the reactants of a
  reaction and of the reactions of a substance are perturbed together, so parameters shared by several records
  are correlated correctly. Cost: two extra calculations per parameter that has an error (none without errors).
  Not covered: the solvent models, correlations between different parameters (no covariance matrices), nonlinear
  models are evaluated at the step `s` (not a Taylor expansion).

## 6. Verification

- The previous implementation was used as an oracle: values, derivatives, statuses and messages of about 9,900
  evaluations (substances, reactions, solvents) in 8 databases. Values, statuses and messages are identical;
  derivatives differ only where they were wrong (section 4) or were zero (GEMS water, fluids).
- Derivatives against finite differences, per model family and database (samples of every combination of methods),
  plus the thermodynamic relations (`dG/dT = -S`, `dH/dT = Cp`, `T dS/dT = Cp`, the Maxwell relation `dS/dP = -dV/dT`).
- Tests: `pytests/test_autodiff.py` (derivatives, GEMS water, fluids, reactions, Python accessors) and the ctests
  `autodiff` (infrastructure), `model-derivatives` (models not covered by the databases: Berman, Birch-Murnaghan,
  constant volume, ideal gas volume, fluids, Frantz-Marshall, Dolejs-Manning, Zhang-Duan) and `interface` (the
  interface of section 2, including `val` and `val()`).

## 7. Known and left unchanged

- Quartz `V` has zero `ddt`/`ddp` exactly at the reference state (298.15 K, 1 bar): `SolidMurnaghanHP98.cpp` takes a
  constant-volume branch there. CORK has a branch at 5 kbar.
- Near the critical point the derivatives of the HGK/LVS water are more self-consistent than finite differences of the values.
- The finite difference inside the Churakov-Gottschalk residual enthalpy and entropy (`T + T*DELTA`) is differentiated as written.
- Reactions from reactants: `Cv`, `U` and `A` are those of the last reactant times its coefficient (as before).
- Many substances of `psinagra-12-07-thermofun.json` (for example `Pu(OH)+3`) crash the library (segmentation
  fault), before and after this work; not investigated.
- The errors of properties calculated by models whose uncertainty depends on several parameters (for example HKF or
  logK(T) conversions) are exact only with `propagate_parameter_errors`; without it the analytic rules above apply to the
  derived properties and the reference errors are carried.
