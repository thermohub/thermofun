// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The propagation of the errors of the parameters to the results, and their rounding.

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

namespace
{

/// u^2 = J S J^T for each result. jacobian: the derivatives of each result by one value, in the order of the fields.
template <class Fields, class Jacobian>
auto setPropagatedErrors(const Fields& fields, const model::UncertainInputs& inputs, Jacobian jacobian) -> void
{
    if (inputs.inputs.empty())
        return;
    std::vector<std::vector<double>> byResult(fields.size(), std::vector<double>(inputs.inputs.size(), 0.0));
    std::vector<char> incomplete(fields.size(), 0);  // a contribution to the error is missing
    for (std::size_t i = 0; i < inputs.inputs.size(); ++i)
    {
        try
        {
            const auto J = jacobian(inputs.inputs[i].ref);
            for (std::size_t k = 0; k < fields.size(); ++k)
                byResult[k][i] = J[k];
        }
        catch (const std::exception&)  // the calculation fails for this value: the error of every result is incomplete
        {
            std::fill(incomplete.begin(), incomplete.end(), 1);
        }
    }
    for (std::size_t k = 0; k < fields.size(); ++k)
    {
        const double variance = inputs.variance(byResult[k]);
        const bool defined = fields[k]->sta.first != Reaktoro_::Status::notdefined;
        if (!defined)
        {
            fields[k]->err = 0.0;
            continue;
        }
        if (!incomplete[k] && std::isfinite(variance))
        {
            fields[k]->err = std::sqrt(std::max(variance, 0.0));
            continue;
        }
        // not a number instead of an error that is too small (or 0) and looks complete
        fields[k]->err = std::numeric_limits<double>::quiet_NaN();
        auto& message = fields[k]->sta.second;
        message += std::string(message.empty() ? "" : " ") +
            "The uncertainty from the parameter errors is incomplete (the calculation failed, or the result is not finite): err is not a number.";
    }
}

} // namespace

auto ThermoEngine::Impl::fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.gibbs_energy, &p.helmholtz_energy, &p.internal_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp, &p.heat_capacity_cv};
}

auto ThermoEngine::Impl::fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
{
    return {&p.reaction_gibbs_energy, &p.reaction_helmholtz_energy, &p.reaction_internal_energy, &p.reaction_enthalpy, &p.reaction_entropy,
            &p.reaction_volume, &p.reaction_heat_capacity_cp, &p.reaction_heat_capacity_cv, &p.ln_equilibrium_constant, &p.log_equilibrium_constant};
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

auto ThermoEngine::Impl::inputsOf(const Records& records, const Substance* given, const Reaction* givenReaction) const -> model::UncertainInputs
{
    model::UncertainInputs inputs;
    const auto& sets = database.parameterSets();
    for (const auto& symbol : records.substances)
        inputs.add(*model::withParameterSets(database.getSubstance(symbol).record(), sets));
    for (const auto& symbol : records.reactions)
        inputs.add(*model::withParameterSets(database.getReaction(symbol).record(), sets));
    if (given)
        inputs.add(*model::withParameterSets(given->record(), sets));
    if (givenReaction)
        inputs.add(*model::withParameterSets(givenReaction->record(), sets));
    return inputs;
}

/// Cv = Cp - T V_T^2 / (-V_P) with V in J/bar. The second derivatives of V by T and P, with a parameter, are differences of the exact derivatives of V by the parameter.
auto ThermoEngine::Impl::substanceJacobian(double T, double& P, const Substance& substance, const ThermoPropertiesSubstance& tps, const ParameterRef& ref) const -> std::vector<double>
{
    const auto d = substanceDerivative(T, P, substance, ref);
    const double U = d.H - P * d.V;
    const double A = U - T * d.S;
    double Cv = d.Cp;
    const double VT = tps.volume.ddt(), VP = tps.volume.ddp();
    const SubstanceKind kind = kindOf(substance);
    if (kind.isH2OSolvent || kind.isH2Ovapor)
        Cv = 0.0;  // the Cv of its model, which has no parameters here
    else if (VP < 0.0 && VT != 0.0)
    {
        const double hT = 0.1, hP = std::max(1e-3 * P, 100.0);
        double Pa = P + hP, Pb = P - hP, Pc = P, Pd = P;
        const double dVT = (substanceDerivative(T + hT, Pc, substance, ref).V - substanceDerivative(T - hT, Pd, substance, ref).V) / (2.0 * hT) * 1e5;
        const double dVP = (substanceDerivative(T, Pa, substance, ref).V - substanceDerivative(T, Pb, substance, ref).V) / (2.0 * hP) * 1e5;
        Cv -= 1e-5 * T * (2.0 * VT * dVT / (-VP) + VT * VT * dVP / (VP * VP));
    }
    return {d.G, A, U, d.H, d.S, d.V * 1e5, d.Cp, Cv};
}

auto ThermoEngine::Impl::reactionJacobian(double T, double& P, const Reaction& reaction, const ParameterRef& ref, bool fromReactants) const -> std::vector<double>
{
    const auto d = reactionDerivative(T, P, reaction, ref, fromReactants);
    const double dH = d.reaction_enthalpy.val, dS = d.reaction_entropy.val, dV = d.reaction_volume.val;
    const double dU = dH - P * dV;
    double dCv = 0.0;
    const auto record = reaction.record();
    if (fromReactants || record->methods.empty())
    {
        // the sum of the Cv of the species
        for (const auto& [symbol, coefficient] : reaction.reactants())
        {
            const auto substance = database.getSubstance(symbol);
            dCv += coefficient * substanceJacobian(T, P, substance, cachedSubstance(T, P, symbol), ref)[7];
        }
    }
    return {d.reaction_gibbs_energy.val, dU - T * dS, dU, dH, dS, dV * 1e5, d.reaction_heat_capacity_cp.val, dCv, d.ln_equilibrium_constant.val, d.log_equilibrium_constant.val};
}

auto ThermoEngine::Impl::addSubstanceErrors(ThermoPropertiesSubstance& tps, double T, double& P, const Substance& substance, const model::UncertainInputs& inputs) const -> void
{
    const auto canonical = thermoPropertiesSubstance(T, P, substance);
    setPropagatedErrors(fields(tps), inputs, [&](const ParameterRef& ref) { return substanceJacobian(T, P, substance, canonical, ref); });
}

auto ThermoEngine::Impl::addReactionErrors(ThermoPropertiesReaction& tpr, double T, double& P, const Reaction& reaction, bool fromReactants, const model::UncertainInputs& inputs) const -> void
{
    setPropagatedErrors(fields(tpr), inputs, [&](const ParameterRef& ref) { return reactionJacobian(T, P, reaction, ref, fromReactants); });
}

} // namespace ThermoFun
