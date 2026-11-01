// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The derivatives of the properties by the parameters of the records, along the same paths as the properties.

#include <algorithm>

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

namespace
{

const char* const substanceValues[] = {"G", "H", "S", "Cp", "V"};
const char* const reactionValues[] = {"logK", "dG", "dH", "dS", "dCp", "dV"};

template <std::size_t N>
auto isIn(const char* const (&names)[N], const std::string& name) -> bool
{
    return std::find(std::begin(names), std::end(names), name) != std::end(names);
}

auto toSI(ThermoPropertiesReaction d) -> ThermoPropertiesReaction
{
    d.reaction_volume.val *= 1e-5;
    return d;
}

auto joined(const std::vector<std::string>& names) -> std::string
{
    std::string out;
    for (const auto& n : names)
        if (!n.empty())
            out += (out.empty() ? "" : ", ") + n;
    return out.empty() ? "none" : out;
}

auto indexOf(const std::vector<std::string>& names, const std::string& name) -> int
{
    const auto it = std::find(names.begin(), names.end(), name);
    return it == names.end() ? -1 : static_cast<int>(it - names.begin());
}

} // namespace

/// The parameter must be in the record: else a derivative of 0 would look like a result.
auto ThermoEngine::Impl::checkParameter(const ParameterRef& ref) const -> void
{
    const auto fail = [&](const std::string& reason) { funError("Cannot derive by the parameter " + ref.name(), reason, __LINE__, __FILE__); };
    if (ref.method.empty())
    {
        if (isIn(substanceValues, ref.parameter))
        {
            if (!database.containsSubstance(ref.symbol))
                fail("The database has no substance with this symbol.");
            const auto s = database.getSubstance(ref.symbol).thermoReferenceProperties();
            const std::map<std::string, const Reaktoro_::ThermoProperty*> values = {{"G", &s.gibbs_energy}, {"H", &s.enthalpy}, {"S", &s.entropy}, {"Cp", &s.heat_capacity_cp}, {"V", &s.volume}};
            if (values.at(ref.parameter)->sta.first == Reaktoro_::Status::notdefined)
                fail("The record does not give this value.");
        }
        else if (isIn(reactionValues, ref.parameter))
        {
            if (!database.containsReaction(ref.symbol))
                fail("The database has no reaction with this symbol.");
        }
        else
            fail("A reference value is one of G, H, S, Cp, V of a substance, or logK, dG, dH, dS, dCp, dV of a reaction.");
        return;
    }
    const auto* spec = v2::MethodCatalog::instance().find(ref.method);
    if (!spec)
        fail("The catalog has no such method.");
    if (spec->operates_on == v2::OperatesOn::Reaction)
    {
        if (!database.containsReaction(ref.symbol))
            fail("The database has no reaction with this symbol.");
        const auto bound = boundReaction(database.getReaction(ref.symbol));
        for (const auto& method : {bound ? bound->base : nullptr, bound ? bound->volume : nullptr})
            if (method && method->spec().id == spec->id)
            {
                const auto names = method->parameterNames();
                if (indexOf(names, ref.parameter) < 0)
                    fail("This method of the record has the parameters: " + joined(names) + ".");
                return;
            }
        fail("The record does not use this method.");
    }
    if (!database.containsSubstance(ref.symbol))
        fail("The database has no substance with this symbol.");
    if (spec->role != v2::MethodRole::SubstanceProperties)
        fail("A method of water has no derivatives by its parameters.");
    auto bound = boundMethods(database.getSubstance(ref.symbol));
    if (!bound)  // only corrections, on what a reaction gives
        bound = pressureCorrections(database.getSubstance(ref.symbol));
    if (bound)
        for (const auto& method : bound->order)
            if (method->spec().id == spec->id)
            {
                const auto names = method->parameterNames();
                if (indexOf(names, ref.parameter) < 0)
                    fail("This method of the record has the parameters: " + joined(names) + ".");
                return;
            }
    fail("The record does not use this method.");
}

/// What a pass by the parameter sets in the bound methods of a record: nothing if the parameter is of another record.
auto ThermoEngine::Impl::seedOf(const v2::BoundSubstance& bound, const ParameterRef& ref) const -> v2::ParameterSeed
{
    v2::ParameterSeed seed;
    if (ref.symbol != bound.record->symbol)
        return seed;
    if (ref.method.empty())
    {
        seed.G = ref.parameter == "G";
        seed.H = ref.parameter == "H";
        seed.S = ref.parameter == "S";
        seed.Cp = ref.parameter == "Cp";
        seed.V = ref.parameter == "V";
        return seed;
    }
    const auto* spec = v2::MethodCatalog::instance().find(ref.method);
    for (std::size_t i = 0; spec && i < bound.order.size(); ++i)
        if (bound.order[i]->spec().id == spec->id)
        {
            seed.method = static_cast<int>(i);
            seed.parameter = indexOf(bound.order[i]->parameterNames(), ref.parameter);
        }
    return seed;
}

/// In the canonical convention: the changes of convention do not depend on a parameter. P is known here: a pressure of 0 was changed by the calculation of the properties.
auto ThermoEngine::Impl::substanceDerivative(double T, double& P, const Substance& substance, const ParameterRef& ref) const -> v2::PropertyDerivatives
{
    const auto symbol = substance.symbol();
    const SubstanceKind kind = kindOf(substance);
    v2::PropertyDerivatives d;
    if (kind.isHydrogen)
        return d;

    const bool own = ref.symbol == symbol && ref.method.empty();
    ThermoPropertiesSubstance raw;  // what the calculation gives before the fallback to the reference values
    const bool isWater = kind.isH2OSolvent || kind.isH2Ovapor;
    const bool fromReaction = kind.isReacDC && !substance.reactionSymbol().empty();

    if (fromReaction)
    {
        raw = reacDCthermoProperties(T, P, substance);
        const Reaction reaction = database.getReaction(substance.reactionSymbol());
        const v2::DefiningReactionGuard no_cycle(symbol);
        const bool ownTerm = pressureFromRecord(substance, reaction);
        double Pdata = dataPressure(T, substance);
        const bool fromData = ownTerm && T < water_critical_T;
        double& Pcalc = fromData ? Pdata : P;
        const auto r = reactionDerivative(T, Pcalc, reaction, ref);
        d = {r.reaction_gibbs_energy.val, r.reaction_enthalpy.val, r.reaction_entropy.val, r.reaction_heat_capacity_cp.val, r.reaction_volume.val};
        auto reactants = reaction.reactants();
        for (const auto& [other, coefficient] : reactants)
            if (other != symbol)
            {
                const auto o = substanceDerivative(T, Pcalc, database.getSubstance(other), ref);
                d.G -= coefficient * o.G;
                d.H -= coefficient * o.H;
                d.S -= coefficient * o.S;
                d.Cp -= coefficient * o.Cp;
                d.V -= coefficient * o.V;
            }
        const double nu = reactants[symbol];
        d.G /= nu;
        d.H /= nu;
        d.S /= nu;
        d.Cp /= nu;
        d.V /= nu;
        if (ownTerm)
        {
            // see reacDCthermoProperties
            if (hasMethod(substance.record()->methods, "ConstantVolume") && substance.thermoReferenceProperties().volume.sta.first != Reaktoro_::Status::notdefined)
            {
                const double seedV = own && ref.parameter == "V" ? 1.0 : 0.0;
                const double from = fromData ? Pdata : substance.referenceP();
                d.V = seedV;
                d.G += seedV * (P - from);
                d.H += seedV * (P - from);
            }
            if (const auto corrections = pressureCorrections(substance); corrections && !corrections->order.empty())
            {
                const auto solvent = solventInputs(*corrections, T, P);
                const std::pair<v2::MethodResult, v2::PropertyDerivatives> initial{valuesOf(raw), d};
                d = v2::substanceParameterDerivative(*corrections, seedOf(*corrections, ref), T, P, substanceOptions(), solvent ? &*solvent : nullptr, &initial);
            }
        }
    }
    else if (kind.isReacDC)
    {
        d = {own && ref.parameter == "G" ? 1.0 : 0.0, own && ref.parameter == "H" ? 1.0 : 0.0, own && ref.parameter == "S" ? 1.0 : 0.0, own && ref.parameter == "Cp" ? 1.0 : 0.0, own && ref.parameter == "V" ? 1.0 : 0.0};
        return d;
    }
    else if (isWater)
    {
        const auto solvent = boundSolvent(substance);
        if (solvent->equation_of_state)
            return d;  // the equation of state does not read the reference values
        if (solvent->reference_values)
        {
            raw = toThermoProperties(v2::executeSubstance(*solvent->reference_values, T, P, substanceOptions()));
            d = v2::substanceParameterDerivative(*solvent->reference_values, seedOf(*solvent->reference_values, ref), T, P, substanceOptions());
        }
    }
    else if (const auto bound = boundMethods(substance))
    {
        const auto solvent = solventInputs(*bound, T, P);
        const auto* inputs = solvent ? &*solvent : nullptr;
        raw = toThermoProperties(v2::executeSubstance(*bound, T, P, substanceOptions(), nullptr, inputs));
        d = v2::substanceParameterDerivative(*bound, seedOf(*bound, ref), T, P, substanceOptions(), inputs);
    }

    if (preferences.fallback_to_reference_properties)
    {
        const auto fallback = [&](const Reaktoro_::ThermoProperty& x, double& to, const char* name) {
            if (x.sta.first == Reaktoro_::Status::notdefined)
                to = own && ref.parameter == name ? 1.0 : 0.0;
        };
        fallback(raw.gibbs_energy, d.G, "G");
        fallback(raw.enthalpy, d.H, "H");
        fallback(raw.entropy, d.S, "S");
        fallback(raw.heat_capacity_cp, d.Cp, "Cp");
        fallback(raw.volume, d.V, "V");
    }
    return d;
}

auto ThermoEngine::Impl::reactionDerivative(double T, double& P, const Reaction& reaction, const ParameterRef& ref, bool alwaysFromReactants) const -> ThermoPropertiesReaction
{
    const auto record = reaction.record();
    const auto source = sourceOf(reaction_sources, record->symbol, record->preferred_property_source);
    bool fromReactants = alwaysFromReactants || record->methods.empty();
    if (source == v2::PropertySource::Reactants && !record->stoichiometry.empty())
        fromReactants = true;

    ThermoPropertiesReaction d;
    if (fromReactants)
    {
        d.reaction_gibbs_energy = d.reaction_enthalpy = d.reaction_entropy = d.reaction_heat_capacity_cp = d.reaction_volume = 0.0;
        for (const auto& [symbol, coefficient] : reaction.reactants())
        {
            const auto o = substanceDerivative(T, P, database.getSubstance(symbol), ref);
            d.reaction_gibbs_energy.val += coefficient * o.G;
            d.reaction_enthalpy.val += coefficient * o.H;
            d.reaction_entropy.val += coefficient * o.S;
            d.reaction_heat_capacity_cp.val += coefficient * o.Cp;
            d.reaction_volume.val += coefficient * o.V;
        }
        d.ln_equilibrium_constant = d.reaction_gibbs_energy.val / -(R_CONSTANT * T);
        d.log_equilibrium_constant = d.ln_equilibrium_constant.val * ln_to_lg;
        return d;
    }
    // the parameter of another reaction, or of a substance
    if (ref.symbol != record->symbol || (ref.method.empty() && !isIn(reactionValues, ref.parameter)))
        return d;
    const auto* spec = ref.method.empty() ? nullptr : v2::MethodCatalog::instance().find(ref.method);
    if (spec && spec->operates_on != v2::OperatesOn::Reaction)
        return d;
    if (const auto bound = boundReaction(reaction))
    {
        std::optional<PropertiesSolvent> solvent;
        if (bound->needs_solvent)
            solvent = cachedSolvent(T, P, preferences.solvent_symbol, -1);
        return toSI(v2::reactionParameterDerivative(*bound, spec ? spec->id : "", ref.parameter, T, P, solvent ? &*solvent : nullptr));
    }
    return d;
}

auto ThermoEngine::parameterSensitivities(double T, double& P, const std::string& symbol, const std::vector<ParameterRef>& parameters) const -> SensitivityResult
{
    if (!Reaktoro_::kWithAutodiff)
        funError("Cannot calculate derivatives by parameters", "ThermoFun is built without autodiff.", __LINE__, __FILE__);
    pimpl->dropStaleResults();
    pimpl->cachedSubstance(T, P, symbol);
    SensitivityResult out;
    for (const auto& ref : parameters)
    {
        pimpl->checkParameter(ref);
        const auto d = pimpl->substanceDerivative(T, P, pimpl->database.getSubstance(symbol), ref);
        const auto name = ref.name();
        out["G"][name] = d.G;
        out["H"][name] = d.H;
        out["S"][name] = d.S;
        out["Cp"][name] = d.Cp;
        out["V"][name] = d.V;
        out["U"][name] = d.H - P * d.V;
        out["A"][name] = d.H - P * d.V - T * d.S;
    }
    return out;
}

auto ThermoEngine::reactionParameterSensitivities(double T, double& P, const std::string& symbol, const std::vector<ParameterRef>& parameters) const -> SensitivityResult
{
    if (!Reaktoro_::kWithAutodiff)
        funError("Cannot calculate derivatives by parameters", "ThermoFun is built without autodiff.", __LINE__, __FILE__);
    pimpl->dropStaleResults();
    pimpl->cachedReaction(T, P, symbol);
    const Reaction reaction = pimpl->reactionOfSymbol(symbol);
    SensitivityResult out;
    for (const auto& ref : parameters)
    {
        pimpl->checkParameter(ref);
        const auto d = pimpl->reactionDerivative(T, P, reaction, ref);
        const auto name = ref.name();
        out["logK"][name] = d.log_equilibrium_constant.val;
        out["lnK"][name] = d.ln_equilibrium_constant.val;
        out["dG"][name] = d.reaction_gibbs_energy.val;
        out["dH"][name] = d.reaction_enthalpy.val;
        out["dS"][name] = d.reaction_entropy.val;
        out["dCp"][name] = d.reaction_heat_capacity_cp.val;
        out["dV"][name] = d.reaction_volume.val;
    }
    return out;
}

} // namespace ThermoFun
