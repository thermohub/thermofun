# 06 — Method registry, parameters, binding and executor

This replaces the three method enums per record and the `switch` blocks in `ThermoEngine.cpp` by a table of
method specifications (data), a registry of factories (code), a binding step (once per record) and executors
(per T, P call). **Phase 2 reproduces today's evaluation order exactly**; more general combinations of methods
are a later, optional extension (§8).

## 1. `Methods/MethodSpec.h`

```cpp
#pragma once
#include <optional>
#include <string>
#include <vector>

namespace ThermoFun {

/// Where a method sits in the evaluation of a record. The first four are today's slots
/// (MethodGenEoS_Thrift, MethodCorrT_Thrift, MethodCorrP_Thrift); see §3.
enum class Slot { General, TCorrection, PCorrection, None };

/// What a method computes.
enum class MethodRole {
    SubstanceProperties,   ///< G, H, S, Cp, V of a substance
    SolventProperties,     ///< water EoS: substance properties + PropertiesSolvent (density, alpha, beta, ...)
    SolventElectro,        ///< water dielectric model: ElectroPropertiesSolvent
    ReactionProperties     ///< logK, dG, dH, dS, dCp, dV of a reaction
};

/// One named parameter of a method.
struct ParameterSpec
{
    std::string name;            ///< "a1"
    std::string si_unit;         ///< UCUM, "J/(mol.Pa)"; "1" for dimensionless
    std::string model_unit;      ///< unit the model class expects in its vector, legacy syntax, "cal/(mol*bar)"
    std::string description;     ///< one line, for the generated reference
    bool required = true;
    std::optional<double> default_si;
    std::string legacy_key;      ///< legacy coefficient key, "eos_hkf_coeffs"
    int legacy_index = -1;       ///< position in the legacy array
};

/// Legacy names of a method, used by the legacy reader/writer and by the old API.
struct LegacyAlias
{
    int tp_method_id = -1;           ///< enum_method_substance / enum_method_reaction key
    std::string tp_method_name;      ///< "solute_hkf88_gems"
    int thrift_code = 0;             ///< value of the MethodGenEoS/CorrT/CorrP enum, e.g. 101 (CTPM_HKF)
};

/// Everything ThermoFun knows about one built-in method, without its code (data only).
struct MethodSpec
{
    std::string id;                      ///< "HKFGems" (D22)
    std::vector<std::string> aliases;    ///< other accepted names, incl. legacy names
    std::string title;                   ///< "Revised HKF equation of state (GEMS implementation)"
    std::string citation;                ///< "1981HEL/KIR" (DataSource symbol) or free text
    int tier = 2;                        ///< D4
    std::string operates_on = "substance";           ///< D23
    std::vector<std::string> applies_to_classes;     ///< §03 §2.2 values; empty = any
    std::vector<std::string> contributes_to;         ///< "G","H","S","Cp","V"
    MethodRole role = MethodRole::SubstanceProperties;
    Slot slot = Slot::General;
    bool needs_solvent = false;          ///< evaluation needs the solvent's properties
    bool piecewise = false;              ///< parameters come as `segments`
    bool implemented = true;             ///< false: known name, no implementation (see §5 tables)
    std::vector<ParameterSpec> parameters;
    std::vector<LegacyAlias> legacy;
};

} // namespace ThermoFun
```

## 2. `Methods/MethodCatalog.h` and `Methods/MethodRegistry.h`

```cpp
/// The built-in method specifications (§5). Pure data, no model code; IO uses it.
class MethodCatalog
{
public:
    static auto instance() -> const MethodCatalog&;
    auto specs() const -> const std::vector<MethodSpec>&;
    auto find(const std::string& id_or_alias) const -> const MethodSpec*;      ///< nullptr if unknown
    auto canonicalId(const std::string& id_or_alias) const -> std::string;     ///< "" if unknown
    auto fromLegacySubstanceId(int tp_method_id) const -> const MethodSpec*;
    auto fromLegacyReactionId(int tp_method_id) const -> const MethodSpec*;
    auto fromThrift(int thrift_code, const std::string& operates_on) const -> const MethodSpec*;
    auto suggest(const std::string& unknown) const -> std::string;   ///< closest id, for error messages
};

/// A bound method ready to run: pre-converted parameters and the function that evaluates it.
struct BoundMethod;   // §6

using SubstanceMethodFactory = std::function<BoundMethod(const MethodApplication&, const Substance&)>;

/// Maps method ids to their implementation. Built-in methods are registered at first use;
/// users can register more (C++ only) before creating an engine.
class MethodRegistry
{
public:
    static auto instance() -> MethodRegistry&;
    auto add(const MethodSpec& spec, SubstanceMethodFactory factory) -> void;   ///< throws if id exists
    auto has(const std::string& id) const -> bool;
    auto spec(const std::string& id) const -> const MethodSpec&;
    auto bind(const MethodApplication& app, const Substance& s) const -> BoundMethod;
    auto ids() const -> std::vector<std::string>;
private:
    MethodRegistry();
    std::map<std::string, std::pair<MethodSpec, SubstanceMethodFactory>> entries_;
};
```

(The same for reactions with `ReactionMethodFactory`.) The catalog table is written once in
`MethodCatalog.cpp` as a `static const std::vector<MethodSpec>`; registration of built-in factories in
`BuiltinSubstanceMethods.cpp` etc. looks the spec up by id, so specs are never duplicated.

## 3. Slots and evaluation order (today's semantics, made explicit)

Each legacy method code belongs to one slot by its number range: `1xx` → `General` (MethodGenEoS), `2xx` →
`TCorrection` (MethodCorrT), `3xx` → `PCorrection` (MethodCorrP). A record has **at most one method per slot**
today; v2 records with two methods in one slot are refused in phase 2 (`V2-MTH-007`) until §8 is done.

The executor (§7) evaluates `General`, then `TCorrection`, then `PCorrection`, with the special cases of §7.
Water methods are special: for the water solvent/vapor record, the **EoS** is in the `TCorrection` slot
(CTM_WAT/WAR/WWP/WZD) and the **dielectric model** in the `General` slot (CTPM_WJNR/WJNG/WSV14/WF97). In the
spec these have roles `SolventProperties` and `SolventElectro`; the executor picks them by role.

## 4. Parameter conventions

- Parameters are **named as in the source paper**; the SI unit is declared; the `model_unit` is the unit the
  existing model class expects (today's `units_to` lists in `ParseJsonToData.cpp`). Binding converts SI →
  `model_unit` once.
- Legacy arrays map position → name with `legacy_index`.
- Piecewise methods (`CpIntegration`) declare the parameters of **one segment**; segments repeat them.

### 4.1 Phase transitions

`CpIntegration` transitions rows `{T, dS, dH, dV, dTdP}` (units K, J/(mol·K), J/mol, m³/mol, K/Pa in SI; model
units K, J/(mol·K), J/mol, J/bar, K/bar) are packed into `phase_transition_prop` in order.

## 5. The catalog — every built-in method

Columns: legacy id/name → `Slot`/Thrift code → **v2 id** → parameters (index: name [model unit]) → notes.
"impl" = implemented in today's engine (has a `case`); methods marked **no** are known names whose legacy use is
silently ignored today (kept that way for legacy files, with warning `LEG-MTH-002`; error `V2-MTH-008` in v2).

### 5.1 Substances

| Legacy id / name | Slot / code | v2 id | Parameters | impl |
|---|---|---|---|---|
| — (API only) | General / 99 CTPM_CON | `ConstantProperties` | — | yes |
| 0 `cp_ft_equation` | General / 100 CTPM_CPT | `CpIntegration` (piecewise) | `m_heat_capacity_ft_coeffs`: 0 a0 [J/(mol*K)], 1 a1 [J/(mol*K^2)], 2 a2 [(J*K)/mol], 3 a3 [J/(mol*K^0.5)], 4 a4 [J/(mol*K^3)], 5 a5 [J/(mol*K^4)], 6 a6 [J/(mol*K^5)], 7 a7 [(J*K^2)/mol], 8 a8 [J/mol], 9 a9 [J/(mol*K^1.5)], 10 a10 [J/(mol*K)] (unused by the formula). Cp = a0 + a1·T + a2/T² + a3/√T + a4·T² + a5·T³ + a6·T⁴ + a7/T³ + a8/T + a9·√T. Segment range from `limitsTP`. Transitions §4.1 | yes |
| 1 `cp_ft_equation_saxena86` | T / 203 CTM_FEI | `CpSaxena86` | (not defined) | **no** |
| 43 `standard_entropy_cp_integration` | T / 200 CTM_CST | `EntropyCpIntegration` | — (uses S°, Cp° reference) | yes |
| 2 `solute_hkf88_gems` | General / 101 CTPM_HKF | `HKFGems` | `eos_hkf_coeffs`: 0 a1 [cal/(mol*bar)], 1 a2 [cal/mol], 2 a3 [(cal*K)/(mol*bar)], 3 a4 [(cal*K)/mol], 4 c1 [cal/(mol*K)], 5 c2 [(cal*K)/mol], 6 omega [cal/mol]. SI: J/(mol.Pa), J/mol, J.K/(mol.Pa), J.K/mol, J/(mol.K), J.K/mol, J/mol. needs_solvent | yes |
| 3 `solute_hkf88_reaktoro` | General / 119 CTPM_HKFR | `HKFReaktoro` | as HKFGems | yes |
| 4 `solute_aknifiev_diamond03` | P / 320 CPM_AKI | `AkinfievDiamond03` | `eos_akinfiev_diamond_coeffs`: 0 xi [1], 1 a [(cm^3)/g], 2 b [(cm^3*K^0.5)/g]. needs_solvent (+ ideal-gas water) | yes |
| 5 `landau_holland_powell98` | T / 201 CTM_CHP | `HP98Landau` | `m_landau_phase_trans_props`: 0 Tc0 [degC], 1 Smax [J/(mol*K)], 2 Vmax [J/bar]; record-level k0 [kbar], a0 [1/K] (§05 note) | yes |
| 6 `landau_berman88` | T / 202 CTM_BER | `Berman88Landau` | (not defined) | **no** |
| 8 `fug_critical_param` | P / 311 CPM_GAS | `GasCritical` | `eos_gas_crit_props` as below. Only used by the rule "HKF + CPM_GAS → H2O vapor" (§7) | **no** (flag only) |
| 9 `fluid_prsv` | P / 312 CPM_PRSV | `PRSV` | `eos_gas_crit_props`: 0 Tc [K], 1 Pc [bar], 2 omega [1], 3 k1 [1], 4 k2 [1], 5 k3 [1], 6 c6 [1] (index 6: name from `s_solmod2_.cpp` `Eosparm` use, S2.3) | yes |
| 10 `fluid_churakov_gottschalk` | P / 313 CPM_EMP | `CGF` | `eos_churakov_gottschalk_coeffs` (empirical, model reads the array as is) else `eos_gas_crit_props`; names c0…cN until checked in `GasCGF.cpp`/`CGFpure.hpp` | yes |
| 11 `fluid_soave_redlich_kwong` | P / 314 CPM_SRK | `SRK` | as PRSV | yes |
| 12 `fluid_sterner_pitzer` | P / 315 CPM_STP | `SternerPitzer` | as PRSV | yes |
| 13 `fluid_peng_robinson78` | P / 316 CPM_PR78 | `PR78` | as PRSV | yes |
| 14 `fluid_comp_redlich_kwong_hp91` | P / 317 CPM_CORK | `CORK` | as PRSV | yes |
| 25 `water_diel_jnort91_reaktoro` | General / 117 CTPM_WJNR | `JN91Reaktoro` | — (constants in code, phase 5) role SolventElectro | yes |
| 26 `water_diel_jnort91_gems` | General / 118 CTPM_WJNG | `JN91Gems` | — role SolventElectro | yes |
| 27 `water_diel_sverj14` | General / 120 CTPM_WSV14 | `Sverjensky14Diel` | — role SolventElectro | yes |
| 28 `water_diel_fern97` | General / 121 CTPM_WF97 | `Fernandez97Diel` | — role SolventElectro | yes |
| 29 `water_eos_hgk84_lvs83_gems` | T / 206 CTM_WAT | `WaterHGKGems` | — role SolventProperties | yes |
| 30 `water_eos_iapws95_gems` | T / 207 CTM_WAS | `WaterIAPWS95Gems` | — | **no** |
| 31 `water_eos_hgk84_reaktoro` | T / 221 CTM_WAR | `WaterHGK` | — role SolventProperties | yes |
| 32 `water_eos_iapws95_reaktoro` | T / 222 CTM_WWP | `WaterWP95` | — role SolventProperties | yes |
| 33 `water_pvt_zhang_duan05` | T / 223 CTM_WZD | `WaterZhangDuan05` | — role SolventProperties | yes |
| 34 `mv_constant` | P / 305 CPM_CON | `ConMolVol` | — (uses V°) | yes |
| 35 `mv_equation_dorogokupets88` | P / 306 CPM_VKE | `Dorogokupets88Vol` | `m_volume_fpt_coeffs`: 0 a1 [1/K], 1 a2 [1/K^2], 2 a3 [1/K^3], 3 b1 [1/bar], 4 b2 [1/bar^2] (same class as Berman88Vol) | yes |
| 36 `mv_equation_berman88` | P / 307 CPM_VBE | `Berman88Vol` | as above | yes |
| 37 `mv_eos_birch_murnaghan_gott97` | P / 308 CPM_VBM | `BirchMurnaghanGott97` | `eos_birch_murnaghan_coeffs` (no units in legacy, passed as is): 0 a1, 1 a2, 2 a3, 3 a4, 4 a5, 5 kt00, 6 dkdt, 7 kp — units to be determined from `SolidBMGottschalk.cpp` (declare `model_unit` = `si_unit` = "1" until then, flagged) | yes |
| 38 `mv_eos_murnaghan_hp98` | P / 309 CPM_CEH | `HP98Min` | record-level k0 [kbar], a0 [1/K] (§05 note) | yes |
| 39 `mv_eos_tait_hp11` | P / 310 CPM_VTA | `HP11Tait` | (not defined) | **no** |
| 40 `mv_pvnrt` | P / 300 CPM_OFF | `IdealGasVol` | — | yes |
| 41 `solute_holland_powell98` | General / 122 CTPM_HP98 | `HP98Sol` | `solute_holland_powell98_coeff`: 0 b [kJ/(mol*K^2)]. needs_solvent. **Legacy files: ignored today** (no `case` in `setTPMethods_old`); reachable only by `setMethodGenEoS` | yes (API) |
| 42 `solute_anderson91` | General / 123 CTPM_AN91 | `Anderson91` | —. needs_solvent. Legacy files: ignored today, as 41 | yes (API) |

### 5.2 Reactions

| Legacy id / name | Slot / code | v2 id | Parameters | impl |
|---|---|---|---|---|
| — | General / 99 CTPM_CON | `ConstantReactionProperties` | — | yes |
| 0 `logk_fpt_function` | General / 102 CTPM_REA | `LogKAnalytic` | `logk_ft_coeffs`: 0 a0 [1], 1 a1 [1/K], 2 a2 [K], 3 a3 [1], 4 a4 [K^2], 5 a5 [1/K^2], 6 a6 [K^0.5]: log K = a0 + a1·T + a2/T + a3·ln T + a4/T² + a5·T² + a6/√T. If no T method is set, the T slot defaults to CTM_LGK (§7.3) | yes |
| 3 `logk_nordstrom_munoz88` | T / 208 CTM_LGK | `LogKNordstromMunoz88` | uses `logk_ft_coeffs` | yes |
| 4 `logk_1_term_extrap0` | T / 209 CTM_EK0 | `LogKExtrap1T0` | — (reference values) | yes |
| 5 `logk_1_term_extrap1` | T / 210 CTM_EK1 | `LogKExtrap1T1` | — | yes |
| 6 `logk_2_term_extrap` | T / 211 CTM_EK2 | `LogKExtrap2T` | — | yes |
| 7 `logk_3_term_extrap` | T / 212 CTM_EK3 | `LogKExtrap3T` | — | yes |
| 8 `logk_lagrange_interp` | T / 213 CTM_IKZ | `LogKLagrange` | — | **no** (empty case) |
| 9 `logk_marshall_frank78` | T / 214 CTM_DKR | `FrantzMarshall` | `dr_marshall_franck_coeffs`: 0 a [1], 1 b [K], 2 c [K^2], 3 d [K^3], 4 e [1], 5 f [K], 6 g [K^2]. needs_solvent | yes |
| 10 `solute_eos_ryzhenko_gems` | T / 216 CTM_MRB | `RyzhenkoBryzgalin` | `dr_ryzhenko_coeffs`: 0 pK298 [1], 1 A [1], 2 B [1]. needs_solvent. Returns directly (no P correction), as today | yes |
| 11 `dr_heat_capacity_ft` | General / 100 CTPM_CPT | `DrCpIntegration` | `dr_heat_capacity_ft_coeffs`: 0 a [J/(mol*K)], 1 b [J/(mol*K^2)], 2 c [(J*K)/mol], 3 d [J/(mol*K^0.5)], 4 e [J/(mol*K^3)] | as today (see note) |
| 12 `dr_volume_fpt` | P / 306 CPM_VKE | `DrVolumeFT` | `dr_volume_fpt_coeffs`: 0 a1 [1/K], 1 a2 [1/K^2], 2 a3 [1/K^3], 3 b1 [1/bar], 4 b2 [1/bar^2] | yes |
| 13 `dr_volume_constant` | P / 305 CPM_CON | `DrVolumeConst` | — | yes |
| 14 `logk_dolejs_manning10` | T / 224 CTM_DMD | `DolejsManning10` | `dr_dolejs_manning10_coeffs`: 0 a [kJ/mol], 1 b [J/(mol*K)], 2 c [J/(mol*K)], 3 d [J/(mol*K^2)], 4 e [J/(mol*K)]. needs_solvent | yes |

Note on `DrCpIntegration`: today a reaction whose General slot is CTPM_CPT is not handled by the General `switch`
(only CON and REA are); its coefficients are used by `Reaction_LogK_fT` for the T-slot methods. Keep exactly this
(the golden suite has 54 such records).

Parameter names marked "to be determined" are filled in step S2.3 by reading the model source; the step lists
what to read. Names follow the paper; when unsure, the name used in the code comment wins.

## 6. Binding (`Methods/Binding.{h,cpp}`)

```cpp
/// A record's methods, bound once: ready-to-run inputs for the model classes.
struct BoundSubstance
{
    const MethodSpec* general = nullptr;      ///< one per slot (phase 2); nullptr = none
    const MethodSpec* t_correction = nullptr;
    const MethodSpec* p_correction = nullptr;
    const MethodSpec* solvent_eos = nullptr;      ///< for water: the SolventProperties method
    const MethodSpec* solvent_electro = nullptr;  ///< for water: the SolventElectro method
    Substance legacy_substance;               ///< a Substance whose legacy fields are packed from the record
};

auto bindSubstance(const Substance& s) -> BoundSubstance;
auto bindReaction(const Reaction& r) -> BoundReaction;
```

**Phase-2 strategy: the existing model classes take a `Substance` and read its legacy fields.** So binding packs
the record into a `Substance` whose legacy fields are exactly what the old parser produced (the
`applyLegacyView` of §05 §5 already does this for legacy records; for v2 records `packLegacy` does the same from
named SI parameters: for each parameter, `vector[legacy_index] = convert(value_SI, si_unit, model_unit)`, missing
optional ones = 0 or the default). The executor then calls the existing classes with that `Substance`. This keeps
the maths untouched and makes v2 records run through the same code as legacy ones.

Cache: `ThermoEngine::Impl` keeps `std::unordered_map<std::string, BoundSubstance>` keyed by symbol, cleared when
`Database::revision()` changes; guarded by the existing memoization mutex pattern.

## 7. Executors — the exact algorithm

`Engine/SubstanceExecutor.cpp` moves `ThermoEngine::Impl::thermoPropertiesSubstance`,
`getThermoPreferencesSubstance`, `reacDCthermoProperties`, `fallbackThermoPropertiesSubstance`,
`propertiesSolvent`, `electroPropertiesSolvent`, `thermoPropertiesReaction`, `thermoPropertiesReactionFromReactants`
there **without changing their logic**, replacing `switch (pref.method_genEOS)` by a dispatch on the bound spec.
The rules below are today's code (ThermoEngine.cpp lines ~234–1000 at v0.7.3); each becomes a named function with
a unit test.

### 7.1 Classification of a substance (`classify`, from `getThermoPreferencesSubstance`)

1. `isHydrogen` = symbol is exactly `"H+"` → all properties 0, return. **[keep; note: depends on the symbol]**
2. `isH2Ovapor` = General is `HKFGems`(101) and (P is `GasCritical`(311) or (T is a water EoS (206/221/222/223) and
   class ≠ aqueous_solute)).
3. `isH2OSolvent` = class == water_solvent.
4. solvent state = 1 (vapor) if aggregate_state == gas else 0.
5. `isReacDC` = thermoCalculationType == REACDC, or no method in any slot, or only P = `ConMolVol`.
6. aqueous solute: if the engine's solvent symbol is not in the database, the solvent is the **last** substance
   (map order) with class water_solvent. **[keep; v2 adds `thermodataset.solvent`]**

### 7.2 Substance properties (`evaluateSubstance`)

```
if isHydrogen: return zeros
if not isReacDC:
    if not (isH2OSolvent or isH2Ovapor):
        tps = General:  CON → reference props | CPT → EmpiricalCpIntegration | HKF → SoluteHKFgems(solvent props, electro)
                        HKFR → SoluteHKFreaktoro(...) | HP98 → SoluteHollandPowell98(solvent at Tr,Pr and T,P)
                        AN91 → SoluteAnderson91(...) | other → tps stays empty
        if General != CON:
            T slot: CHP → HPLandau(tps) | CST → if General != CPT: EntropyCpIntegration
        if General != CON:
            P slot: AKI → AkinfievDiamond(tps, water props, ideal gas water, at T,P and Tr,Pr)
                    CEH → MinMurnaghanEOSHP98 | VKE, VBE → MinBerman88 | VBM → MinBMGottschalk
                    CORK, PRSV, EMP(CGF), SRK, PR78, STP → gas EoS(tps, apply_p, cancel_fugacity)
                    OFF → IdealGasLawVol(apply_p) | CON → ConMolVol
    if isH2OSolvent or isH2Ovapor:
        if General == CON: tps = reference props
        else T slot: WAT → WaterHGK (sets g_has_RTlnP) | WAR → WaterHGKreaktoro (g_has_RTlnP)
                     WWP → WaterWP95reaktoro | WZD → WaterZhangDuan2005
                     other: if T == CST and General != CPT → EntropyCpIntegration
                            else if General == CPT → EmpiricalCpIntegration
    if g_has_RTlnP and isH2Ovapor and not isH2OSolvent and not cancel_fugacity and aggregate_state == gas and P > 0:
        G -= R T ln(P/1bar); dG/dT -= R ln(P/1bar); dG/dP -= R T / P
    completeThermoProperties(tps, T, P, calculateCv = not (isH2OSolvent or isH2Ovapor))
    if isH2OSolvent and conventions.water_properties == STEAM_TABLES: toSteamTables
    elif not isH2OSolvent and conventions.apparent_properties == BERMAN_BROWN: toBermanBrown
else:
    tps = reacDC(T, P)                                   # §7.4
    completeThermoProperties(tps, T, P, not (isH2OSolvent or isH2Ovapor))
tps = fallback(tps)    # if preferences.fallback_to_reference_properties: notdefined → reference value
```

### 7.3 Reaction properties (`evaluateReaction`)

```
if no method in any slot: return fromReactants(T, P)        # sum ν_i X_i with gas P correction on
General: CON → reference props | REA → if T slot empty: T = LGK, elif T != LGK: warnIfLogKFunctionDisagrees (once)
if General != CON:
    T slot: LGX, LGK, EK0, EK1, EK2, EK3 → Reaction_LogK_fT(T, P, slot code)
            DMD → DolejsManning10(solvent props) | DKR → FrantzMarshall(solvent props)
            MRB → return RyzhenkoBryzgalin(solvent props)          # returns immediately
            IKZ → nothing
if General != CON:
    P slot: VKE, VBE → Reaction_Vol_fT(tpr)
            NUL, CON → constant-volume correction (twoPass with Vref, statuses kept, errors propagated as today)
```

### 7.4 Reaction-defined substance (`reacDC`)

As `reacDCthermoProperties`: if `defining_reaction` is set, X_s = (X_r − Σ_{i≠s} ν_i X_i) / ν_s with the gas
P-correction flag forced on during the sub-calls, statuses propagated, then: gas defined by a reaction with no P
method and no reaction volume → ideal-gas molar volume; if P = ConMolVol and the reaction has no P method (or OFF)
→ fallback then ConMolVol. If no reaction is set: evaluate as `ConstantProperties`.

### 7.5 Solvent

`propertiesSolvent` uses the `solvent_eos` method (T slot WAT/WAR/WWP/WZD); `electroPropertiesSolvent` uses the
`solvent_electro` method (General slot WJNR/WJNG/WSV14/WF97) with the solvent properties. State argument 0/1
overrides the engine's solvent state.

## 8. After phase 2: several methods per slot, detailed results

- **Detailed results (D65)** come in phase 2: the executor records `tps` after each slot step into
  `DetailedPropertiesSubstance { total; std::vector<std::pair<std::string, ThermoPropertiesSubstance>> steps; }`
  (`steps[i].first` = method id, value = properties after that step; the contribution of a step is the
  difference with the previous one). Auxiliary outputs: for water, `PropertiesSolvent` and
  `ElectroPropertiesSolvent` at T, P.
- **Several methods in one slot / additive contributions** (D45, D61: lattice + Landau + electronic) need a
  decision on how v2 orders and combines methods (§12 Q2). Until decided, the registry has the field `slot`
  and the executor refuses two methods in the same slot. The proposed rule, for discussion: `TCorrection` and
  `PCorrection` methods are applied in list order, each transforming the result of the previous one; a new slot
  `Additive` adds a contribution to the result; `General` stays single.
- **Composite methods (D7)**: not executed (D61).

## 9. Registering a user method (C++)

```cpp
#include <ThermoFun/Methods/MethodRegistry.h>

ThermoFun::MethodSpec spec;
spec.id = "MyCpModel";
spec.slot = ThermoFun::Slot::General;
spec.parameters = {{"c0", "J/(mol.K)", "J/(mol*K)", "constant Cp"}};
ThermoFun::MethodRegistry::instance().add(spec, [](const auto& app, const auto& substance) { /* ... */ });
```

User registration is a phase-2 extension point; it is documented as `provisional` (D5 maturity tags) in 1.0.
