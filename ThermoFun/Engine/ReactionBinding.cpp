// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReactionBinding.h"

#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodRegistry.h>

namespace ThermoFun::v2 {

auto canBindReaction(const ReactionRecord& record) -> bool
{
    int bases = 0;
    for (const auto& a : record.methods)
    {
        const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Reaction);
        if (!spec || !MethodRegistry::instance().hasReactionMethod(a.method))
            return false;
        bases += spec->reaction_effects.base;
    }
    return bases == 1;
}

auto bindReaction(std::shared_ptr<const ReactionRecord> record) -> std::shared_ptr<const BoundReaction>
{
    auto bound = std::make_shared<BoundReaction>();
    const auto& registry = MethodRegistry::instance();
    for (const auto& a : record->methods)
    {
        auto method = registry.createReaction(a, *record);
        if (method->spec().reaction_effects.base)
            bound->base = method;
        else
            bound->volume = method;
        bound->needs_solvent = bound->needs_solvent || method->spec().needs.solvent;
    }
    bound->record = std::move(record);
    return bound;
}

auto executeReaction(const BoundReaction& bound, double T, double P, const PropertiesSolvent* solvent) -> ThermoPropertiesReaction
{
    auto tpr = bound.base->calculate(T, P, ThermoPropertiesReaction{}, solvent);
    if (bound.volume)
        tpr = bound.volume->calculate(T, P, tpr, solvent);
    return tpr;
}

auto reactionParameterDerivative(const BoundReaction& bound, const std::string& method, const std::string& parameter, double T, double P, const PropertiesSolvent* solvent) -> ThermoPropertiesReaction
{
    auto seedOf = [&](const ReactionMethod& m) { return method.empty() || method == m.spec().id ? parameter : std::string(); };
    auto derivative = bound.base->parameterDerivative(seedOf(*bound.base), T, P, ThermoPropertiesReaction{}, solvent);
    if (!bound.volume)
        return derivative;
    // the volume method works on the values of the base method, with their derivatives in ddt
    auto incoming = bound.base->calculate(T, P, ThermoPropertiesReaction{}, solvent);
    auto carry = [](Reaktoro_::ThermoProperty& to, const Reaktoro_::ThermoProperty& d) { to.ddt = d.val; to.ddp = 0.0; };
    carry(incoming.log_equilibrium_constant, derivative.log_equilibrium_constant);
    carry(incoming.ln_equilibrium_constant, derivative.ln_equilibrium_constant);
    carry(incoming.reaction_gibbs_energy, derivative.reaction_gibbs_energy);
    carry(incoming.reaction_enthalpy, derivative.reaction_enthalpy);
    carry(incoming.reaction_entropy, derivative.reaction_entropy);
    carry(incoming.reaction_heat_capacity_cp, derivative.reaction_heat_capacity_cp);
    carry(incoming.reaction_volume, derivative.reaction_volume);
    return bound.volume->parameterDerivative(seedOf(*bound.volume), T, P, incoming, solvent);
}

} // namespace ThermoFun::v2
