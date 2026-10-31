// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The engine inside: what ThermoEngine.cpp and the ThermoEngine*.cpp files share. Not installed.

#ifndef THERMOFUN_THERMOENGINEIMPL_H
#define THERMOFUN_THERMOENGINEIMPL_H

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
#include "Common/Sha256.h"
#include "Conventions/ConventionConverter.h"
#include "Conventions/HydrogenIon.h"
#include "Model/SubstanceRecord.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Reaction.h"
#include "Substance.h"
#include "Element.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/Rounding.hpp"
#include "Engine/ResultCache.h"
#include "Methods/MethodCatalog.h"
#include "Methods/MethodRegistry.h"
#include "Methods/Reactions/ReactionMethods.h"
#include "Methods/Water/WaterIdealGasWolley.h"
#include <stdexcept>
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

struct ThermoEngine::Impl
{
    /// The database instance
    Database database;

    mutable EnginePreferences preferences;

    mutable EngineConventions conventions;

    /// The source of the properties of single records (see setPropertySource)
    std::map<std::string, v2::PropertySource> property_sources;

    /// The results of the calculations (see enable_memoize)
    mutable v2::ResultCache<ThermoPropertiesSubstance> substance_results;
    mutable v2::ResultCache<ElectroPropertiesSolvent> electro_results;
    mutable v2::ResultCache<PropertiesSolvent> solvent_results;
    mutable v2::ResultCache<ThermoPropertiesReaction> reaction_results;

    /// The database revision and the settings with which the results were calculated
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
        std::mutex entropy_mutex;
        std::map<std::string, double> element_entropies;   ///< by formula
    };
    std::shared_ptr<BoundCache> bound_cache = std::make_shared<BoundCache>();

    Impl()
    {
        resultsAreCurrent();
    }

    Impl(const Database &database)
        : database(database)
    {
        // the solvent of the dataset
        if (const auto& solvent = this->database.thermoDataSet().solvent; !solvent.empty())
            preferences.solvent_symbol = solvent;
        resultsAreCurrent();
    }

    void resultsAreCurrent()
    {
        const Database& db = database;
        cached_for = {db.revision(), preferences, conventions, db.conventions()};
    }

    void clearResults()
    {
        substance_results.clear();
        electro_results.clear();
        solvent_results.clear();
        reaction_results.clear();
        perturbed_engine.reset();
        // the methods are bound with the implementations of the preferences
        bound_cache = std::make_shared<BoundCache>();
        resultsAreCurrent();
    }

    /// The result of a calculation from the cache, else calculated and kept. The reactants of a reaction-defined substance and of
    /// a reaction from reactants are calculated with the pressure correction of gases switched on (FlagGuard): the key holds the flag,
    /// so a result of a gas does not depend on the calculations made before it.
    template <class Value, class Calculate>
    auto cached(v2::ResultCache<Value>& cache, double T, double& P, const std::string& symbol, int state, Calculate calculate) const -> Value
    {
        if (!preferences.enable_memoize)
            return calculate();
        const v2::ResultKey key{symbol, T, P, state, preferences.apply_pressure_correction_to_gas_props};
        if (auto hit = cache.find(key, P))
            return *hit;
        const Value value = calculate();
        cache.store(key, value, P, preferences.max_cache_size);
        return value;
    }

    auto cachedSubstance(double T, double& P, const std::string& symbol) const -> ThermoPropertiesSubstance
    {
        return cached(substance_results, T, P, symbol, -1, [&] { return thermoPropertiesSubstance(T, P, symbol); });
    }

    auto cachedElectro(double T, double& P, const std::string& symbol, int state) const -> ElectroPropertiesSolvent
    {
        return cached(electro_results, T, P, symbol, state, [&] { return electroPropertiesSolvent(T, P, symbol, state); });
    }

    auto cachedSolvent(double T, double& P, const std::string& symbol, int state) const -> PropertiesSolvent
    {
        return cached(solvent_results, T, P, symbol, state, [&] { return propertiesSolvent(T, P, symbol, state); });
    }

    auto cachedReaction(double T, double& P, const std::string& symbol) const -> ThermoPropertiesReaction
    {
        return cached(reaction_results, T, P, symbol, -1, [&] { return thermoPropertiesReaction(T, P, symbol); });
    }

    /// Clear the cached results if the records, the preferences or the conventions changed after they were cached.
    /// Called at the start of each calculation, so a change in place (preferences(), conventions(),
    /// Database::substance(), ...) can never give a stale result.
    void dropStaleResults();

    /// Where the properties of a record come from: the source of the symbol, else the preference of the engine, else the
    /// record. Empty: the rule of the record.
    auto sourceOf(const std::string& symbol, const std::optional<v2::PropertySource>& preferred) const -> std::optional<v2::PropertySource>;

    /// A record with the implementations of the preferences
    auto withImplementations(std::shared_ptr<const v2::SubstanceRecord> record) const -> std::shared_ptr<const v2::SubstanceRecord>;

    /// The sum of the entropies of the elements of a formula, for a change of the energy reference. A value that a record of the
    /// database does not give comes from the element table of ChemicalFun, with a warning.
    auto elementEntropy(const std::string& formula) const -> double;

    /// What a conversion needs to know about a record
    auto subjectOf(const v2::SubstanceRecord& record) const -> v2::ConversionSubject;

    /// The convention that the engine returns
    auto outputConvention() const -> v2::Convention;

    /// The convention of the values of a record: its own, else that of the dataset
    auto declaredConvention(const v2::SubstanceRecord& record) const -> v2::Convention;

    /// The methods of a substance. Null if they give no one base method: then no method runs.
    auto boundMethods(const Substance& substance) const -> std::shared_ptr<const v2::BoundSubstance>;

    /// The state of the solvent that the methods of a record need, calculated with the methods of the solvent record.
    /// A pressure of 0 is the saturation pressure: it is written to P.
    auto solventInputs(const v2::BoundSubstance& bound, double T, double& P) const -> std::optional<v2::SolventInputs>;

    /// The methods of a reaction. Null if they give no one base method: then no method runs.
    auto boundReaction(const Reaction& reaction) const -> std::shared_ptr<const v2::BoundReaction>;

    /// The methods of a water record.
    auto boundSolvent(const Substance& substance) const -> std::shared_ptr<const v2::BoundSolvent>;

    auto substanceOptions() const -> v2::SubstanceOptions;

    /// The result of the executor in the form of the old engine: V in J/bar, and the status of each property
    static auto toThermoProperties(const v2::SubstanceResult& r) -> ThermoPropertiesSubstance;

    /// Water from the geochemical reference to the steam tables (sign -1), and back (sign 1)
    auto steamTablesShift(ThermoPropertiesSubstance &tps, double sign) const -> void;

    static auto add(Reaktoro_::ThermoProperty& to, double factor, const Reaktoro_::ThermoScalar& term) -> void
    {
        to.val += factor * term.val;
        to.ddt += factor * term.ddt;
        to.ddp += factor * term.ddp;
    }

    static auto defined(const Reaktoro_::ThermoProperty& x) -> bool { return x.sta.first != Reaktoro_::Status::notdefined; }

    /// The density terms of a change between molarity and molality at T, P. V in J/bar.
    auto molarityTermsAt(double T, double& P, const std::string& what) const -> v2::MolarityTerms<Reaktoro_::ThermoScalar>;

    /// A change of convention of the properties of a substance
    auto convert(ThermoPropertiesSubstance &tps, const v2::ConventionShift& shift, double T, double& P, const std::string& symbol) const -> void;

    /// The properties in the convention that the engine returns. Inside the engine all values are canonical.
    auto toOutput(ThermoPropertiesSubstance tps, double T, double& P, const Substance& substance, DetailedPropertiesSubstance* detailed = nullptr) const -> ThermoPropertiesSubstance;

    /// The reference properties of a substance in the canonical convention. They are the values at the reference temperature.
    auto canonicalReference(const Substance& substance) const -> ThermoPropertiesSubstance;

    /// The change of a reaction for a change of convention of its species: the sum over the species. A balanced reaction has no change for the energy
    /// reference: the standard pressure and the concentration scale are what count.
    auto reactionShift(const Reaction& reaction, const v2::Convention& convention, bool toCanonical) const -> v2::ConventionShift;

    /// A change of convention of the properties of a reaction
    auto convert(ThermoPropertiesReaction &tpr, const v2::ConventionShift& shift, double T, double& P, const std::string& symbol) const -> void;

    auto toOutput(ThermoPropertiesReaction tpr, double T, double& P, const Reaction& reaction) const -> ThermoPropertiesReaction;

    auto reactionOfSymbol(const std::string& symbol) const -> Reaction;

    /// The properties that follow from the others (exact derivatives and propagated errors), for the pressure P (Pa) and the temperature T (K):
    ///   U = H - P V,  A = U - T S,  Cv = Cp - T V alpha^2/beta = Cp - T V_T^2/(-V_P)
    /// with V in J/bar (V_T, V_P: the derivatives of V). The derivatives of Cv are those of Cp (the second derivatives of the volume
    /// are not available): they are exact when the volume does not depend on T (Cv = Cp). Cv of the solvent is the one of its model.
    auto completeThermoProperties(ThermoPropertiesSubstance &tps, double T, double P, bool calculateCv) const -> void;

    /// What the record says about the way to calculate a substance
    struct SubstanceKind
    {
        bool isHydrogen = false;
        bool isH2Ovapor = false;
        bool isH2OSolvent = false;
        bool isReacDC = false;
    };

    auto kindOf(const Substance &substance) const -> SubstanceKind;

    static auto hasMethod(const std::vector<v2::MethodApplication>& methods, const char* id) -> bool
    {
        return std::any_of(methods.begin(), methods.end(), [&](const auto& a) { return a.method == id; });
    }

    auto thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance;

    auto detailedSubstance(double T, double &P, const Substance &substance) const -> DetailedPropertiesSubstance;

    /// The properties in the canonical convention. toOutput gives them in the convention that the user asks for; reactions use these.
    auto thermoPropertiesSubstance(double T, double &P, const Substance &substance, DetailedPropertiesSubstance* detailed = nullptr) const -> ThermoPropertiesSubstance;

    auto fallbackThermoPropertiesSubstance(ThermoPropertiesSubstance tps, Substance subst) const -> ThermoPropertiesSubstance;

    auto electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent;

    auto electroPropertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> ElectroPropertiesSolvent;

    auto propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent;

    auto propertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> PropertiesSolvent;

    auto reacDCthermoProperties(double T, double &P, Substance subst) const -> ThermoPropertiesSubstance;

    auto thermoPropertiesReaction(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction;

    /// A record with a logK(T) function and a method of T correction that does not use it: the function and the reference values
    /// should give the same logK at the reference temperature. One warning for each reaction.
    auto warnIfLogKFunctionDisagrees(const Reaction &reaction) const -> void;

    auto thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction;

    auto thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction;

    auto thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction;

    //=================================================================================================
    // Propagation of the errors of the reference properties and of the coefficients of the models
    //=================================================================================================

    /// A copy of the engine used to calculate with perturbed parameters (without memoization and error propagation)
    mutable std::shared_ptr<ThermoEngine> perturbed_engine;

    auto perturbedEngine() const -> ThermoEngine&;

    static auto fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>;
    static auto fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>;
    static auto referenceFields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>;
    static auto referenceFields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>;

    /// The records (substances and reactions of the database) a calculation depends on
    struct Records
    {
        std::set<std::string> substances, reactions;
    };

    auto collectSubstance(Records& r, const std::string& symbol) const -> void;

    auto collectReaction(Records& r, const std::string& symbol) const -> void;

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
    auto addParameterErrors(Props& result, const Records& records, const Substance* givenS, const Reaction* givenR, Eval eval) const -> void;

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

    auto recordsOfSubstance(const std::string& symbol) const -> Records;
    auto recordsOfReaction(const std::string& symbol) const -> Records;
    auto recordsOfReactants(const Reaction& reaction) const -> Records;
};

} // namespace ThermoFun

#endif
