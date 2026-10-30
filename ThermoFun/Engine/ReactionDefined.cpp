// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReactionDefined.h"

#include <algorithm>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>

namespace ThermoFun::v2 {

auto substanceFromReaction(double T, double P, const ThermoPropertiesReaction& tpr, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& others, double coeff) -> ThermoPropertiesSubstance
{
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        ThermoPropertiesSubstanceAD a;
        a.enthalpy         = pass(tpr.reaction_enthalpy);
        a.entropy          = pass(tpr.reaction_entropy);
        a.gibbs_energy     = pass(tpr.reaction_gibbs_energy);
        a.heat_capacity_cp = pass(tpr.reaction_heat_capacity_cp);
        a.heat_capacity_cv = pass(tpr.reaction_heat_capacity_cv);
        a.helmholtz_energy = pass(tpr.reaction_helmholtz_energy);
        a.internal_energy  = pass(tpr.reaction_internal_energy);
        a.volume           = pass(tpr.reaction_volume);

        for (const auto& other : others)
        {
            a.enthalpy         -= pass(other.first.enthalpy) * other.second;
            a.entropy          -= pass(other.first.entropy) * other.second;
            a.gibbs_energy     -= pass(other.first.gibbs_energy) * other.second;
            a.heat_capacity_cp -= pass(other.first.heat_capacity_cp) * other.second;
            a.heat_capacity_cv -= pass(other.first.heat_capacity_cv) * other.second;
            a.helmholtz_energy -= pass(other.first.helmholtz_energy) * other.second;
            a.internal_energy  -= pass(other.first.internal_energy) * other.second;
            a.volume           -= pass(other.first.volume) * other.second;
        }

        a.enthalpy         = a.enthalpy / coeff;
        a.entropy          = a.entropy / coeff;
        a.gibbs_energy     = a.gibbs_energy / coeff;
        a.heat_capacity_cp = a.heat_capacity_cp / coeff;
        a.heat_capacity_cv = a.heat_capacity_cv / coeff;
        a.helmholtz_energy = a.helmholtz_energy / coeff;
        a.internal_energy  = a.internal_energy / coeff;
        a.volume           = a.volume / coeff;
        return a;
    });

    // each property is not defined if the property of the reaction or of any other reactant is not defined
    auto setStatus = [&](Reaktoro_::ThermoProperty& p, const Reaktoro_::ThermoProperty& fromReaction,
                         Reaktoro_::ThermoProperty ThermoPropertiesSubstance::* member)
    {
        p.propagateFrom(fromReaction);
        for (const auto& other : others)
            p.propagateFrom(p, other.first.*member);
    };
    setStatus(tps.enthalpy,         tpr.reaction_enthalpy,          &ThermoPropertiesSubstance::enthalpy);
    setStatus(tps.entropy,          tpr.reaction_entropy,           &ThermoPropertiesSubstance::entropy);
    setStatus(tps.gibbs_energy,     tpr.reaction_gibbs_energy,      &ThermoPropertiesSubstance::gibbs_energy);
    setStatus(tps.heat_capacity_cp, tpr.reaction_heat_capacity_cp,  &ThermoPropertiesSubstance::heat_capacity_cp);
    setStatus(tps.heat_capacity_cv, tpr.reaction_heat_capacity_cv,  &ThermoPropertiesSubstance::heat_capacity_cv);
    setStatus(tps.helmholtz_energy, tpr.reaction_helmholtz_energy,  &ThermoPropertiesSubstance::helmholtz_energy);
    setStatus(tps.internal_energy,  tpr.reaction_internal_energy,   &ThermoPropertiesSubstance::internal_energy);
    setStatus(tps.volume,           tpr.reaction_volume,            &ThermoPropertiesSubstance::volume);
    return tps;
}

auto setIdealGasVolume(ThermoPropertiesSubstance& tps, double T, double P) -> void
{
    const double Pbar = P / bar_to_Pa;
    tps.volume.val = R_CONSTANT * T / Pbar; // J/bar
    tps.volume.ddt = R_CONSTANT / Pbar;
    tps.volume.ddp = -tps.volume.val / P;
    tps.volume.sta = {Reaktoro_::Status::calculated, ""};
}

namespace {

auto stack() -> std::vector<std::string>&
{
    thread_local std::vector<std::string> symbols;
    return symbols;
}

} // namespace

DefiningReactionGuard::DefiningReactionGuard(const std::string& symbol)
{
    auto& symbols = stack();
    const auto found = std::find(symbols.begin(), symbols.end(), symbol);
    if (found != symbols.end())
    {
        std::string cycle;
        for (auto it = found; it != symbols.end(); ++it)
            cycle += *it + " -> ";
        funError("A cycle of defining reactions: " + cycle + symbol, "Each of these substances is defined by a reaction with the next one. Give one of them its own properties.", __LINE__, __FILE__);
    }
    symbols.push_back(symbol);
}

DefiningReactionGuard::~DefiningReactionGuard()
{
    stack().pop_back();
}

} // namespace ThermoFun::v2
