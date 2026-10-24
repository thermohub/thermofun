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

/// What a method does to one property of the result it receives (Q2, decided: effects).
enum class Effect {
    None,     ///< does not touch the property
    Set,      ///< replaces the property (a base model, or the method that owns V)
    Add,      ///< adds an increment to the property
    Read      ///< uses the incoming value as an input (a transformation, e.g. Akinfiev-Diamond)
};

/// The effect of a method on each property it contributes to (G, H, S, Cp, V). U, A, Cv are always
/// recomputed by the engine at the end and are not declared.
struct MethodEffects
{
    Effect G = Effect::None, H = Effect::None, S = Effect::None, Cp = Effect::None, V = Effect::None;
    bool base = false;          ///< produces the complete property set from the record (CpIntegration, HKF, water EoS, ...)
    bool transform = false;     ///< reads the incoming result as input (any Read effect)
    int rank = 0;               ///< deterministic order among methods of the same group (lower first), see §8.2
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
    Slot slot = Slot::General;           ///< legacy slot (old API, legacy files, phase-2 executor)
    MethodEffects effects;               ///< how the method combines with others (§8)
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
today; in phase 2, v2 records with two methods in one slot are refused (`V2-MTH-007`); from step S2.9 the
effect rules of §8 replace this restriction.

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
| 5 `landau_holland_powell98` | T / 201 CTM_CHP | `HP98Landau` | `m_landau_phase_trans_props`: 0 Tc0 [degC], 1 Smax [J/(mol*K)], 2 Vmax [J/bar]; `alpha0` [1/K] (legacy record-level `m_expansivity`), `kappa0` [kbar, SI Pa] (legacy `m_compressibility`, the bulk modulus at 298 K) (05 §4 note) | yes |
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
| 38 `mv_eos_murnaghan_hp98` | P / 309 CPM_CEH | `HP98Min` | `alpha0` [1/K], `kappa0` [kbar, SI Pa] (as HP98Landau) | yes |
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

Parameter sets (phase 5): values come from the referenced `MethodParameterSet` (or the built-in default set), then
inline `parameters` with the same names override them (Q7).

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

## 8. Combining methods by their effects (Q2, decided)

Phase 2 keeps today's three slots (§3, §7) so results stay identical while the code moves. The **long-term rule**,
used for v2 records once step S2.9 is done, is that each method declares its **effect on each property**, and the
engine derives the order from the effects. Authors may list methods in any order; the result does not depend on it.

### 8.1 Why effects (cases found in the code and data)

| Case | Example | What the methods do |
|---|---|---|
| Cp integration + volume EoS | 381 minerals (`HP98Min`) | base sets G, H, S, Cp; EoS **adds** ΔG, ΔH, ΔS, ΔCp and V |
| Cp + Landau + Murnaghan | 60 minerals (e.g. Sillimanite) | Landau **adds** to G, H, S, Cp and **sets** V to its excess volume; Murnaghan then **adds** its V(T,P). Correct only with Landau first — with list order alone, swapping the two entries silently gives a wrong V |
| Cp + gas EoS | 43 gases | EoS adds RT ln φ to G and its terms to H, S; **sets** V |
| Cp (gas state) + Akinfiev–Diamond | 9 neutral aqueous species | AD **reads** the ideal-gas result and turns it into aqueous properties (transformation) |
| Water EoS + dielectric model | 20 water records | different outputs (roles), not combined |
| Future: lattice + magnetic + disorder Cp | SUPCRTBL, Berman minerals | pure sums |
| Two methods both setting V | never meaningful | must be an error |

### 8.2 Execution order (derived, deterministic)

```
1. base:        exactly one method with effects.base (else: ConstantProperties if the record has standard-state
                properties; else error V2-MTH-012)
2. transforms:  methods with effects.transform, in list order (they depend on what came before them)
3. setters:     non-base methods that Set any property, by rank
4. adders:      methods that only Add, by rank
5. engine:      U, A, Cv recomputed once; conventions (Berman–Brown, steam tables); fallback to reference values
```

Validation (at load, `Validate`):
- more than one base → `V2-MTH-012`;
- two non-base methods that `Set` the same property → `V2-MTH-013` (names the property and both methods);
- a method that `Add`s V when no earlier step defines V → `V2-MTH-014` (warning: V starts from 0);
- a transform after a setter or adder in the list → `V2-MTH-015` (warning: transforms run before them anyway).

`rank` makes the order of additions fixed (floating-point sums depend on order, and the two-build comparison has
tolerance 1e-12): built-in methods get the rank of their legacy slot (T-correction 100, P-correction 200), ties
broken by method id. So a legacy record converted to v2 runs its methods in exactly today's order.

### 8.3 Declared effects of the built-in methods (from reading the code — confirm in S2.3)

| Method | G | H | S | Cp | V | Base / transform | Rank | Notes |
|---|---|---|---|---|---|---|---|---|
| `ConstantProperties`, `CpIntegration`, `EntropyCpIntegration`, `HKFGems`, `HKFReaktoro`, `HP98Sol`, `Anderson91`, water EoS methods | Set | Set | Set | Set | Set | base | 0 | `CpIntegration`: check whether it sets V or leaves it undefined |
| `HP98Landau` | Add | Add | Add | Add | **Set** | — | 100 | V = Landau excess volume (`SolidHPLandau.cpp:74`) |
| `HP98Min` (Murnaghan) | Add | Add | Add | Add | Add | — | 200 | adds its full V(T,P) (`SolidMurnaghanHP98.cpp:56`); see note |
| `Berman88Vol`, `Dorogokupets88Vol` | Add | Add | Add | — | Set | — | 200 | sets V = V° then adds (`SolidBerman88.cpp:33-44`) |
| `BirchMurnaghanGott97` | Add | Add | Add | — | Add | — | 200 | |
| `PRSV`, `CORK`, `CGF`, `SRK`, `PR78`, `SternerPitzer` | Add | Add | Add | — | Set | — | 200 | depend on engine preferences (pressure correction, fugacity) |
| `IdealGasVol`, `ConMolVol` | Add | Add | — | — | Set | — | 200 | `ConMolVol` reads V° (from the record, not the incoming result) — confirm |
| `AkinfievDiamond03` | Read | Read | Read | Read | Set | transform | — | uses the incoming ideal-gas properties |
| dielectric methods | — | — | — | — | — | role SolventElectro | — | not part of substance composition |

**Note (possible inconsistency, keep as is):** at exactly T = Tr and P = Pr, `HP98Min` takes another branch
(`SolidMurnaghanHP98.cpp:63`) that **sets** V = V° instead of adding, which drops a Landau excess volume set
before it. For Landau minerals, V at exactly 298.15 K / 1 bar may therefore differ from the limit next to it.
Found by reading the code; not fixed during the refactoring (golden rule); reported as an issue (§12 Q22).

### 8.4 Reactions

Reaction methods follow the same idea with effects on logK, dG, dH, dS, dCp, dV: the log K method is the base
(`LogKAnalytic`, extrapolations, `FrantzMarshall`, `DolejsManning10`, `RyzhenkoBryzgalin`), volume methods
(`DrVolumeConst`, `DrVolumeFT`) add the P correction (`Add` on logK, dG, dH; `Set` on dV). `RyzhenkoBryzgalin`
today returns without a P correction (§7.3); declare it as base + `final = true` (no later steps), to keep that.

### 8.5 Detailed results

The executor records the properties after each step (§08 §4 `DetailedPropertiesSubstance::steps`); with effects,
the contribution of an `Add` method is exactly its increment, which is what D65 asks for.

### 8.6 Proposal to the schema

Add to the v2 Method record an `effects` block (`{G: set|add|read|none, ..., base: bool, rank: int}`), so data
authors and other calculators see how a method composes. ThermoFun checks a Method record's `effects` against its
built-in spec (`V2-MTH-011`). Composite methods (D7) stay not executed (D61).

## 9. Registering a user method (C++)

```cpp
#include <ThermoFun/Methods/MethodRegistry.h>

ThermoFun::MethodSpec spec;
spec.id = "MyCpModel";
spec.slot = ThermoFun::Slot::General;
spec.effects = {.G = Effect::Set, .H = Effect::Set, .S = Effect::Set, .Cp = Effect::Set, .base = true};
spec.parameters = {{"c0", "J/(mol.K)", "J/(mol*K)", "constant Cp"}};
ThermoFun::MethodRegistry::instance().add(spec, [](const auto& app, const auto& substance) { /* ... */ });
```

User registration is a phase-2 extension point; it is documented as `provisional` (D5 maturity tags) in 1.0.
