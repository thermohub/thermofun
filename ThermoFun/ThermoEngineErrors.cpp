// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The propagation of the errors of the parameters to the results, and their rounding.

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

auto ThermoEngine::Impl::perturbedEngine() const -> ThermoEngine&
{
    if (!perturbed_engine)
        perturbed_engine.reset(new ThermoEngine(database));
    auto& impl = *perturbed_engine->pimpl;
    impl.preferences = preferences;
    impl.preferences.enable_memoize = false;
    impl.preferences.propagate_parameter_errors = false;
    impl.preferences.round_to_uncertainty = false;
    impl.conventions = conventions;
    impl.property_sources = property_sources;
    return *perturbed_engine;
}

auto ThermoEngine::Impl::fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.gibbs_energy, &p.helmholtz_energy, &p.internal_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp, &p.heat_capacity_cv};
}

auto ThermoEngine::Impl::fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.reaction_gibbs_energy, &p.reaction_helmholtz_energy, &p.reaction_internal_energy, &p.reaction_enthalpy, &p.reaction_entropy,
            &p.reaction_volume, &p.reaction_heat_capacity_cp, &p.reaction_heat_capacity_cv, &p.ln_equilibrium_constant, &p.log_equilibrium_constant};
}

auto ThermoEngine::Impl::referenceFields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.gibbs_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp};
}

auto ThermoEngine::Impl::referenceFields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.log_equilibrium_constant, &p.reaction_gibbs_energy, &p.reaction_enthalpy, &p.reaction_entropy, &p.reaction_volume, &p.reaction_heat_capacity_cp};
}

auto ThermoEngine::Impl::collectSubstance(Records& r, const std::string& symbol) const -> void
{
    if (r.substances.count(symbol) || !database.containsSubstance(symbol)) return;
    r.substances.insert(symbol);
    const auto reaction = database.getSubstance(symbol).reactionSymbol();
    if (!reaction.empty()) collectReaction(r, reaction);
}

auto ThermoEngine::Impl::collectReaction(Records& r, const std::string& symbol) const -> void
{
    if (r.reactions.count(symbol) || !database.containsReaction(symbol)) return;
    r.reactions.insert(symbol);
    for (const auto& reactant : database.getReaction(symbol).reactants())
        collectSubstance(r, reactant.first);
}

auto ThermoEngine::Impl::recordsOfSubstance(const std::string& symbol) const -> Records
{
    Records r; collectSubstance(r, symbol); return r;
}

auto ThermoEngine::Impl::recordsOfReaction(const std::string& symbol) const -> Records
{
    Records r; collectReaction(r, symbol); return r;
}

auto ThermoEngine::Impl::recordsOfReactants(const Reaction& reaction) const -> Records
{
    Records r;
    for (const auto& reactant : reaction.reactants()) collectSubstance(r, reactant.first);
    return r;
}

} // namespace ThermoFun
