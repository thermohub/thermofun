// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <algorithm>

#include "ThermoEngineImpl.h"

namespace ThermoFun {

namespace {

auto methodsOf(const std::vector<v2::MethodApplication>& methods, const std::map<std::string, std::string>& implementations) -> std::vector<std::string>
{
    std::vector<std::string> names;
    for (const auto& a : methods)
    {
        // the preferences replace the implementation of a record
        const auto chosen = implementations.find(a.method);
        const std::string& implementation = chosen != implementations.end() ? chosen->second : a.implementation;
        names.push_back(implementation.empty() ? a.method : a.method + "/" + implementation);
    }
    return names;
}

auto loopFrom(const std::vector<std::string>& above, const std::string& symbol) -> std::string
{
    std::string out;
    for (auto it = std::find(above.begin(), above.end(), symbol); it != above.end(); ++it)
        out += *it + " -> ";
    return "A cycle of defining reactions: " + out + symbol;
}

} // namespace

auto ThermoEngine::Impl::pathOfReaction(const std::string& symbol, std::vector<std::string>& above) const -> CalculationPath
{
    CalculationPath path;
    path.kind = "reaction";
    path.symbol = symbol;
    if (!database.containsReaction(symbol))
    {
        path.problem = "There is no such reaction.";
        return path;
    }
    if (std::find(above.begin(), above.end(), symbol) != above.end())
    {
        path.problem = loopFrom(above, symbol);
        return path;
    }
    const auto reaction = database.getReaction(symbol);
    path.methods = methodsOf(reaction.record()->methods, preferences.method_implementation);
    above.push_back(symbol);
    try
    {
        if (reactionFromReactants(reaction))
        {
            path.source = "reactants";
            for (const auto& [reactant, coefficient] : reaction.reactants())
            {
                path.steps.push_back(pathOfSubstance(reactant, above));
                path.steps.back().coefficient = coefficient;
                path.steps.back().role = "reactant";
            }
        }
        else
        {
            path.source = "methods";
            for (const auto& a : reaction.record()->methods)
                if (const auto* spec = v2::MethodCatalog::instance().find(a.method, v2::OperatesOn::Reaction); spec && spec->needs.solvent)
                {
                    path.steps.push_back(pathOfSubstance(preferences.solvent_symbol, above));
                    path.steps.back().role = "solvent";
                    break;
                }
        }
    }
    catch (const std::exception& e)
    {
        path.problem = e.what();
    }
    above.pop_back();
    return path;
}

auto ThermoEngine::Impl::pathOfSubstance(const std::string& symbol, std::vector<std::string>& above) const -> CalculationPath
{
    CalculationPath path;
    path.kind = "substance";
    path.symbol = symbol;
    if (!database.containsSubstance(symbol))
    {
        path.problem = "There is no such substance.";
        return path;
    }
    if (std::find(above.begin(), above.end(), symbol) != above.end())
    {
        path.problem = loopFrom(above, symbol);
        return path;
    }
    const auto substance = database.getSubstance(symbol);
    const auto record = substance.record();
    path.methods = methodsOf(record->methods, preferences.method_implementation);
    if (v2::isZeroHydrogenIon(*record, database.thermoDataSet().convention))
    {
        path.source = "hydrogen_ion";
        return path;
    }
    above.push_back(symbol);
    const bool isWater = record->class_ == v2::SubstanceClass::WaterSolvent;
    if (comesFromReaction(substance) && !substance.reactionSymbol().empty())
    {
        path.source = "reaction";
        const auto& name = substance.reactionSymbol();
        path.steps.push_back(pathOfReaction(name, above));
        path.steps.back().role = "defining_reaction";
        if (database.containsReaction(name))
            for (const auto& [reactant, coefficient] : database.getReaction(name).reactants())
                if (reactant != symbol)
                {
                    path.steps.push_back(pathOfSubstance(reactant, above));
                    path.steps.back().coefficient = coefficient;
                    path.steps.back().role = "reactant";
                }
    }
    else if (comesFromReaction(substance))
        path.source = "reference";
    else
    {
        path.source = isWater ? "water" : "methods";
        for (const auto& a : record->methods)
            if (const auto* spec = v2::MethodCatalog::instance().find(a.method, v2::OperatesOn::Substance); !isWater && spec && spec->needs.solvent)
            {
                path.steps.push_back(pathOfSubstance(preferences.solvent_symbol, above));
                path.steps.back().role = "solvent";
                break;
            }
    }
    above.pop_back();
    return path;
}

auto ThermoEngine::calculationPath(const std::string& symbol) const -> CalculationPath
{
    std::vector<std::string> above;
    return pimpl->database.containsSubstance(symbol) || !pimpl->database.containsReaction(symbol) ? pimpl->pathOfSubstance(symbol, above) : pimpl->pathOfReaction(symbol, above);
}

} // namespace ThermoFun
