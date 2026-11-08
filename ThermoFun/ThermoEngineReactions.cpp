// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The properties of reactions, in the canonical convention.

#include <mutex>
#include <set>
#include "ThermoEngineImpl.h"

namespace ThermoFun
{

/// The methods of a reaction. Null if they give no one base method: then no method runs.
auto ThermoEngine::Impl::boundReaction(const Reaction& reaction) const -> std::shared_ptr<const model::BoundReaction>
{
    const auto record = reaction.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->reactions.find(record.get());
    if (it != bound_cache->reactions.end())
        return it->second;
    auto bound = model::canBindReaction(*record, &database.methods()) ? model::bindReaction(model::withParameterSets(record, database.parameterSets()), &database.methods()) : nullptr;
    bound_cache->reactions[record.get()] = bound;
    bound_cache->reaction_records.push_back(record);
    return bound;
}

auto ThermoEngine::Impl::thermoPropertiesReaction(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
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
auto ThermoEngine::Impl::warnIfLogKFunctionDisagrees(const Reaction &reaction) const -> void
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
    ReactionInputs inputs;
    inputs.reference = ref;
    inputs.Tr = Tr;
    inputs.Pr = reaction.referenceP();
    const auto& logK = reaction.thermoParameters().reaction_logK_fT_coeff;
    inputs.coefficients.assign(logK.begin(), logK.end());
    const auto& cp = reaction.thermoParameters().reaction_Cp_fT_coeff;
    inputs.cp_coefficients.assign(cp.begin(), cp.end());
    const auto fn = model::logKProperties(inputs, LogKForm::Polynomial, Tr, reaction.referenceP());
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

auto ThermoEngine::Impl::reactionFromReactants(const Reaction& reaction) const -> bool
{
    const auto record = reaction.record();
    const auto source = sourceOf(reaction_sources, record->symbol, record->preferred_property_source);
    if (!record->methods.empty() && (source != model::PropertySource::Reactants || record->stoichiometry.empty()))
        return false;
    for (const auto& reactant : reaction.reactants())
    {
        if (!database.containsSubstance(reactant.first))
            continue;
        const auto substance = database.getSubstance(reactant.first);
        if (substance.reactionSymbol() != reaction.symbol() || !comesFromReaction(substance))
            continue;
        const std::string loop = reactant.first + " -> " + reaction.symbol() + " -> " + reactant.first;
        const bool forThisReaction = reaction_sources.count(record->symbol) || record->preferred_property_source == model::PropertySource::Reactants;
        if (record->methods.empty() || forThisReaction)
            funError("Cannot calculate the reaction " + reaction.symbol() + " from its reactants",
                     "A cycle of defining reactions: " + loop + ". Give " + reactant.first + " a base method, or give the reaction a method and use it.", __LINE__, __FILE__);
        static std::mutex mutex;
        static std::set<std::string> warned;
        const std::lock_guard<std::mutex> lock(mutex);
        if (warned.insert(reaction.symbol()).second)
            thfun_logger->warn("The reaction {} keeps its own methods: from its reactants would be a cycle of defining reactions, {}.", reaction.symbol(), loop);
        return false;
    }
    return true;
}

auto ThermoEngine::Impl::thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction
{
    // from the reactants: a record without methods, unless the source says otherwise. "reaction" is for substances only.
    const auto record = reaction.record();
    if (reactionFromReactants(reaction))
        return thermoPropertiesReactionFromReactants(T, P, reaction);

    if (const auto bound = boundReaction(reaction))
    {
        if (bound->base->spec().id != "LogKPolynomial")
            warnIfLogKFunctionDisagrees(reaction);
        std::optional<PropertiesSolvent> solvent;
        if (bound->needs_solvent)
            solvent = cachedSolvent(T, P, solvent_in_use, -1);
        else if (P == 0.0)
            P = saturationPressure(T);  // P = 0 is the saturation pressure of water also for a reaction that does not use water
        auto tpr = model::executeReaction(*bound, T, P, solvent ? &*solvent : nullptr);
        // the species of the record in another standard state: to the canonical one
        const auto shift = reactionShift(reaction, model::mergeConvention(record->convention.value_or(model::Convention{}), database.thermoDataSet().convention), true);
        if (!shift.identity())
            convert(tpr, shift, T, P, reaction.symbol());
        return tpr;
    }
    return {};
}

auto ThermoEngine::Impl::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    return thermoPropertiesReactionFromReactants(T, P, reactionOfSymbol(symbol));
}

auto ThermoEngine::Impl::thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
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

    // the properties of the reactants
    std::vector<std::pair<ThermoPropertiesSubstance, double>> components;
    std::vector<std::string> symbols;

    for (auto &reactant : reaction.reactants())
    {
        auto coeff      = reactant.second;
        auto substance  = reactant.first;
        auto s = database.getSubstance(substance);

        components.push_back({cachedSubstance(T, P, substance) /*thermoPropertiesSubstance(T, P, substance);*/, coeff});
        symbols.push_back(substance);
    }

    tpr = model::reactionFromReactants(T, P, reaction.symbol(), components, symbols);

    return tpr;
}

} // namespace ThermoFun
