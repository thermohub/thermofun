<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Migrate from the old API

**You need:** code that uses ThermoFun before version 1.0.

Calculations do not change their names: `ThermoEngine`, `ThermoBatch`, `thermoPropertiesSubstance` and the other calls stay as they are.

## Gases: G has new terms by default

**Warning:** from version 1.0, G of a gas with a fluid method or `IdealGas` has `RT ln(P/Pr)` and `RT ln(phi)` by default, on every path. Before 1.0 the default was G(T, Pr). A code that adds these terms itself, such as GEMS, counts them twice if it keeps the defaults, and no error tells it.

The change is large at high pressure. aq17 at 473.15 K and 1 kbar: CH4 +28.9 kJ/mol, CO +29.6, CO2 +26.5, H2 +29.1. `RT ln(P/Pr)` alone is 27.2 kJ/mol.

To keep the old values, call `useStandardStateOfGases()` on the preferences of the engine. It sets `apply_pressure_correction_to_gas_props = False` and `cancel_fugacity_coefficient_in_gas_props = True`.

```python
import thermofun as tf

preferences = tf.EnginePreferences().useStandardStateOfGases()  # G(T, Pr) for a gas, as before 1.0
assert preferences.apply_pressure_correction_to_gas_props is False
assert preferences.cancel_fugacity_coefficient_in_gas_props is True
```

In C++: `engine.preferences().useStandardStateOfGases();`.

## Volumes are in m3/mol

**Warning:** from version 1.0, `volume` and `reaction_volume` are in m3/mol, SI as the pressure in Pa, so dG/dP = V. Before 1.0 they were in J/bar. 1 J/bar is 1e-5 m3/mol, so a value is 100000 times smaller, and a code that converted it with a factor gets wrong numbers without an error. For cm3/mol: multiply by 1e6 (it was 10).

```python
import thermofun as tf

assert tf.volume_unit == "m3/mol"   # absent before 1.0: the volumes were in J/bar
```

In C++, `THERMOFUN_VOLUME_SI` is defined from 1.0 on. `ThermoBatch` gives m3/mol unless you ask for another unit, for example `batch.setPropertiesUnits(["volume"], ["J/bar"])`.

## Results that change

Each row is a change of a result for the same data. The [CHANGELOG](../../CHANGELOG.md) gives more detail.

| Change | Size | What to do |
|---|---|---|
| G of a gas has `RT ln(P/Pr)` and `RT ln(phi)` by default. | CH4, CO, CO2, H2: +26 to +30 kJ/mol at 473.15 K and 1 kbar | See [Gases](#gases-g-has-new-terms-by-default). |
| V is in m3/mol. | Values are 100000 times smaller than in J/bar. | See [Volumes](#volumes-are-in-m3mol). |
| log K from log K and dH (or dS) of a reaction: dG comes from log K, and a missing dH or dS is filled at Tr. | log K and dH 10 kJ/mol: log K at 25 °C is 2, not -1.75. PSI/Nagra `PuO2(coll,hyd)`: log K -5.97, not 0. | Nothing. The old values were wrong. |
| `LogKConstantCp` with log K, dH and dCp now changes with T. A legacy polynomial from dCp(T) keeps its dCp terms. | The dCp term of Dolejš (2013), Eq. 36, was missing | Nothing. |
| `LogKVantHoff` with log K only keeps log K constant. | It gave log K = 0. | Add dH or dS. `validate` gives V2-MTH-024. |
| P = 0 is the saturation pressure also for minerals, gases and reactions with a volume term. | NpO2SiO3@ of PSI/Nagra: 293 J/mol at 573 K | Nothing. `engine.saturationPressure(T)` gives the pressure in Pa. |
| P = 0 at or above 647.096 K, a negative P, T <= 0, and NaN are refused with an error. | Before: a value at 0 Pa, or an unclear error | Give a pressure. Above the critical temperature of water, P = 0 has no meaning. |
| The solvent is chosen when the engine is made. | PSI-TDB, AgCl(cr) at 573 K and 1 kbar: -117860 J/mol, also as the first call (it was -116386) | Nothing. The result no longer depends on the order of calls. |
| A substance from a reaction with its own pressure method: the other species are at max(Pr, Psat, P at 420 kg/m3 of water). | Near and above 374 °C | Nothing. The pressure no longer counts twice. |
| H of `AndersonEtAl91` has the term `-T K (alpha(Tr) - alpha(T))`. | H, U and A; G, S and Cp do not change | Nothing. dH/dT is now Cp. |
| S, Cp and H of `SoluteHP98` are the exact derivatives of G. | G and V do not change | To keep the published formulas, use the implementation `published`. |
| Cp of a gas with a fluid method (`PRSV86`, `CorkHP91`, `SoaveRedlichKwong72`, `PengRobinson78`, `SternerPitzer94`) has its residual Cp. | CO2, 373 K, 500 bar: 38 J/(mol K) | To keep the values of GEMS, set the implementation `gems` of the method. |
| H and S of `PRSV86` below the critical temperature of the gas, of `CorkHP91` H2O, and of `ChurakovGottschalk03` are those of G. | H2O (PRSV86), 298 K: H 675 J/mol; H2O (CORK), 373 K, 500 bar: S 53 J/(mol K); CGF: H by RT | The same. G does not change. |
| G of `MurnaghanHP98` and `LandauHP98` minerals integrates V from 1 bar, not from 0, and does not jump at 298.15 K and 1 bar. | G 2-10 J/mol (Albite 10.0 J/mol), and log K of their reactions | The same. |
| U and A of `HKF88` solutes: P V is not multiplied by 41.84. | U and A of aqueous species | Nothing. |
| U, A and Cv of every substance come from the other properties: `U = H - P V`, `A = U - T S`, `Cv = Cp - T V alpha^2/beta`. | Before, many methods gave Cv = 0 or Cv = Cp | Nothing. |
| Cv, U and A of a reaction from its reactants are the sums over all the reactants. | Before, those of the last reactant only | Nothing. |
| `LandauHP98`: V is dG/dP of its own G. | V, U and A up to 7e-4 relative (Quartz) | Nothing. |
| `ChurakovGottschalk03`: Z, U and S are analytical. | About 1e-5 relative (V 1e-5, H 4e-5) | Nothing. |
| `DolejsManning10` and `MarshallFranck81` return log K. `DolejsManning10` uses its own function and `a` in J/mol. | log K was 0; dG and dH were wrong by the value of `a` | Nothing. |
| A legacy value 7777777 (the mark of GEMS for no value) is read as no value. | slop98: G and H of 40 minerals were 7.78 MJ/mol | Add the values, or do not use these records. The migration report lists them. |
| On the steam tables, G and A of water have the term `-Str (T - Ttr)`. | 283 K, 100 bar: G is 166 J/mol (it was -466) | Nothing. |
| `WaterHGK84` with `reaktoro` has its triple point at 273.16 K. | G of water: about 0.63 J/mol | Nothing. |
| The `NEA_HGK` and `NEA_IAPWS` data of the triple point have new U and A. | U and A only | Nothing. Equilibrium calculations do not read U or A. |
| A cached result no longer depends on the order of calls. | H2(g), 273.15 K, 100 bar: G is 3235.96 J/mol (after Fe4(OH)8Clwn(s) it was 5484.36) | Nothing. |
| The `a9` term of `CpPolynomial` has its factor `sqrt(T)`. | Only `BaI2(g)` of heracles | Nothing. |
| `WaterZhangDuan05` stops at 2000 K, the limit of the paper. | Before: 2273.15 K | Use another water model above 2000 K. |
| R is 8.31451 J/(mol K) in Churakov-Gottschalk too, and ln(10) has all its digits. | Below 1e-6 relative | Nothing. |

## Removed

| Removed | Use instead |
|---|---|
| The model classes, for example `WaterHGK`, `SoluteHKFgems`, `EmpiricalCpIntegration`, `GasPRSV`, `Reaction_LogK_fT`, and their headers `ThermoModelsSubstance.h`, `ThermoModelsSolvent.h`, `ThermoModelsReaction.h`, `ElectroModelsSolvent.h` | A method alone: `tf.methods.substanceMethod("HKF88")`, `solventMethod("WaterHGK84")`, `dielectricMethod("JohnsonNorton91")`, `reactionMethod("LogKPolynomial")`. See [Use a method alone](use-a-method-alone.md). |
| `parseSubstance`, `parseReaction`, `parseElement` in C++, and `Common/ParseJsonToData.h` | The constructors `Substance(json)`, `Reaction(json)`, `Element(json)`. The Python functions stay. |
| `ThermoFun/OptimizationUtils.h` | Nothing: the engine keeps its results itself. Limit them with `max_cache_size`. |
| The CMake option `TFUN_USE_AUTODIFF` | Nothing: autodiff is always used. |
| The headers that are not public (99 of 144) | `#include <ThermoFun/ThermoFun.h>` |

## Calls that have a new form

The old calls still work, through `Compat/`. They will go in version 2.0.

| Old call | New form |
|---|---|
| `ThermoFun::v2::...` in C++ | `ThermoFun::model::...`; the old name is an alias until 2.0 |
| An include of a header other than `ThermoFun/ThermoFun.h` | `#include <ThermoFun/ThermoFun.h>`: only the public headers are installed |
| `setMethodGenEoS`, `setMethod_T`, `setMethod_P` | The `methods` list of a record |
| `setThermoParameters`, `thermoParameters` | The `parameters` of a method, by name |
| `setThermoReferenceProperties` | `standard_state_properties` |
| `setReaction` of a substance | `defining_reaction` |
| Keys such as `sm_gibbs_energy`, `TPMethods` | The v2 format: see [The legacy format](../reference/legacy-format.md) |
| `uncertainCoefficients()` | `uncertainty` of a value, and `covariances` |

**Check:** compare your results before and after, and read the changes above if they differ.

```python
import thermofun as tf

substance = tf.Substance()
substance.setSymbol("X")
assert substance.symbol() == "X"                                # an old setter still works
```
