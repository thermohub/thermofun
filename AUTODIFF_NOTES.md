# ThermoFun on autodiff: work notes

Branch: `thermofun_autodiff` (pushed). Issue: thermohub/thermofun#28.

## 1. Refactor: `ThermoScalar` on `autodiff::real`

- `ThermoFun/Common/ThermoScalar.hpp` rewritten. `ThermoScalar` holds **two `autodiff::real`**: one seeded along T (`wrtT()`), one along P (`wrtP()`), plus `err` and `sta` (status/message).
- All derivatives come from autodiff's operators (`+ - * /`, `exp`, `log`, `sqrt`, `pow`). The hand-written chain rules are gone. `ThermoScalar` itself only propagates `err` and `sta`.
- Remaining hand-written derivative code: guards at value 0 (`sqrt`, `log`, `pow` return zero derivative) and `pow(ThermoScalar, ThermoScalar)` with a zero base.
- Accessors: `val()`, `ddt()`, `ddp()`, setters `setVal/setDdt/setDdp`. Call sites changed mechanically (`x.val` -> `x.val()`).
- `Temperature(T)` / `Pressure(P)` seed their own direction as before.
- Python: `val`, `ddt`, `ddp` are properties with the same names, so the Python API and tests are unchanged.
- Cost: each operation evaluates the value once per pass, so about 2x per scalar operation. `autodiff::real` carries a single seeded direction, so one object cannot hold both d/dT and d/dP. Options if this matters: one pass per direction driven from `ThermoEngine`, or a lazy/optional P pass.
- Reaktoro (reaktoro/reaktoro) uses `autodiff::real` the same way (`autodiff::seed(wrtvar)` per pass).

### Build / dependencies
- `cmake/modules/ThermoFunFindDeps.cmake`: `find_package(autodiff 1.1.1)`, fallback `FetchContent` pinned to commit `b0a4fef` (v1.1.2 plus the change making Eigen optional; the v1.1.2 tag itself requires Eigen).
- `ThermoFun/CMakeLists.txt` links `autodiff::autodiff`; `ThermoFunConfig.cmake.in`, `environment.devenv.yml`, `install-dependencies.sh`, README updated.

## 2. Tests added
- `pytests/test_autodiff.py` (26 tests): d/dT and d/dP of G, H, S, Cp, and of reaction G and logK vs central finite differences; `dG/dT = -S`, `dH/dT = Cp`, `T dS/dT = Cp`; Python accessor API; status handling; Quartz `V = dG/dP`.
- `tests/autodiff/` (ctest `autodiff`): `ThermoScalar` operations vs a direct `autodiff::real` evaluation and analytic derivatives, zero guards, status and error propagation.
- Regression guard: ~4,100 substance/reaction evaluations dumped before and after the refactor. No value changed; derivatives matched within 1e-8.
- Final state: 57 pytests and 2 ctests pass.

## 3. Pre-existing derivative bugs found and fixed (they exist in master too)
1. **Akinfiev-Diamond (e.g. CO2@) H and S `ddt` wrong** (`SoluteADgems.cpp`). `Tr = 298.15` was converted to a seeded `Temperature`, so the constant reference-state increments (`Geos298`, `Seos298`, `Heos298`) carried dT derivatives. Error in `dS/dT` was exactly `dSeos298/dT` (0.5806); in `dH/dT` it was `CPeos298` (173.1). `dG/dT` was right only by cancellation with an omitted term. Fix: zero those derivatives and keep the derivative in the `S(T-Tr)` term.
2. **logK `ddt` wrong** (`ThermoEngine.cpp`). `lnK = dGr / -(R*T)` used `T` as a double, dropping `d(1/T)` (missing term = logK/T). Fix: use `Temperature(T)`.
3. **HP Landau volume not equal to `dG/dP`** (`SolidHPLandau.cpp`, ~0.07% for Quartz). With equilibrium order parameter (`dG/dQ = 0`, Eq. 13 of Holland & Powell 1992, Am. Min. 77:53), `V = dG/dP` at fixed Q: `V = v_bis*(1+4P/(1000 kT))^(-1/4) + Vmax*(Q^6/3 - Q^2)`. The paper gives no volume formula; this was derived from the HP98 form used in the code and checked against a finite difference of G (V now equals `dG/dP*1e5` to ~1e-15 relative).
   - Consequence: V, U, A change by up to ~7e-4 relative for 14 Landau minerals in the test databases (Quartz, Albite, Anorthite, Calcite, Dolomite, ...). Quartz reference volume at 873.15 K/5000 bar updated from 2.3183336 to 2.3167016 in `pytests/test_thermoengine.py`. Results no longer match GEMS exactly for these.

## 4. Known, left unchanged
- Quartz V `ddt`/`ddp` = 0 exactly at the reference state (298.15 K, 1 bar): `SolidMurnaghanHP98.cpp` takes a constant-volume branch there (model kink, not an AD error). Tests avoid that point.
- Other places divide by a plain `T` (e.g. `reaction_entropy = (dH-dG)/T`); their derivatives may drop terms. Not audited.

## 5. Commits on `thermofun_autodiff`
1. Base ThermoScalar on autodiff::real (#28)
2. Fix derivatives of Akinfiev-Diamond reference increments and logK
3. Use the exact derivative for the Landau volume
