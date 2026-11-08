// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The engine inside: what ThermoEngine.cpp and the ThermoEngine*.cpp files share. Not installed.

#ifndef THERMOFUN_THERMOENGINEIMPL_H
#define THERMOFUN_THERMOENGINEIMPL_H

#include <ThermoFun/Model/BuiltinParameterSets.h>
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
#include "Engine/Uncertainty.h"
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

struct ThermoEngine::Impl
{
    /// The database instance
    Database database;

    mutable EnginePreferences preferences;

    /// The solvent that the calculations use: preferences.solvent_symbol if the database has it, else the one chooseSolvent
    /// finds. Chosen when the engine is made and when the database or a setting changes, never during a calculation, so a
    /// result does not depend on what was calculated before.
    mutable std::string solvent_in_use;

    mutable EngineConventions conventions;

    /// The source of the properties of single records (see setPropertySource). A substance and its defining reaction often have the same symbol: each has its own choice.
    std::map<std::string, v2::PropertySource> substance_sources, reaction_sources;

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
        std::map<const v2::SubstanceRecord*, std::shared_ptr<const v2::BoundSubstance>> corrections;
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
        if (const auto& dataset_solvent = this->database.thermoDataSet().solvent; !dataset_solvent.empty())
            preferences.solvent_symbol = dataset_solvent;
        resultsAreCurrent();
    }

    /// The solvent for preferences.solvent_symbol in this database
    auto chosenSolvent() const -> std::string
    {
        if (database.containsSubstance(preferences.solvent_symbol))
            return preferences.solvent_symbol;
        std::map<std::string, v2::SubstanceClass> classes;
        for (const auto& pair : database.mapSubstances())
            classes[pair.first] = pair.second.substanceClass() == SubstanceClass::type::AQSOLVENT ? v2::SubstanceClass::WaterSolvent : v2::SubstanceClass::Other;
        const auto chosen = v2::chooseSolvent(preferences.solvent_symbol, "", classes);
        return chosen.empty() ? preferences.solvent_symbol : chosen;
    }

    void resultsAreCurrent()
    {
        const Database& db = database;
        cached_for = {db.revision(), preferences, conventions, db.conventions()};
        solvent_in_use = chosenSolvent();
    }

    void clearResults()
    {
        substance_results.clear();
        electro_results.clear();
        solvent_results.clear();
        reaction_results.clear();
        users_are_current = false;
        // the methods are bound with the implementations of the preferences
        bound_cache = std::make_shared<BoundCache>();
        resultsAreCurrent();
    }

    /// The result of a calculation from the cache, else calculated and kept. A substance has one result at T and P, also as a reactant.
    template <class Value, class Calculate>
    auto cached(v2::ResultCache<Value>& cache, double T, double& P, const std::string& symbol, int state, Calculate calculate) const -> Value
    {
        if (!preferences.enable_memoize)
            return calculate();
        const v2::ResultKey key{symbol, T, P, state};
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
    auto sourceOf(const std::map<std::string, v2::PropertySource>& own, const std::string& symbol, const std::optional<v2::PropertySource>& preferred) const -> std::optional<v2::PropertySource>;

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
    auto steamTablesShift(ThermoPropertiesSubstance &tps, double T, double sign) const -> void;

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
        int state = 0;  ///< the state of water of the record: 0 liquid, 1 vapour
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

    /// True if the properties of a reaction change with P: from a model of log K, from a method of its volume with a volume that is not 0, or from its species
    auto reactionCarriesPressure(const Reaction& reaction) const -> bool;

    /// True if a substance is calculated from its defining reaction, by its record and the sources.
    auto comesFromReaction(const Substance& substance) const -> bool;

    /// True if the parameter is a built-in constant of its method. A derivative by a constant is a central difference of the whole calculation.
    auto isConstant(const ParameterRef& ref) const -> bool;

    /// True if a reaction is the sum of its reactants, by its record and the sources. If a reactant comes from this reaction there is a loop: a choice
    /// for this reaction is an error that names the loop, and a choice for all records leaves this one to its own methods, with a warning.
    auto reactionFromReactants(const Reaction& reaction) const -> bool;

    auto pathOfSubstance(const std::string& symbol, std::vector<std::string>& above) const -> CalculationPath;
    auto pathOfReaction(const std::string& symbol, std::vector<std::string>& above) const -> CalculationPath;

    /// The pressure of the log K data of a reaction at T: the largest of the reference pressure, the saturation pressure of water
    /// (the critical pressure above Tcr), and the pressure at which water has the density rho_min of DataPressure.yaml.
    /// So the species stay in dense water, and G is continuous at Tcr.
    auto dataPressure(double T, const Substance& substance) const -> double;
    /// The pressure at which the solvent has density rho (kg/m3) at T, at least P
    auto pressureAtDensity(double T, double rho, double P) const -> double;
    static auto waterCriticalT() -> double
    {
        static const double T = v2::builtinConstants("WaterSaturation")->parameters.at("Tcr").value;
        return T;
    }
    static auto waterCriticalP() -> double
    {
        static const double P = v2::builtinConstants("WaterSaturation")->parameters.at("Pcr").value;
        return P;
    }

    static auto hasBaseMethod(const v2::SubstanceRecord& record) -> bool;

    /// The methods of a substance for the pressure, apart from ConstantVolume, to run on what its defining reaction gives
    auto pressureCorrections(const Substance& substance) const -> std::shared_ptr<const v2::BoundSubstance>;

    /// The properties in the form of the methods: V in m3/mol
    static auto valuesOf(const ThermoPropertiesSubstance& tps) -> v2::MethodResult;

    /// True if a substance defined by this reaction takes its pressure term from its own method. Else there would be two of them.
    auto pressureFromRecord(const Substance& substance, const Reaction& reaction) const -> bool;

    auto thermoPropertiesReaction(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction;

    /// A record with a logK(T) function and a method of T correction that does not use it: the function and the reference values
    /// should give the same logK at the reference temperature. One warning for each reaction.
    auto warnIfLogKFunctionDisagrees(const Reaction &reaction) const -> void;

    auto thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction;

    auto thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction;

    auto thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction;

    //=================================================================================================
    // The derivatives by the parameters of the records
    //=================================================================================================

    auto checkParameter(const ParameterRef& ref) const -> void;

    auto seedOf(const v2::BoundSubstance& bound, const ParameterRef& ref) const -> v2::ParameterSeed;

    auto substanceDerivative(double T, double& P, const Substance& substance, const ParameterRef& ref) const -> v2::PropertyDerivatives;

    /// Each val holds a derivative, V in m3/mol per unit.
    auto reactionDerivative(double T, double& P, const Reaction& reaction, const ParameterRef& ref, bool alwaysFromReactants = false) const -> ThermoPropertiesReaction;

    //=================================================================================================
    // The handles on the parameters
    //=================================================================================================

    /// A change of a value of a record, in the database of this engine. Only the results that depend on it are cleared.
    template <class Edit>
    auto editValue(const ParameterRef& ref, Edit edit) -> void;

    auto isOfReaction(const ParameterRef& ref) const -> bool;
    auto valueOf(const ParameterRef& ref) const -> v2::Quantity;

    /// The substances and reactions whose results use this record, and itself
    auto dependentsOf(const std::string& symbol) const -> std::set<std::string>;

    /// Who uses each symbol, directly. Filled when needed; a change of a value does not change it.
    mutable std::map<std::string, std::set<std::string>> users;
    mutable bool users_are_current = false;

    //=================================================================================================
    // Propagation of the errors of the reference properties and of the coefficients of the models
    //=================================================================================================

    static auto fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>;
    static auto fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>;

    /// The records (substances and reactions of the database) a calculation depends on
    struct Records
    {
        std::set<std::string> substances, reactions;
    };

    auto collectSubstance(Records& r, const std::string& symbol) const -> void;

    auto collectReaction(Records& r, const std::string& symbol) const -> void;

    /// The uncertain values of these records, and of a record that is not in the database
    auto inputsOf(const Records& records, const Substance* given = nullptr, const Reaction* given_reaction = nullptr) const -> v2::UncertainInputs;

    /// The derivatives of all the properties of a substance by one value, in the order of fields: V in J/bar. tps: the properties at T, P.
    auto substanceJacobian(double T, double& P, const Substance& substance, const ThermoPropertiesSubstance& tps, const ParameterRef& ref) const -> std::vector<double>;

    /// The same for a reaction
    auto reactionJacobian(double T, double& P, const Reaction& reaction, const ParameterRef& ref, bool fromReactants) const -> std::vector<double>;

    /// The errors of the results from all the uncertain values: u^2 = J S J^T (first order; S is diagonal, apart from the covariance blocks of the records).
    auto addSubstanceErrors(ThermoPropertiesSubstance& tps, double T, double& P, const Substance& substance, const v2::UncertainInputs& inputs) const -> void;
    auto addReactionErrors(ThermoPropertiesReaction& tpr, double T, double& P, const Reaction& reaction, bool fromReactants, const v2::UncertainInputs& inputs) const -> void;

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
