// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReactionDefined.h"

#include <algorithm>
#include <cmath>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/ThermoPropertiesStatus.h>

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

auto reactionFromReactants(double T, double P, const std::string& reaction, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& components, const std::vector<std::string>& symbols) -> ThermoPropertiesReaction
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

    std::string message = "Calculated from the reaction components: " + reaction + "; ";

        // the values and derivatives of the properties of the reaction
        if (!components.empty())
        {
            auto values = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
                ThermoPropertiesReactionAD a;
                a.reaction_heat_capacity_cp = 0.0;
                a.reaction_gibbs_energy = 0.0;
                a.reaction_enthalpy = 0.0;
                a.reaction_entropy = 0.0;
                a.reaction_volume = 0.0;
                a.reaction_heat_capacity_cv = 0.0;
                a.reaction_internal_energy = 0.0;
                a.reaction_helmholtz_energy = 0.0;

                for (const auto& component : components)
                {
                    const auto& tps = component.first;
                    const auto coeff = component.second;

                    a.reaction_heat_capacity_cp += pass(tps.heat_capacity_cp)*coeff;
                    a.reaction_gibbs_energy     += pass(tps.gibbs_energy)*coeff;
                    a.reaction_enthalpy         += pass(tps.enthalpy)*coeff;
                    a.reaction_entropy          += pass(tps.entropy)*coeff;
                    a.reaction_volume           += pass(tps.volume)*coeff;
                    a.reaction_heat_capacity_cv += pass(tps.heat_capacity_cv)*coeff;
                    a.reaction_internal_energy  += pass(tps.internal_energy)*coeff;
                    a.reaction_helmholtz_energy += pass(tps.helmholtz_energy)*coeff;
                }
                a.ln_equilibrium_constant  = a.reaction_gibbs_energy / -(R_CONSTANT*pass.T);
                a.log_equilibrium_constant = a.ln_equilibrium_constant * ln_to_lg;
                return a;
            });

            // the errors and statuses are set below
            auto copyValues = [](Reaktoro_::ThermoProperty& to, const Reaktoro_::ThermoProperty& from) {
                to.val = from.val; to.ddt = from.ddt; to.ddp = from.ddp;
            };
            copyValues(tpr.reaction_heat_capacity_cp, values.reaction_heat_capacity_cp);
            copyValues(tpr.reaction_gibbs_energy, values.reaction_gibbs_energy);
            copyValues(tpr.reaction_enthalpy, values.reaction_enthalpy);
            copyValues(tpr.reaction_entropy, values.reaction_entropy);
            copyValues(tpr.reaction_volume, values.reaction_volume);
            copyValues(tpr.ln_equilibrium_constant, values.ln_equilibrium_constant);
            copyValues(tpr.log_equilibrium_constant, values.log_equilibrium_constant);
            copyValues(tpr.reaction_heat_capacity_cv, values.reaction_heat_capacity_cv);
            copyValues(tpr.reaction_internal_energy, values.reaction_internal_energy);
            copyValues(tpr.reaction_helmholtz_energy, values.reaction_helmholtz_energy);
        }

        // the errors and statuses, and the messages of the properties that are not defined
        for (size_t i = 0; i < components.size(); ++i)
        {
            const auto& tps = components[i].first;
            const auto& substance = symbols[i];

            tpr.reaction_heat_capacity_cp.propagateFrom(tpr.reaction_heat_capacity_cp, tps.heat_capacity_cp);
            tpr.reaction_gibbs_energy.propagateFrom(tpr.reaction_gibbs_energy, tps.gibbs_energy);
            tpr.reaction_enthalpy.propagateFrom(tpr.reaction_enthalpy, tps.enthalpy);
            tpr.reaction_entropy.propagateFrom(tpr.reaction_entropy, tps.entropy);
            tpr.reaction_volume.propagateFrom(tpr.reaction_volume, tps.volume);
            tpr.ln_equilibrium_constant.propagateFrom(tpr.reaction_gibbs_energy);
            tpr.log_equilibrium_constant.propagateFrom(tpr.ln_equilibrium_constant);
            tpr.reaction_heat_capacity_cv.propagateFrom(tpr.reaction_heat_capacity_cv, tps.heat_capacity_cv);
            tpr.reaction_internal_energy.propagateFrom(tpr.reaction_internal_energy, tps.internal_energy);
            tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_helmholtz_energy, tps.helmholtz_energy);

            setMessage(tps.heat_capacity_cp.sta.first, "Cp of component " + substance, message+tps.heat_capacity_cp.sta.second, tpr.reaction_heat_capacity_cp.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.reaction_gibbs_energy.sta.second);
            setMessage(tps.enthalpy.sta.first,         "H0 of component " + substance, message+tps.enthalpy.sta.second,         tpr.reaction_enthalpy.sta.second);
            setMessage(tps.entropy.sta.first,          "S0 of component " + substance, message+tps.entropy.sta.second,          tpr.reaction_entropy.sta.second);
            setMessage(tps.volume.sta.first,           "V0 of component " + substance, message+tps.volume.sta.second,           tpr.reaction_volume.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.log_equilibrium_constant.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.ln_equilibrium_constant.sta.second);
        }

    // the errors: first-order propagation of the errors of the reactants (independent), weighted by the
    // stoichiometric coefficients; ln K = -G/(R T)
    {
        double cp = 0, g = 0, h = 0, sv = 0, v = 0, cv = 0, u = 0, am = 0;
        for (const auto& component : components)
        {
            const auto& tps = component.first;
            const double nu = component.second;
            cp += nu*nu*tps.heat_capacity_cp.err*tps.heat_capacity_cp.err;
            g  += nu*nu*tps.gibbs_energy.err*tps.gibbs_energy.err;
            h  += nu*nu*tps.enthalpy.err*tps.enthalpy.err;
            sv += nu*nu*tps.entropy.err*tps.entropy.err;
            v  += nu*nu*tps.volume.err*tps.volume.err;
            cv += nu*nu*tps.heat_capacity_cv.err*tps.heat_capacity_cv.err;
            u  += nu*nu*tps.internal_energy.err*tps.internal_energy.err;
            am += nu*nu*tps.helmholtz_energy.err*tps.helmholtz_energy.err;
        }
        tpr.reaction_heat_capacity_cp.setError({{1.0, std::sqrt(cp)}});
        tpr.reaction_gibbs_energy.setError({{1.0, std::sqrt(g)}});
        tpr.reaction_enthalpy.setError({{1.0, std::sqrt(h)}});
        tpr.reaction_entropy.setError({{1.0, std::sqrt(sv)}});
        tpr.reaction_volume.setError({{1.0, std::sqrt(v)}});
        tpr.ln_equilibrium_constant.setError({{1.0/(R_CONSTANT*T), std::sqrt(g)}});
        tpr.log_equilibrium_constant.setError({{ln_to_lg, tpr.ln_equilibrium_constant.err}});
        tpr.reaction_heat_capacity_cv.setError({{1.0, std::sqrt(cv)}});
        tpr.reaction_internal_energy.setError({{1.0, std::sqrt(u)}});
        tpr.reaction_helmholtz_energy.setError({{1.0, std::sqrt(am)}});
    }

    return tpr;
}

auto constantVolumeCorrection(double T, double P, const ThermoPropertiesSubstance& tps, const ThermoPropertiesSubstance& reference, double Pr) -> ThermoPropertiesSubstance
{
    if (reference.volume.sta.first == Reaktoro_::notdefined)  // do pressure correction only if the molar volume is given
        return tps;
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real t = pass.T;
        const real p = pass.P / bar_to_Pa;
        auto state = lift(pass, tps);
        const real V = Reaktoro_::constant(reference.volume);
        const real dV = V * (p - (Pr / bar_to_Pa));
        state.volume           = V;
        state.gibbs_energy    += dV;
        state.enthalpy        += dV;
        state.internal_energy  = state.enthalpy - p*state.volume;
        state.helmholtz_energy = state.internal_energy - (t)*state.entropy;
        return state;
    });

    const auto copy = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
    copy(out.gibbs_energy, tps.gibbs_energy);
    copy(out.helmholtz_energy, tps.helmholtz_energy);
    copy(out.internal_energy, tps.internal_energy);
    copy(out.enthalpy, tps.enthalpy);
    copy(out.entropy, tps.entropy);
    copy(out.volume, tps.volume);
    copy(out.heat_capacity_cp, tps.heat_capacity_cp);
    copy(out.heat_capacity_cv, tps.heat_capacity_cv);
    out.volume.sta = reference.volume.sta;
    out.volume.err = reference.volume.err;
    out.gibbs_energy.propagateFrom(tps.gibbs_energy, reference.volume);
    out.enthalpy.propagateFrom(tps.enthalpy, reference.volume);
    const double Pbar = P / bar_to_Pa;
    out.internal_energy.propagateFrom(out.enthalpy, out.volume);
    out.internal_energy.setError({{1.0, out.enthalpy.err}, {Pbar, out.volume.err}});
    out.helmholtz_energy.propagateFrom(out.internal_energy, out.entropy);
    out.helmholtz_energy.setError({{1.0, out.internal_energy.err}, {T, out.entropy.err}});
    return out;
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
