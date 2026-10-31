// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// ThermoFun includes
#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include "Common/Exception.h"
#include "Compat/SubstanceCompat.h"
#include "Engine/Executor.h"
#include "Engine/Solvent.h"
#include "Engine/ReactionBinding.h"
#include "Engine/ReactionDefined.h"
#include "Model/ReactionRecord.h"
#include "Conventions/HydrogenIon.h"
#include "Model/SubstanceRecord.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Reaction.h"
#include "Substance.h"
#include "Element.h"
#include "ThermoModelsSubstance.h"
#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/Rounding.hpp"
#include "ElectroModelsSolvent.h"
#include "ThermoModelsReaction.h"

#include "OptimizationUtils.h"
#include <functional>
#include <cmath>
#include <set>
#include <mutex>
#include <array>

namespace ThermoFun
{

/// Sets a flag and restores its previous value when the scope ends (also if an exception is thrown or the function returns
/// early); release() restores it before that
class FlagGuard
{
public:
    FlagGuard(bool& flag, bool value) : flag_(&flag), previous_(flag) { flag = value; }
    ~FlagGuard() { release(); }
    auto release() -> void { if (flag_) { *flag_ = previous_; flag_ = nullptr; } }
    FlagGuard(const FlagGuard&) = delete;
    FlagGuard& operator=(const FlagGuard&) = delete;
private:
    bool* flag_;
    bool previous_;
};

bool iequals(const std::string &a, const std::string &b)
{
    size_t sz = a.size();
    if (b.size() != sz)
        return false;
    for (size_t i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

using ThermoPropertiesSubstanceFunction =
    std::function<ThermoPropertiesSubstance(double, double, double &, std::string)>;

using ElectroPropertiesSolventFunction =
    std::function<ElectroPropertiesSolvent(double, double, double &, std::string, int)>;

using PropertiesSolventFunction =
    std::function<PropertiesSolvent(double, double, double &, std::string, int)>;

using ThermoPropertiesReactionFunction =
    std::function<ThermoPropertiesReaction(double, double, double &, std::string)>;

struct ThermoEngine::Impl
{
    /// The database instance
    Database database;

    mutable EnginePreferences preferences;

    mutable EngineConventions conventions;

    ThermoPropertiesSubstanceFunction thermo_properties_substance_fn;

    ElectroPropertiesSolventFunction electro_properties_solvent_fn;

    PropertiesSolventFunction properties_solvent_fn;

    ThermoPropertiesReactionFunction thermo_properties_reaction_fn;

    /// The database revision and the settings with which the cached results were calculated
    struct CacheState
    {
        std::uint64_t database_revision = 0;
        EnginePreferences preferences;
        EngineConventions conventions;
        DatabaseConventions database_conventions;
    };

    CacheState cached_for;

    /// The bound methods of each record that this engine has seen; the records are kept so that their addresses stay unique
    struct BoundCache
    {
        std::mutex mutex;
        std::map<const v2::SubstanceRecord*, std::shared_ptr<const v2::BoundSubstance>> substances;
        std::map<const v2::SubstanceRecord*, std::shared_ptr<const v2::BoundSolvent>> solvents;
        std::vector<std::shared_ptr<const v2::SubstanceRecord>> records;
        std::map<const v2::ReactionRecord*, std::shared_ptr<const v2::BoundReaction>> reactions;
        std::vector<std::shared_ptr<const v2::ReactionRecord>> reaction_records;
    };
    std::shared_ptr<BoundCache> bound_cache = std::make_shared<BoundCache>();

    Impl()
    {
        set_fn() ;
    }

    Impl(const Database &database)
        : database(database)
    {
        set_fn();
    }

    /// Two memoized copies of f, one for each value of apply_pressure_correction_to_gas_props, chosen at each call.
    /// The reactants of a reaction-defined substance and of a reaction from reactants are calculated with the flag
    /// switched on (FlagGuard), and the cache key does not hold the flag: with one cache, the result of a gas
    /// depended on the calculations made before it.
    template <typename Ret, typename... Args>
    auto memoizeByGasCorrection(const std::function<Ret(Args...)>& f) -> std::function<Ret(Args...)>
    {
        auto memoized = [&]() -> std::function<Ret(Args...)> {
            if (preferences.max_cache_size == 0)
                return memoize(f);
            return memoizeN(f, preferences.max_cache_size);
        };
        auto without = memoized(), with = memoized();
        return [this, without, with](Args... args) -> Ret {
            return preferences.apply_pressure_correction_to_gas_props ? with(std::forward<Args>(args)...)
                                                                      : without(std::forward<Args>(args)...);
        };
    }

    void set_fn()
    {
        thermo_properties_substance_fn = [=, this](double T, double P_, double &P, std::string symbol) {
            auto x = P_;
            return thermoPropertiesSubstance(T, P, symbol);
        };

        electro_properties_solvent_fn = [=, this](double T, double P_, double &P, std::string symbol, int state) {
            auto x = P_;
            return electroPropertiesSolvent(T, P, symbol, state);
        };

        properties_solvent_fn = [=, this](double T, double P_, double &P, std::string symbol, int state) {
            auto x = P_;
            return propertiesSolvent(T, P, symbol, state);
        };

        thermo_properties_reaction_fn = [=, this](double T, double P_, double &P, std::string symbol) {
            auto x = P_;
            return thermoPropertiesReaction(T, P, symbol);
        };

        if( preferences.enable_memoize ) {
            thermo_properties_substance_fn = memoizeByGasCorrection(thermo_properties_substance_fn);
            electro_properties_solvent_fn = memoizeByGasCorrection(electro_properties_solvent_fn);
            properties_solvent_fn = memoizeByGasCorrection(properties_solvent_fn);
            thermo_properties_reaction_fn = memoizeByGasCorrection(thermo_properties_reaction_fn);
        }
        const Database& db = database;
        cached_for = {db.revision(), preferences, conventions, db.conventions()};
    }

    /// Clear the cached results if the records, the preferences or the conventions changed after they were cached.
    /// Called at the start of each calculation, so a change in place (preferences(), conventions(),
    /// Database::substance(), ...) can never give a stale result.
    void dropStaleResults()
    {
        const Database& db = database;
        // solvent_state is not compared: each calculation of a substance sets it (to vapor for a gas), so it is
        // working state of the engine, not a setting of the user
        auto settings = cached_for.preferences;
        settings.solvent_state = preferences.solvent_state;
        if (db.revision() == cached_for.database_revision && preferences == settings &&
            conventions == cached_for.conventions && db.conventions() == cached_for.database_conventions)
            return;
        perturbed_engine.reset();
        set_fn();
    }

    /// The methods of a substance. Null if they give no one base method: then no method runs.
    auto boundMethods(const Substance& substance) const -> std::shared_ptr<const v2::BoundSubstance>
    {
        const auto record = substance.record();
        std::lock_guard<std::mutex> lock(bound_cache->mutex);
        const auto it = bound_cache->substances.find(record.get());
        if (it != bound_cache->substances.end())
            return it->second;
        auto bound = v2::canBind(*record) ? v2::bindSubstance(record) : nullptr;
        bound_cache->substances[record.get()] = bound;
        bound_cache->records.push_back(record);
        return bound;
    }

    /// The state of the solvent that the methods of a record need, calculated with the methods of the solvent record.
    /// A pressure of 0 is the saturation pressure: it is written to P.
    auto solventInputs(const v2::BoundSubstance& bound, double T, double& P) const -> std::optional<v2::SolventInputs>
    {
        if (!bound.needs.solvent)
            return std::nullopt;
        v2::SolventInputs in;
        const auto symbol = preferences.solvent_symbol;
        in.properties = properties_solvent_fn(T, P, P, symbol, -1);
        if (bound.needs.dielectric)
            in.electro = electro_properties_solvent_fn(T, P, P, symbol, -1);
        if (bound.needs.solvent_at_reference)
        {
            const auto& solvent = database.getSubstance(symbol);
            double Pr = solvent.referenceP();
            const double Tr = solvent.referenceT();
            in.properties_at_reference = properties_solvent_fn(Tr, Pr, Pr, symbol, -1);
            if (bound.needs.ideal_gas_water)
            {
                in.water = thermo_properties_substance_fn(T, P, P, symbol);
                in.ideal_gas_water = WaterIdealGasWoolley(solvent).thermoProperties(T, P);
                in.water_at_reference = thermo_properties_substance_fn(Tr, Pr, Pr, symbol);
                in.ideal_gas_water_at_reference = WaterIdealGasWoolley(solvent).thermoProperties(Tr, Pr);
            }
        }
        return in;
    }

    /// The methods of a reaction. Null if they give no one base method: then no method runs.
    auto boundReaction(const Reaction& reaction) const -> std::shared_ptr<const v2::BoundReaction>
    {
        const auto record = reaction.record();
        std::lock_guard<std::mutex> lock(bound_cache->mutex);
        const auto it = bound_cache->reactions.find(record.get());
        if (it != bound_cache->reactions.end())
            return it->second;
        auto bound = v2::canBindReaction(*record) ? v2::bindReaction(record) : nullptr;
        bound_cache->reactions[record.get()] = bound;
        bound_cache->reaction_records.push_back(record);
        return bound;
    }

    /// The methods of a water record.
    auto boundSolvent(const Substance& substance) const -> std::shared_ptr<const v2::BoundSolvent>
    {
        const auto record = substance.record();
        std::lock_guard<std::mutex> lock(bound_cache->mutex);
        const auto it = bound_cache->solvents.find(record.get());
        if (it != bound_cache->solvents.end())
            return it->second;
        auto bound = v2::bindSolvent(record);
        bound_cache->solvents[record.get()] = bound;
        bound_cache->records.push_back(record);
        return bound;
    }

    auto substanceOptions() const -> v2::SubstanceOptions
    {
        return {preferences.apply_pressure_correction_to_gas_props, preferences.cancel_fugacity_coefficient_in_gas_props};
    }

    /// The result of the executor in the form of the old engine: V in J/bar, and the status of each property
    static auto toThermoProperties(const v2::SubstanceResult& r) -> ThermoPropertiesSubstance
    {
        auto one = [](const v2::PropertyValue& v, const v2::ValueInfo& i, double factor = 1.0) {
            Reaktoro_::ThermoProperty p;
            p.val = v.val * factor;
            p.ddt = v.ddt * factor;
            p.ddp = v.ddp * factor;
            p.err = i.err * factor;
            p.sta = {i.status == v2::ValueInfo::Status::NotDefined ? Reaktoro_::Status::notdefined
                     : i.status == v2::ValueInfo::Status::Assigned ? Reaktoro_::Status::assigned
                     : i.status == v2::ValueInfo::Status::Read ? Reaktoro_::Status::read : Reaktoro_::Status::calculated, i.message};
            return p;
        };
        ThermoPropertiesSubstance tps;
        tps.gibbs_energy = one(r.values.G, r.info.G);
        tps.enthalpy = one(r.values.H, r.info.H);
        tps.entropy = one(r.values.S, r.info.S);
        tps.heat_capacity_cp = one(r.values.Cp, r.info.Cp);
        tps.volume = one(r.values.V, r.info.V, 1e5);  // 1 J/bar = 1e-5 m3
        return tps;
    }

    auto toSteamTables(ThermoPropertiesSubstance &tps, WaterTripleProperties wat) const -> void
    {
        // constants are subtracted: the derivatives, error and status are not changed
        tps.gibbs_energy.val -= wat.Gtr;
        tps.enthalpy.val -= wat.Htr;
        tps.entropy.val -= wat.Str;
        tps.helmholtz_energy.val -= wat.Atr;
        tps.internal_energy.val -= wat.Utr;
    }

    /// The properties that follow from the others (exact derivatives and propagated errors), for the pressure P (Pa) and the temperature T (K):
    ///   U = H - P V,  A = U - T S,  Cv = Cp - T V alpha^2/beta = Cp - T V_T^2/(-V_P)
    /// with V in J/bar (V_T, V_P: the derivatives of V). The derivatives of Cv are those of Cp (the second derivatives of the volume
    /// are not available): they are exact when the volume does not depend on T (Cv = Cp). Cv of the solvent is the one of its model.
    auto completeThermoProperties(ThermoPropertiesSubstance &tps, double T, double P, bool calculateCv) const -> void
    {
        using Reaktoro_::Status;
        const double Pbar = P / bar_to_Pa;
        auto defined = [](const Reaktoro_::ThermoProperty& x) { return x.sta.first != Status::notdefined; };

        auto &H = tps.enthalpy, &V = tps.volume, &S = tps.entropy, &U = tps.internal_energy, &A = tps.helmholtz_energy;
        if (defined(H) && defined(V))
        {
            const double uv = H.val() - Pbar*V.val(), ut = H.ddt() - Pbar*V.ddt(), up = H.ddp() - V.val()/bar_to_Pa - Pbar*V.ddp();
            const auto sta = U.sta;
            U.propagateFrom(H, V);
            U.setError({{1.0, H.err}, {Pbar, V.err}});
            U.val = uv; U.ddt = ut; U.ddp = up;
            if (!Reaktoro_::kWithAutodiff) U.ddt = U.ddp = 0.0;
            if (sta.first == Status::notdefined) U.sta = {Status::calculated, std::string("")};
        }
        if (defined(U) && defined(S))
        {
            const double av = U.val() - T*S.val(), at = U.ddt() - S.val() - T*S.ddt(), ap = U.ddp() - T*S.ddp();
            A.propagateFrom(U, S);
            A.setError({{1.0, U.err}, {T, S.err}});
            A.val = av; A.ddt = at; A.ddp = ap;
            if (!Reaktoro_::kWithAutodiff) A.ddt = A.ddp = 0.0;
        }
        auto &Cp = tps.heat_capacity_cp, &Cv = tps.heat_capacity_cv;
        if (calculateCv && !Reaktoro_::kWithAutodiff)
        {   // Cv needs dV/dT and dV/dP
            Cv = Reaktoro_::ThermoProperty();
            Cv.sta = {Status::notdefined, "Cv needs the derivatives of the volume: not calculated (built without autodiff)"};
        }
        else if (calculateCv && defined(Cp) && defined(V))
        {
            double diff = 0.0;                                 // Cp - Cv
            if (V.ddp() < 0.0 && V.ddt() != 0.0)
                diff = 1e-5 * T * V.ddt()*V.ddt() / (-V.ddp());
            Cv.propagateFrom(Cp, V);
            Cv.setError({{1.0, Cp.err}});
            Cv.val = Cp.val() - diff; Cv.ddt = Cp.ddt(); Cv.ddp = Cp.ddp();
        }
    }

    auto toBermanBrown(ThermoPropertiesSubstance &tps, const Substance &subst) const -> void
    {
        const auto Tr = subst.referenceT();
        const auto entropyElements = database.elementalEntropyFormula(subst.formula());
        tps.gibbs_energy.val -= (Tr * entropyElements); // a constant: the derivatives, error and status are not changed
    }

    /// What the record says about the way to calculate a substance
    struct SubstanceKind
    {
        bool isHydrogen = false;
        bool isH2Ovapor = false;
        bool isH2OSolvent = false;
        bool isReacDC = false;
    };

    auto kindOf(const Substance &substance) const -> SubstanceKind
    {
        const auto record = substance.record();
        SubstanceKind kind;
        kind.isHydrogen = v2::isHydrogenIon(*record);
        kind.isH2Ovapor = compat::isWaterVapour(*record);
        kind.isH2OSolvent = record->class_ == v2::SubstanceClass::WaterSolvent;
        kind.isReacDC = substance.thermoCalculationType() == SubstanceThermoCalculationType::type::REACDC ||
                        record->preferred_property_source == v2::PropertySource::Reaction || record->methods.empty();

        preferences.solvent_state = v2::solventStateOf(*record);

        // see if the solvent of an aqueous solute is in the database; if not, search for a solvent
        if (record->class_ == v2::SubstanceClass::AqueousSolute && !database.containsSubstance(preferences.solvent_symbol))
        {
            std::map<std::string, v2::SubstanceClass> classes;
            for (const auto& pair : database.mapSubstances())
                classes[pair.first] = pair.second.substanceClass() == SubstanceClass::type::AQSOLVENT ? v2::SubstanceClass::WaterSolvent : v2::SubstanceClass::Other;
            const auto chosen = v2::chooseSolvent(preferences.solvent_symbol, "", classes);
            if (!chosen.empty())
                preferences.solvent_symbol = chosen;
        }
        return kind;
    }

    static auto hasMethod(const std::vector<v2::MethodApplication>& methods, const char* id) -> bool
    {
        return std::any_of(methods.begin(), methods.end(), [&](const auto& a) { return a.method == id; });
    }

    auto thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
    {
        return thermoPropertiesSubstance(T, P, database.getSubstance(substance));
    }

    auto thermoPropertiesSubstance(double T, double &P, const Substance &substance) const -> ThermoPropertiesSubstance
    {
        const SubstanceKind kind = kindOf(substance);
        ThermoPropertiesSubstance tps;

        if (kind.isHydrogen)
        {
            tps.volume = 0.0;
            tps.entropy = 0.0;
            tps.enthalpy = 0.0;
            tps.gibbs_energy = 0.0;
            tps.internal_energy = 0.0;
            tps.heat_capacity_cp = 0.0;
            tps.heat_capacity_cv = 0.0;
            tps.helmholtz_energy = 0.0;
            return tps;
        }

        const bool isWater = kind.isH2OSolvent || kind.isH2Ovapor;

        // without a reaction, a substance that should have one has its reference properties
        if (kind.isReacDC && !substance.reactionSymbol().empty())
        {
            tps = reacDCthermoProperties(T, P, substance);
            completeThermoProperties(tps, T, P, !isWater);
        }
        else
        {
            bool water_g_contains_rt_ln_p = false; // true if G of the water model has the term RT ln(P/1 bar)

            if (kind.isReacDC)
                tps = substance.thermoReferenceProperties();
            else if (isWater)
            {
                const auto solvent = boundSolvent(substance);
                if (solvent->equation_of_state)
                {
                    tps = solvent->equation_of_state->substanceProperties(T, P, preferences.solvent_state, database.conventions().water_triple_point_reference);
                    water_g_contains_rt_ln_p = solvent->equation_of_state->gibbsEnergyContainsRTlnP();
                }
                else if (solvent->reference_values)
                    tps = toThermoProperties(v2::executeSubstance(*solvent->reference_values, T, P, substanceOptions()));
            }
            else if (const auto bound = boundMethods(substance))
            {
                const auto solvent = solventInputs(*bound, T, P);
                tps = toThermoProperties(v2::executeSubstance(*bound, T, P, substanceOptions(), nullptr, solvent ? &*solvent : nullptr));
            }

            // With the fugacity coefficient kept (switch false), G of steam is G(T, 1 bar) + RT ln(phi) like the other fluids: RT ln(P) that the
            // HGK equation of state contains in G is removed (H and S are not changed)
            if (water_g_contains_rt_ln_p && kind.isH2Ovapor && !kind.isH2OSolvent && !preferences.cancel_fugacity_coefficient_in_gas_props &&
                substance.aggregateState() == AggregateState::type::GAS && P > 0.0)
            {
                const double Pb = P / bar_to_Pa;
                tps.gibbs_energy.val -= R_CONSTANT * T * std::log(Pb);
                tps.gibbs_energy.ddt -= R_CONSTANT * std::log(Pb);
                tps.gibbs_energy.ddp -= R_CONSTANT * T / P;
            }

            // the properties that follow from the others (U, A, Cv), in the convention of the models
            completeThermoProperties(tps, T, P, !isWater);

            /// Convetion convert
            if (kind.isH2OSolvent)
            {
                if (iequals(conventions.water_properties, "STEAM_TABLES"))
                    toSteamTables(tps, waterTripleData.at(database.conventions().water_triple_point_reference));
            }
            else if (iequals(conventions.apparent_properties, "BERMAN_BROWN"))
                toBermanBrown(tps, substance);
        }

        // check properties
        tps = fallbackThermoPropertiesSubstance(tps, substance);

        return tps;
    }

    auto fallbackThermoPropertiesSubstance(ThermoPropertiesSubstance tps, Substance subst) const -> ThermoPropertiesSubstance
    {
        auto tpref = subst.thermoReferenceProperties();
        // leave volume as is in substance record if no function to calculate it exists
        if (preferences.fallback_to_reference_properties)
        {
            if (tps.volume.sta.first == Reaktoro_::Status::notdefined)
                tps.volume = tpref.volume;
            if (tps.gibbs_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.gibbs_energy = tpref.gibbs_energy;
            if (tps.enthalpy.sta.first == Reaktoro_::Status::notdefined)
                tps.enthalpy = tpref.enthalpy;
            if (tps.entropy.sta.first == Reaktoro_::Status::notdefined)
                tps.entropy = tpref.entropy;
            if (tps.heat_capacity_cp.sta.first == Reaktoro_::Status::notdefined)
                tps.heat_capacity_cp = tpref.heat_capacity_cp;
            if (tps.heat_capacity_cv.sta.first == Reaktoro_::Status::notdefined)
                tps.heat_capacity_cv = tpref.heat_capacity_cv;
            if (tps.internal_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.internal_energy = tpref.internal_energy;
            if (tps.helmholtz_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.helmholtz_energy = tpref.helmholtz_energy;
        }
        return tps;
    }

    auto electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
    {
        return electroPropertiesSolvent(T, P, database.getSubstance(solvent), state);
    }

    auto electroPropertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> ElectroPropertiesSolvent
    {
        const SubstanceKind kind = kindOf(solvent);
        if (state>-1 && state<2)
            preferences.solvent_state = state;
        PropertiesSolvent ps = propertiesSolvent(T, P, solvent, preferences.solvent_state);
        ElectroPropertiesSolvent eps;

        if (kind.isH2OSolvent)
            if (const auto bound = boundSolvent(solvent); bound->dielectric)
                eps = bound->dielectric->properties(T, P, ps, preferences.solvent_state);
        return eps;
    }

    auto propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
    {
        return propertiesSolvent(T, P, database.getSubstance(solvent), state);
    }

    auto propertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> PropertiesSolvent
    {
        const SubstanceKind kind = kindOf(solvent);
        if (state > -1 && state <2)
            preferences.solvent_state = state;
        PropertiesSolvent ps;

        if (kind.isH2OSolvent)
            if (const auto bound = boundSolvent(solvent); bound->equation_of_state)
                ps = bound->equation_of_state->properties(T, P, preferences.solvent_state);
        return ps;
    }

    auto reacDCthermoProperties(double T, double &P, Substance subst) const -> ThermoPropertiesSubstance
    {
        ThermoPropertiesSubstance tps, reacTps;
        ThermoPropertiesReaction tpr;
        const std::string reactionSymbol = subst.reactionSymbol();
        const Reaction reaction = database.getReaction(reactionSymbol);
        std::map<std::string, double> reactants;

        const v2::DefiningReactionGuard no_cycle(subst.symbol());

        // if reaction involves gases we switch on the P correction to the thermo props
        FlagGuard gas_pressure_correction(preferences.apply_pressure_correction_to_gas_props, true);

        tpr = thermo_properties_reaction_fn(T, P, P, reactionSymbol); /*thermoPropertiesReaction(T, P, reactionSymbol);*/

        reactants = reaction.reactants();

        // the properties of the other reactants
        std::vector<std::pair<ThermoPropertiesSubstance, double>> others;
        for (const auto& reactant : reactants)
        {
            if (reactant.first != subst.symbol())
                others.push_back({thermo_properties_substance_fn(T, P, P, reactant.first) /* thermoPropertiesSubstance(T, P, reactant.first);*/, reactant.second});
        }

        const double coeff = reactants[subst.symbol()];

        // the properties of the substance from the properties of the reaction and of the other reactants
        tps = v2::substanceFromReaction(T, P, tpr, others, coeff);

        gas_pressure_correction.release();

        // a gas defined by a reaction and without a volume method has the ideal gas molar volume
        if (subst.aggregateState() == AggregateState::type::GAS &&
            subst.substanceClass() == SubstanceClass::type::GASFLUID && !subst.method_P() && P > 0.0 &&
            !(tpr.reaction_volume.sta.first != Reaktoro_::Status::notdefined && tpr.reaction_volume.val != 0.0))
        {
            v2::setIdealGasVolume(tps, T, P);
        }

        // pressure correction on rdc assuming constant molar volume
        if (hasMethod(subst.record()->methods, "ConstantVolume") &&
            !hasMethod(reaction.record()->methods, "ReactionConstantVolume") && !hasMethod(reaction.record()->methods, "ReactionVolumePolynomial"))
        {
            // check properites
            tps = fallbackThermoPropertiesSubstance(tps, subst);
            tps = ConMolVol(subst).thermoProperties(T, P, tps);
        }

        return tps;
    }

    auto thermoPropertiesReaction(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
    {
        // if the thermoPropertiesReaction function is called using a reaction equation
        if (!database.containsReaction(symbol) && (symbol.find("=") != std::string::npos))
        {
            Reaction reaction;
            reaction.fromEquation(symbol);
            return thermoPropertiesReaction(T, P, reaction);
            //database.addReaction(reaction);
        }
        return thermoPropertiesReaction(T, P, database.getReaction(symbol));
    }

    /// A record with a logK(T) function and a method of T correction that does not use it: the function and the reference values
    /// should give the same logK at the reference temperature. One warning for each reaction.
    auto warnIfLogKFunctionDisagrees(const Reaction &reaction) const -> void
    {
        if (reaction.thermoParameters().reaction_logK_fT_coeff.empty())
            return;
        const auto ref = reaction.thermoReferenceProperties();
        const double Tr = reaction.referenceT();
        double lgK_ref;
        if (ref.log_equilibrium_constant.sta.first != Reaktoro_::Status::notdefined)
            lgK_ref = ref.log_equilibrium_constant.val;
        else if (ref.reaction_gibbs_energy.sta.first != Reaktoro_::Status::notdefined)
            lgK_ref = -ref.reaction_gibbs_energy.val / (R_CONSTANT * Tr * lg_to_ln);
        else
            return;
        const auto fn = Reaction_LogK_fT(reaction).thermoProperties(Tr, reaction.referenceP());
        const double lgK_fn = fn.log_equilibrium_constant.val;
        if (!std::isfinite(lgK_fn) || std::fabs(lgK_fn - lgK_ref) <= 0.01)
            return;
        static std::mutex warned_mutex;
        static std::set<std::string> warned;
        std::lock_guard<std::mutex> lock(warned_mutex);
        if (warned.insert(reaction.symbol()).second)
            thfun_logger->warn("Reaction {}: logK at the reference temperature is {} from the logK(T) function and {} from the reference values; "
                               "the reference values are used (the T correction method of the record does not use the function)",
                               reaction.symbol(), lgK_fn, lgK_ref);
    }

    auto thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction
    {
        if (const auto bound = boundReaction(reaction))
        {
            if (bound->base->spec().id != "LogKPolynomial")
                warnIfLogKFunctionDisagrees(reaction);
            std::optional<PropertiesSolvent> solvent;
            if (bound->needs_solvent)
                solvent = properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1);
            return v2::executeReaction(*bound, T, P, solvent ? &*solvent : nullptr);
        }

        // no method at all: from the reactants
        if (reaction.record()->methods.empty())
            return thermoPropertiesReactionFromReactants(T, P, reaction);
        return {};
    }

    auto thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
    {
        return thermoPropertiesReactionFromReactants(T, P, database.getReaction(symbol));
    }

    auto thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
    {
        ThermoPropertiesReaction tpr;
        tpr.reaction_heat_capacity_cp = 0.0;
        tpr.reaction_gibbs_energy = 0.0;
        tpr.reaction_enthalpy = 0.0;
        tpr.reaction_entropy = 0.0;
        tpr.reaction_volume = 0.0;
        tpr.ln_equilibrium_constant = 0.0;
        tpr.log_equilibrium_constant = 0.0;
        tpr.reaction_heat_capacity_cv = 0.0;
        tpr.reaction_internal_energy = 0.0;
        tpr.reaction_helmholtz_energy = 0.0;

        FlagGuard gas_pressure_correction(preferences.apply_pressure_correction_to_gas_props, true);

        // the properties of the reactants
        std::vector<std::pair<ThermoPropertiesSubstance, double>> components;
        std::vector<std::string> symbols;

        for (auto &reactant : reaction.reactants())
        {
            auto coeff      = reactant.second;
            auto substance  = reactant.first;
            auto s = database.getSubstance(substance);

            // check if substance is correctly defined
            if (!s.methodGenEOS() && !s.method_P() && !s.method_T() && s.reactionSymbol() == reaction.symbol())
            {
                errorMethodNotFound("reaction", reaction.symbol(), __LINE__, __FILE__);
            }

            components.push_back({thermo_properties_substance_fn(T, P,P, substance) /*thermoPropertiesSubstance(T, P, substance);*/, coeff});
            symbols.push_back(substance);
        }

        tpr = ReactionFromReactantsProperties(reaction).thermoProperties(T, P, components, symbols);

        gas_pressure_correction.release();
        return tpr;
    }

    //=================================================================================================
    // Propagation of the errors of the reference properties and of the coefficients of the models
    //=================================================================================================

    /// A copy of the engine used to calculate with perturbed parameters (without memoization and error propagation)
    mutable std::shared_ptr<ThermoEngine> perturbed_engine;

    auto perturbedEngine() const -> ThermoEngine&
    {
        if (!perturbed_engine)
            perturbed_engine.reset(new ThermoEngine(database));
        auto& impl = *perturbed_engine->pimpl;
        const bool reset = impl.preferences.enable_memoize;
        impl.preferences = preferences;
        impl.preferences.enable_memoize = false;
        impl.preferences.propagate_parameter_errors = false;
        impl.preferences.round_to_uncertainty = false;
        impl.conventions = conventions;
        if (reset) impl.set_fn(); // without memoization
        return *perturbed_engine;
    }

    static auto fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.gibbs_energy, &p.helmholtz_energy, &p.internal_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp, &p.heat_capacity_cv};
    }
    static auto fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.reaction_gibbs_energy, &p.reaction_helmholtz_energy, &p.reaction_internal_energy, &p.reaction_enthalpy, &p.reaction_entropy,
                &p.reaction_volume, &p.reaction_heat_capacity_cp, &p.reaction_heat_capacity_cv, &p.ln_equilibrium_constant, &p.log_equilibrium_constant};
    }
    static auto referenceFields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.gibbs_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp};
    }
    static auto referenceFields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.log_equilibrium_constant, &p.reaction_gibbs_energy, &p.reaction_enthalpy, &p.reaction_entropy, &p.reaction_volume, &p.reaction_heat_capacity_cp};
    }

    /// The records (substances and reactions of the database) a calculation depends on
    struct Records
    {
        std::set<std::string> substances, reactions;
    };

    auto collectSubstance(Records& r, const std::string& symbol) const -> void
    {
        if (r.substances.count(symbol) || !database.containsSubstance(symbol)) return;
        r.substances.insert(symbol);
        const auto reaction = database.getSubstance(symbol).reactionSymbol();
        if (!reaction.empty()) collectReaction(r, reaction);
    }

    auto collectReaction(Records& r, const std::string& symbol) const -> void
    {
        if (r.reactions.count(symbol) || !database.containsReaction(symbol)) return;
        r.reactions.insert(symbol);
        for (const auto& reactant : database.getReaction(symbol).reactants())
            collectSubstance(r, reactant.first);
    }

    /// One parameter with an uncertainty
    struct Parameter
    {
        int record;           // 0: substance of the database, 1: reaction of the database, 2: given substance, 3: given reaction
        std::string symbol;
        int reference;        // index of the reference property, or -1 for a coefficient
        std::string key;      // the coefficient (see UncertainCoefficient)
        double sigma;
    };

    /// Add the effect of the uncertain parameters to the errors of the properties calculated by eval: the standard error
    /// of a property is the quadrature sum of (y(p+s) - y(p-s))/2 over the parameters p with an error s (first-order
    /// propagation, independent parameters; parameters shared by several records are perturbed together)
    template<class Props, class Eval>
    auto addParameterErrors(Props& result, const Records& records, const Substance* givenS, const Reaction* givenR, Eval eval) const -> void
    {
        using Sub = Substance;
        using Rea = Reaction;
        std::vector<Parameter> parameters;

        auto listSubstance = [&](Sub s, int kind, const std::string& symbol) {
            auto ref = s.thermoReferenceProperties();
            auto refs = referenceFields(ref);
            for (size_t i = 0; i < refs.size(); ++i)
                if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
            auto p = s.thermoParameters();
            for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
        };
        auto listReaction = [&](Rea r, int kind, const std::string& symbol) {
            auto ref = r.thermoReferenceProperties();
            auto refs = referenceFields(ref);
            for (size_t i = 0; i < refs.size(); ++i)
                if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
            auto p = r.thermoParameters();
            for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
        };
        for (const auto& symbol : records.substances) listSubstance(database.getSubstance(symbol), 0, symbol);
        for (const auto& symbol : records.reactions) listReaction(database.getReaction(symbol), 1, symbol);
        if (givenS) listSubstance(*givenS, 2, givenS->symbol());
        if (givenR) listReaction(*givenR, 3, givenR->symbol());
        if (parameters.empty()) return;

        auto& engine = perturbedEngine();
        auto& db = engine.pimpl->database;

        auto perturb = [&](const Parameter& q, double delta) -> Props {
            // the value of the parameter is changed in a copy of its record
            Substance tmpS; Reaction tmpR;
            const bool isSub = (q.record == 0 || q.record == 2);
            if (q.record == 0) tmpS = db.getSubstance(q.symbol);
            if (q.record == 1) tmpR = db.getReaction(q.symbol);
            if (q.record == 2) tmpS = *givenS;
            if (q.record == 3) tmpR = *givenR;
            const Substance origS = tmpS; const Reaction origR = tmpR;

            auto apply = [&](auto& record) {
                if (q.reference >= 0)
                {
                    auto ref = record.thermoReferenceProperties();
                    referenceFields(ref)[q.reference]->val += delta;
                    record.setThermoReferenceProperties(ref);
                }
                else
                {
                    auto par = record.thermoParameters();
                    for (const auto& c : uncertainCoefficients(par))
                        if (c.key == q.key) { *c.value += delta; break; }
                    record.setThermoParameters(par);
                }
            };
            if (isSub) apply(tmpS); else apply(tmpR);

            Props out;
            try
            {
                if (q.record == 0) db.setMapSubstances({{tmpS.symbol(), tmpS}});
                if (q.record == 1) db.setMapReactions({{tmpR.symbol(), tmpR}});
                out = eval(engine, q.record == 2 ? &tmpS : givenS, q.record == 3 ? &tmpR : givenR);
            }
            catch (...)
            {
                if (q.record == 0) db.setMapSubstances({{origS.symbol(), origS}});
                if (q.record == 1) db.setMapReactions({{origR.symbol(), origR}});
                throw;
            }
            if (q.record == 0) db.setMapSubstances({{origS.symbol(), origS}});
            if (q.record == 1) db.setMapReactions({{origR.symbol(), origR}});
            return out;
        };

        auto fr = fields(result);
        std::vector<double> sum2(fr.size(), 0.0);
        std::vector<char> incomplete(fr.size(), 0); // a contribution to the error is missing
        auto defined = [&](size_t k) { return fr[k]->sta.first != Reaktoro_::Status::notdefined; };
        for (const auto& q : parameters)
        {
            try
            {
                auto plus  = perturb(q, +q.sigma);
                auto minus = perturb(q, -q.sigma);
                auto fp = fields(plus), fm = fields(minus);
                for (size_t k = 0; k < fp.size(); ++k)
                {
                    if (!defined(k)) continue;
                    if (fp[k]->sta.first == Reaktoro_::Status::notdefined || fm[k]->sta.first == Reaktoro_::Status::notdefined)
                    {
                        incomplete[k] = 1;
                        continue;
                    }
                    const double d = 0.5 * (fp[k]->val - fm[k]->val);
                    if (std::isfinite(d)) sum2[k] += d * d; else incomplete[k] = 1;
                }
            }
            catch (...) // the calculation fails for this parameter: the error of every property is incomplete
            {
                std::fill(incomplete.begin(), incomplete.end(), 1);
            }
        }
        for (size_t k = 0; k < fr.size(); ++k)
        {
            if (!defined(k)) { fr[k]->err = std::sqrt(sum2[k]); continue; }
            if (!incomplete[k]) { fr[k]->err = std::sqrt(sum2[k]); continue; }
            // not a number instead of an error that is too small (or 0) and looks complete
            fr[k]->err = std::numeric_limits<double>::quiet_NaN();
            auto& message = fr[k]->sta.second;
            message += std::string(message.empty() ? "" : " ") +
                "The uncertainty from the parameter errors is incomplete (the calculation failed or was not defined for a perturbed parameter): err is not a number.";
        }
    }

    /// Round the values of the properties to the decimals of their uncertainties (NEA TDB rules) if requested
    template<class Props>
    auto roundResults(Props& result) const -> void
    {
        if (!preferences.round_to_uncertainty) return;
        for (auto* f : fields(result))
            if (f->sta.first != Reaktoro_::Status::notdefined)
            {
                double value = f->val, error = f->err;
                rounding::toUncertainty(value, error, preferences.uncertainty_significant_digits);
                f->val = value;
                f->err = error;
            }
    }

    auto recordsOfSubstance(const std::string& symbol) const -> Records { Records r; collectSubstance(r, symbol); return r; }
    auto recordsOfReaction(const std::string& symbol) const -> Records { Records r; collectReaction(r, symbol); return r; }
    auto recordsOfReactants(const Reaction& reaction) const -> Records
    {
        Records r;
        for (const auto& reactant : reaction.reactants()) collectSubstance(r, reactant.first);
        return r;
    }
};

ThermoEngine::ThermoEngine(const std::string filename)
    : pimpl()
{
    Database db(filename);
    pimpl.reset(new Impl(db));
}

ThermoEngine::ThermoEngine(const Database &database)
    : pimpl(new Impl(database))
{
}

ThermoEngine::ThermoEngine(const ThermoEngine &other)
    : pimpl(new Impl(*other.pimpl))
{
    // the copied functions are bound to the Impl of the other engine (and so dangle when it is destroyed):
    // bind them to this one (the memoization caches start empty)
    pimpl->set_fn();
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tps = pimpl->thermo_properties_substance_fn(T, P, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tps, pimpl->recordsOfSubstance(substance), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesSubstance(T, p, substance); });
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->electro_properties_solvent_fn(T, P, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->properties_solvent_fn(T, P, P, solvent, state);
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, const Substance& substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tps = pimpl->thermoPropertiesSubstance(T, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
    {
        ThermoEngine::Impl::Records records;
        if (!substance.reactionSymbol().empty()) pimpl->collectReaction(records, substance.reactionSymbol());
        pimpl->addParameterErrors(tps, records, &substance, nullptr,
            [&](ThermoEngine& e, const Substance* s, const Reaction*) { double p = P0; return e.thermoPropertiesSubstance(T, p, *s); });
    }
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->electroPropertiesSolvent(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->propertiesSolvent(T, P, solvent, state);
}

// Reaction
auto ThermoEngine::thermoPropertiesReaction(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tpr = pimpl->thermo_properties_reaction_fn(T, P, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReaction(reaction), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesReaction(T, p, reaction); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReactionFromReactants(T, P, symbol);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReaction(symbol), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesReactionFromReactants(T, p, symbol); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReaction(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReaction(T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReactants(reaction), nullptr, &reaction,
            [&](ThermoEngine& e, const Substance*, const Reaction* r) { double p = P0; return e.thermoPropertiesReaction(T, p, *r); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReactionFromReactants(T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReactants(reaction), nullptr, &reaction,
            [&](ThermoEngine& e, const Substance*, const Reaction* r) { double p = P0; return e.thermoPropertiesReactionFromReactants(T, p, *r); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::setPreferences(const EnginePreferences preferences) -> void
{
    pimpl->preferences = preferences;
    pimpl->perturbed_engine.reset();
    pimpl->set_fn(); // applies the memoization settings (the caches are cleared)
}

auto ThermoEngine::setConventions(const EngineConventions conventions) -> void
{
    pimpl->conventions = conventions;
    pimpl->dropStaleResults();
}

auto ThermoEngine::setSolventSymbol(const std::string solvent_symbol) -> void
{
    pimpl->preferences.solvent_symbol = solvent_symbol;
}

auto ThermoEngine::solventSymbol() -> std::string&
{
    return pimpl->preferences.solvent_symbol;
}

EnginePreferences& ThermoEngine::preferences() {
    return pimpl->preferences;
}

const EnginePreferences& ThermoEngine::preferences() const {
    return pimpl->preferences;
}

auto ThermoEngine::database() const -> const Database &
{
    return pimpl->database;
}

auto ThermoEngine::appendData(std::string filename) -> void
{
    pimpl->database.appendData(filename);
    pimpl->perturbed_engine.reset();
}

auto ThermoEngine::appendData(std::vector<std::string> jsonRecords, std::string _label = "unknown label") -> void
{
    pimpl->database.appendData(jsonRecords, _label);
    pimpl->perturbed_engine.reset();
}

auto ThermoEngine::explain(double T, double P, std::string substance) const -> std::vector<ExplainStep>
{
    std::vector<ExplainStep> out;
    const auto bound = pimpl->boundMethods(pimpl->database.getSubstance(substance));
    if (!bound)
        return out;
    std::vector<v2::ExplainStep> steps;
    const auto solvent = pimpl->solventInputs(*bound, T, P);
    v2::executeSubstance(*bound, T, P, pimpl->substanceOptions(), &steps, solvent ? &*solvent : nullptr);
    for (const auto& step : steps)
        out.push_back({step.method, step.values.G.val, step.values.H.val, step.values.S.val, step.values.Cp.val, step.values.V.val});
    return out;
}

auto ThermoEngine::parseSubstanceFormula(std::string formula) const -> std::map<Element, double>
{
    return pimpl->database.parseSubstanceFormula(formula);
}

EngineConventions& ThermoEngine::conventions() {
    return pimpl->conventions;
}

const EngineConventions& ThermoEngine::conventions() const {
    return pimpl->conventions;
}

} // namespace ThermoFun
