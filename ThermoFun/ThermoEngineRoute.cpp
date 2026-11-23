// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The routes of the results (decision 0030): the helpers of the calculations, and the route of a record without numbers.

#include <algorithm>
#include <cstdio>

#include "ThermoEngineImpl.h"

namespace ThermoFun {

namespace {

auto loopFrom(const std::vector<std::string>& above, const std::string& symbol) -> std::string
{
    std::string out;
    for (auto it = std::find(above.begin(), above.end(), symbol); it != above.end(); ++it)
        out += *it + " -> ";
    return "A cycle of defining reactions: " + out + symbol;
}

auto problem(Route& route, const std::string& text) -> void
{
    route.notes.push_back(Note{Note::Kind::Undefined, Note::Severity::Hard, "", text, {}, {}});
}

auto number(double x) -> std::string
{
    char buf[32];
    std::snprintf(buf, sizeof buf, "%g", x);
    return buf;
}

/// A region as text, for example "T 273.15-1273.15 K, P 0-5e+08 Pa"; empty for no limit
auto regionText(const model::Region& r) -> std::string
{
    if (r.empty())
        return {};
    std::string out;
    if (r.t_min || r.t_max)
        out = "T " + (r.t_min ? number(*r.t_min) : std::string("0")) + "-" + (r.t_max ? number(*r.t_max) : std::string("inf")) + " K";
    if (r.p_min || r.p_max)
        out += (out.empty() ? "" : ", ") + std::string("P ") + (r.p_min ? number(*r.p_min) : std::string("0")) + "-" + (r.p_max ? number(*r.p_max) : std::string("inf")) + " Pa";
    return out;
}

} // namespace

auto ThermoEngine::Impl::routeValues(const ThermoPropertiesSubstance& tps, double volume) -> RouteValues
{
    return {tps.gibbs_energy.val, tps.enthalpy.val, tps.entropy.val, tps.heat_capacity_cp.val, tps.volume.val * volume};
}

auto ThermoEngine::Impl::routeValues(const ThermoPropertiesReaction& tpr, double volume) -> RouteValues
{
    return {tpr.reaction_gibbs_energy.val, tpr.reaction_enthalpy.val, tpr.reaction_entropy.val, tpr.reaction_heat_capacity_cp.val, tpr.reaction_volume.val * volume};
}

auto ThermoEngine::Impl::waterMethods(const model::SubstanceRecord& record, double T, double P) const -> std::vector<RouteMethod>
{
    const bool all = std::isnan(T);
    const auto implementationOf = [&](const model::MethodApplication& a) {
        const auto chosen = preferences.method_implementation.find(a.method);
        return chosen != preferences.method_implementation.end() ? chosen->second : a.implementation;
    };
    // the methods of each kind (the equation of state, the dielectric model) in record order: the first whose region holds T, P
    std::vector<RouteMethod> out;
    std::vector<const model::MethodApplication*> group[2];
    for (const auto& a : record.methods)
    {
        const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance);
        if (!spec)
            continue;
        if (all || !model::combinableByRegion(*spec))
        {
            out.push_back({a.method, implementationOf(a), regionText(a.region), {}, {}, {}});
            continue;
        }
        group[spec->contributes_to.base ? 0 : 1].push_back(&a);
    }
    for (const auto& methods : group)
    {
        if (methods.empty())
            continue;
        const model::MethodApplication* used = methods.back();
        for (const auto* a : methods)
            if (a->region.holds(T, P))
            {
                used = a;
                break;
            }
        out.push_back({used->method, implementationOf(*used), methods.size() > 1 ? regionText(used->region) : std::string(), {}, {}, {}});
    }
    return out;
}

auto ThermoEngine::Impl::staticRoute(const std::string& symbol, bool reaction) const -> std::shared_ptr<const Route>
{
    const std::pair<std::string, bool> key{symbol, reaction};
    {
        std::lock_guard<std::mutex> lock(bound_cache->mutex);
        if (const auto it = bound_cache->routes.find(key); it != bound_cache->routes.end())
            return it->second;
    }
    std::vector<std::string> above;
    std::shared_ptr<const Route> built = reaction ? routeOfReaction(symbol, above) : routeOfSubstance(symbol, above);
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    return bound_cache->routes.try_emplace(key, std::move(built)).first->second;
}

auto ThermoEngine::Impl::routeOfReaction(const std::string& symbol, std::vector<std::string>& above) const -> std::shared_ptr<Route>
{
    auto route = std::make_shared<Route>();
    route->kind = "reaction";
    route->symbol = symbol;
    if (!database.containsReaction(symbol))
    {
        problem(*route, "There is no such reaction.");
        return route;
    }
    if (std::find(above.begin(), above.end(), symbol) != above.end())
    {
        problem(*route, loopFrom(above, symbol));
        return route;
    }
    const auto reaction = database.getReaction(symbol);
    above.push_back(symbol);
    try
    {
        if (reactionFromReactants(reaction))
        {
            route->by = "reactants";
            for (const auto& [reactant, coefficient] : reaction.reactants())
                route->needs.push_back({"reactant", coefficient, routeOfSubstance(reactant, above)});
        }
        else
        {
            route->by = "methods";
            for (const auto& a : reaction.record()->methods)
            {
                const auto chosen = preferences.method_implementation.find(a.method);
                route->methods.push_back({a.method, chosen != preferences.method_implementation.end() ? chosen->second : a.implementation, "", {}, {}, {}});
            }
            for (const auto& a : reaction.record()->methods)
                if (const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Reaction); spec && spec->needs.solvent)
                {
                    route->needs.push_back({"solvent", 1.0, routeOfSubstance(solvent_in_use, above)});
                    break;
                }
        }
    }
    catch (const std::exception& e)
    {
        problem(*route, e.what());
    }
    above.pop_back();
    return route;
}

auto ThermoEngine::Impl::routeOfSubstance(const std::string& symbol, std::vector<std::string>& above) const -> std::shared_ptr<Route>
{
    auto route = std::make_shared<Route>();
    route->kind = "substance";
    route->symbol = symbol;
    if (!database.containsSubstance(symbol))
    {
        problem(*route, "There is no such substance.");
        return route;
    }
    if (std::find(above.begin(), above.end(), symbol) != above.end())
    {
        problem(*route, loopFrom(above, symbol));
        return route;
    }
    const auto substance = database.getSubstance(symbol);
    const auto record = substance.record();
    const auto kind = kindOf(substance);
    if (kind.isHydrogen)
    {
        route->by = "hydrogen_ion";
        return route;
    }
    if (kind.isElectron)
        if (const auto gas = hydrogenGas(); !gas.empty())
        {
            route->by = "hydrogen_electrode";
            above.push_back(symbol);
            route->needs.push_back({"reactant", 0.5, routeOfSubstance(gas, above)});
            above.pop_back();
            return route;
        }
    above.push_back(symbol);
    const bool isWater = kind.isH2OSolvent || kind.isH2Ovapor;
    const auto methodsOfRecord = [&] {
        for (const auto& a : withImplementations(record)->methods)
        {
            const auto chosen = preferences.method_implementation.find(a.method);
            route->methods.push_back({a.method, chosen != preferences.method_implementation.end() ? chosen->second : a.implementation, regionText(a.region), {}, {}, {}});
        }
    };
    if (kind.isReacDC && !substance.reactionSymbol().empty())
    {
        route->by = "reaction";
        const auto& name = substance.reactionSymbol();
        route->needs.push_back({"defining_reaction", 1.0, routeOfReaction(name, above)});
        if (database.containsReaction(name))
            for (const auto& [reactant, coefficient] : database.getReaction(name).reactants())
                if (reactant != symbol)
                    route->needs.push_back({"reactant", coefficient, routeOfSubstance(reactant, above)});
        // the methods for the pressure that run on what the reaction gives
        for (const auto& a : record->methods)
            if (const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance);
                spec && !spec->contributes_to.base && (spec->depends_on == model::Dependence::P || spec->depends_on == model::Dependence::TP))
                route->methods.push_back({a.method, a.implementation, "", {}, {}, {}});
    }
    else if (kind.isReacDC)
        route->by = "reference";
    else if (isWater)
    {
        const auto solvent = boundSolvent(substance);
        route->by = solvent->equation_of_state ? "water" : "reference";
        route->methods = waterMethods(*record, std::numeric_limits<double>::quiet_NaN(), 0.0);
    }
    else
    {
        route->by = "methods";
        try
        {
            if (const auto bound = boundMethods(substance))
            {
                // in the order they run, as on the route of a result
                for (const auto& method : bound->order)
                {
                    RouteMethod m;
                    m.method = method->spec().id;
                    for (const auto& a : bound->record->methods)
                        if (a.method == m.method)
                        {
                            m.implementation = a.implementation;
                            break;
                        }
                    route->methods.push_back(std::move(m));
                }
                if (bound->needs.solvent)
                    route->needs.push_back({"solvent", 1.0, routeOfSubstance(solvent_in_use, above)});
            }
            else
            {
                methodsOfRecord();
                problem(*route, record->methods.empty() ? "The record has no methods and no defining reaction." : "The methods of the record give no one base method.");
            }
        }
        catch (const std::exception& e)
        {
            methodsOfRecord();
            problem(*route, e.what());
        }
    }
    above.pop_back();
    return route;
}

} // namespace ThermoFun
