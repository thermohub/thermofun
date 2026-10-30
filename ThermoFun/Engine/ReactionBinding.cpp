// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReactionBinding.h"

#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodRegistry.h>

namespace ThermoFun::v2 {

auto canBindReaction(const ReactionRecord& record) -> bool
{
    if (record.preferred_property_source == PropertySource::Reactants)
        return false;
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

} // namespace ThermoFun::v2
