// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The properties of reactions, in the canonical convention.

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

/// The methods of a reaction. Null if they give no one base method: then no method runs.
auto ThermoEngine::Impl::boundReaction(const Reaction& reaction) const -> std::shared_ptr<const v2::BoundReaction>
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
    const auto fn = v2::logKProperties(inputs, LogKForm::Polynomial, Tr, reaction.referenceP());
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

auto ThermoEngine::Impl::thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction
{
    // from the reactants: a record without methods, unless the source says otherwise. "reaction" is for substances only.
    const auto record = reaction.record();
    const auto source = sourceOf(record->symbol, record->preferred_property_source);
    bool fromReactants = record->methods.empty();
    if (source == v2::PropertySource::Reactants && !record->stoichiometry.empty())
        fromReactants = true;
    if (fromReactants)
        return thermoPropertiesReactionFromReactants(T, P, reaction);

    if (const auto bound = boundReaction(reaction))
    {
        if (bound->base->spec().id != "LogKPolynomial")
            warnIfLogKFunctionDisagrees(reaction);
        std::optional<PropertiesSolvent> solvent;
        if (bound->needs_solvent)
            solvent = cachedSolvent(T, P, preferences.solvent_symbol, -1);
        auto tpr = v2::executeReaction(*bound, T, P, solvent ? &*solvent : nullptr);
        // the species of the record in another standard state: to the canonical one
        const auto shift = reactionShift(reaction, v2::mergeConvention(record->convention.value_or(v2::Convention{}), database.thermoDataSet().convention), true);
        if (!shift.identity())
            convert(tpr, shift, T, P, reaction.symbol());
        return tpr;
    }
    return {};
}

auto ThermoEngine::Impl::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    return thermoPropertiesReactionFromReactants(T, P, database.getReaction(symbol));
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

        components.push_back({cachedSubstance(T, P, substance) /*thermoPropertiesSubstance(T, P, substance);*/, coeff});
        symbols.push_back(substance);
    }

    tpr = v2::reactionFromReactants(T, P, reaction.symbol(), components, symbols);

    gas_pressure_correction.release();
    return tpr;
}

} // namespace ThermoFun
