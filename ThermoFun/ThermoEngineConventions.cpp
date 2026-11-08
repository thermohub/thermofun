// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// A change of convention of the results: the engine is canonical inside.

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

/// The sum of the entropies of the elements of a formula, for a change of the energy reference. A value that a record of the
/// database does not give comes from the element table of ChemicalFun, with a warning.
auto ThermoEngine::Impl::elementEntropy(const std::string& formula) const -> double
{
    std::lock_guard<std::mutex> entropy_lock(bound_cache->entropy_mutex);
    const auto known = bound_cache->element_entropies.find(formula);
    if (known != bound_cache->element_entropies.end())
        return known->second;
    const auto sources = database.elementEntropySources(formula);
    const auto names = [](const std::vector<std::string>& list) {
        std::string text;
        for (const auto& name : list)
            text += (text.empty() ? "" : ", ") + name;
        return text;
    };
    if (!sources.missing.empty())
        throw std::runtime_error("CNV-001: a conversion for " + formula + " needs the entropies of the elements " + names(sources.missing) + ", and no source has them.");
    static std::mutex warned_mutex;
    static std::set<std::string> warned;
    std::lock_guard<std::mutex> lock(warned_mutex);
    for (const auto& element : sources.from_table)
        if (warned.insert(element).second)
            thfun_logger->warn("CNV-006: the entropy of the element {} comes from the element table of ChemicalFun, not from the database", element);
    return bound_cache->element_entropies[formula] = database.elementalEntropyFormula(formula);
}

/// What a conversion needs to know about a record
auto ThermoEngine::Impl::subjectOf(const v2::SubstanceRecord& record) const -> v2::ConversionSubject
{
    v2::ConversionSubject subject;
    subject.Tr = record.reference_conditions.T;
    subject.gas = record.aggregate_state == v2::AggregateState::Gas;
    subject.solute = record.class_ == v2::SubstanceClass::AqueousSolute;
    const auto formula = record.formula;
    subject.element_entropy = [this, formula] { return elementEntropy(formula); };
    return subject;
}

/// The convention that the engine returns
auto ThermoEngine::Impl::outputConvention() const -> v2::Convention
{
    return {conventions.energy_reference, conventions.standard_pressure, conventions.concentration_scale, conventions.water_reference};
}

/// The convention of the values of a record: its own, else that of the dataset
auto ThermoEngine::Impl::declaredConvention(const v2::SubstanceRecord& record) const -> v2::Convention
{
    return v2::mergeConvention(record.convention.value_or(v2::Convention{}), database.thermoDataSet().convention);
}

/// Water from the geochemical reference to the steam tables (sign -1), and back (sign 1)
auto ThermoEngine::Impl::steamTablesShift(ThermoPropertiesSubstance &tps, double T, double sign) const -> void
{
    const auto wat = waterTripleData().at(database.conventions().water_triple_point_reference);
    // the entropy has a constant added, so G and A have also a term linear in T: G = G(steam) + Gtr - Str (T - Ttr). The error and status are not changed
    const double Ttr = v2::builtinConstants("WaterHGK84", "reaktoro")->parameters.at("Ttr").value;
    const double linear = wat.Str * (T - Ttr);
    tps.gibbs_energy.val += sign * (wat.Gtr - linear);
    tps.gibbs_energy.ddt -= sign * wat.Str;
    tps.enthalpy.val += sign * wat.Htr;
    tps.entropy.val += sign * wat.Str;
    tps.helmholtz_energy.val += sign * (wat.Atr - linear);
    tps.helmholtz_energy.ddt -= sign * wat.Str;
    tps.internal_energy.val += sign * wat.Utr;
}

/// The density terms of a change between molarity and molality at T, P. V in J/bar.
auto ThermoEngine::Impl::molarityTermsAt(double T, double& P, const std::string& what) const -> v2::MolarityTerms<Reaktoro_::ThermoScalar>
{
    const auto symbol = solvent_in_use;
    if (!database.containsSubstance(symbol))
        throw std::runtime_error("CNV-002: " + what + " needs the solvent " + symbol + " for molarity, and the database does not have it.");
    const auto s = cachedSolvent(T, P, symbol, -1);
    Reaktoro_::ThermoScalar Ts(T);
    Ts.ddt = 1.0;
    auto m = v2::molarityTerms(Ts, s.density, s.densityT, s.densityP, s.densityTT);
    m.V *= 1e5;
    return m;
}

/// A change of convention of the properties of a substance
auto ThermoEngine::Impl::convert(ThermoPropertiesSubstance &tps, const v2::ConventionShift& shift, double T, double& P, const std::string& symbol) const -> void
{
    // a constant or linear in T: the error and status are not changed. A = U - T S follows S; G0 does not enter it.
    if (defined(tps.gibbs_energy))
    {
        tps.gibbs_energy.val += shift.G0 + shift.G_per_K * T;
        tps.gibbs_energy.ddt += shift.G_per_K;
    }
    if (defined(tps.entropy))
        tps.entropy.val -= shift.G_per_K;
    if (defined(tps.helmholtz_energy))
    {
        tps.helmholtz_energy.val += shift.G_per_K * T;
        tps.helmholtz_energy.ddt += shift.G_per_K;
    }
    if (!shift.needsSolvent())
        return;
    const auto m = molarityTermsAt(T, P, symbol);
    add(tps.gibbs_energy, shift.molarity, m.G);
    add(tps.enthalpy, shift.molarity, m.H);
    add(tps.entropy, shift.molarity, m.S);
    add(tps.heat_capacity_cp, shift.molarity, m.Cp);
    add(tps.volume, shift.molarity, m.V);
    completeThermoProperties(tps, T, P, true);
}

/// The properties in the convention that the engine returns. Inside the engine all values are canonical.
auto ThermoEngine::Impl::toOutput(ThermoPropertiesSubstance tps, double T, double& P, const Substance& substance, DetailedPropertiesSubstance* detailed) const -> ThermoPropertiesSubstance
{
    if (outputConvention() == v2::canonicalConvention())
        return tps;
    const auto record = substance.record();
    const auto label = [&](const std::string& word) { if (detailed) detailed->conversions.push_back(word); };
    if (record->class_ == v2::SubstanceClass::WaterSolvent)
    {
        if (conventions.water_reference == v2::WaterReference::SteamTables)
        {
            steamTablesShift(tps, T, -1.0);
            label("steam_tables");
        }
        return tps;
    }
    const auto out = outputConvention();
    const auto shift = v2::shiftFromCanonical(out, subjectOf(*record));
    if (shift.identity())
        return tps;
    convert(tps, shift, T, P, record->symbol);
    if (conventions.energy_reference != v2::EnergyReference::BensonHelgeson)
        label(v2::toString(conventions.energy_reference));
    if (subjectOf(*record).gas && conventions.standard_pressure != v2::StandardPressure::OneBar)
        label(v2::toString(conventions.standard_pressure));
    if (subjectOf(*record).solute && conventions.concentration_scale != v2::ConcentrationScale::Molality)
        label(v2::toString(conventions.concentration_scale));
    return tps;
}

/// The reference properties of a substance in the canonical convention. They are the values at the reference temperature.
auto ThermoEngine::Impl::canonicalReference(const Substance& substance) const -> ThermoPropertiesSubstance
{
    auto tps = substance.thermoReferenceProperties();
    const auto record = substance.record();
    const auto declared = declaredConvention(*record);
    if (record->class_ == v2::SubstanceClass::WaterSolvent)
    {
        if (declared.water_reference == v2::WaterReference::SteamTables)
            steamTablesShift(tps, substance.referenceT(), 1.0);
        return tps;
    }
    if (declared == v2::Convention{})
        return tps;
    const auto shift = v2::shiftToCanonical(declared, subjectOf(*record));
    if (!shift.identity())
    {
        double Pr = record->reference_conditions.P;
        convert(tps, shift, record->reference_conditions.T, Pr, record->symbol);
    }
    return tps;
}

/// The change of a reaction for a change of convention of its species: the sum over the species. A balanced reaction has no change for the energy
/// reference: the standard pressure and the concentration scale are what count.
auto ThermoEngine::Impl::reactionShift(const Reaction& reaction, const v2::Convention& convention, bool toCanonical) const -> v2::ConventionShift
{
    v2::Convention state = convention;
    state.energy_reference.reset();
    v2::ConventionShift sum;
    if ((!state.standard_pressure || *state.standard_pressure == v2::StandardPressure::OneBar) &&
        (!state.concentration_scale || *state.concentration_scale == v2::ConcentrationScale::Molality))
        return sum;
    for (const auto& [symbol, coefficient] : reaction.reactants())
    {
        const auto subject = subjectOf(*database.getSubstance(symbol).record());
        const auto shift = toCanonical ? v2::shiftToCanonical(state, subject) : v2::shiftFromCanonical(state, subject);
        sum.G_per_K += coefficient * shift.G_per_K;
        sum.molarity += coefficient * shift.molarity;
    }
    return sum;
}

/// A change of convention of the properties of a reaction
auto ThermoEngine::Impl::convert(ThermoPropertiesReaction &tpr, const v2::ConventionShift& shift, double T, double& P, const std::string& symbol) const -> void
{
    if (defined(tpr.reaction_gibbs_energy))
    {
        tpr.reaction_gibbs_energy.val += shift.G_per_K * T;
        tpr.reaction_gibbs_energy.ddt += shift.G_per_K;
    }
    if (defined(tpr.reaction_entropy))
        tpr.reaction_entropy.val -= shift.G_per_K;
    if (defined(tpr.reaction_helmholtz_energy))
    {
        tpr.reaction_helmholtz_energy.val += shift.G_per_K * T;
        tpr.reaction_helmholtz_energy.ddt += shift.G_per_K;
    }
    if (defined(tpr.ln_equilibrium_constant))
        tpr.ln_equilibrium_constant.val -= shift.G_per_K / R_CONSTANT;
    if (defined(tpr.log_equilibrium_constant))
        tpr.log_equilibrium_constant.val -= shift.G_per_K / (R_CONSTANT * lg_to_ln);
    if (!shift.needsSolvent())
        return;
    const auto m = molarityTermsAt(T, P, symbol);
    const double Pbar = P / bar_to_Pa;
    const auto U = m.H - Pbar * m.V;
    const auto A = U - T * m.S;
    Reaktoro_::ThermoScalar RT(R_CONSTANT * T);
    RT.ddt = R_CONSTANT;
    const auto lnK = -(m.G / RT);
    add(tpr.reaction_gibbs_energy, shift.molarity, m.G);
    add(tpr.reaction_enthalpy, shift.molarity, m.H);
    add(tpr.reaction_entropy, shift.molarity, m.S);
    add(tpr.reaction_heat_capacity_cp, shift.molarity, m.Cp);
    add(tpr.reaction_heat_capacity_cv, shift.molarity, m.Cp);
    add(tpr.reaction_volume, shift.molarity, m.V);
    add(tpr.reaction_internal_energy, shift.molarity, U);
    add(tpr.reaction_helmholtz_energy, shift.molarity, A);
    add(tpr.ln_equilibrium_constant, shift.molarity, lnK);
    add(tpr.log_equilibrium_constant, shift.molarity / lg_to_ln, lnK);
}

auto ThermoEngine::Impl::toOutput(ThermoPropertiesReaction tpr, double T, double& P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    if (outputConvention() == v2::canonicalConvention())
        return tpr;
    const auto shift = reactionShift(reaction, outputConvention(), false);
    if (!shift.identity())
        convert(tpr, shift, T, P, reaction.symbol());
    return tpr;
}

auto ThermoEngine::Impl::reactionOfSymbol(const std::string& symbol) const -> Reaction
{
    if (!database.containsReaction(symbol) && symbol.find("=") != std::string::npos)
    {
        Reaction reaction;
        reaction.fromEquation(symbol);
        return reaction;
    }
    return database.getReaction(symbol);
}

} // namespace ThermoFun
