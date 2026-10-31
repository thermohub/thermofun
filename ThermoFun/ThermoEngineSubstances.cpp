// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The properties of substances and of the solvent, in the canonical convention.

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

/// Where the properties of a record come from: the source of the symbol, else the preference of the engine, else the
/// record. Empty: the rule of the record.
auto ThermoEngine::Impl::sourceOf(const std::string& symbol, const std::optional<v2::PropertySource>& preferred) const -> std::optional<v2::PropertySource>
{
    const auto own = property_sources.find(symbol);
    if (own != property_sources.end())
        return own->second;
    if (preferences.property_source != v2::PropertySource::Record)
        return preferences.property_source;
    return preferred;
}

/// A record with the implementations of the preferences
auto ThermoEngine::Impl::withImplementations(std::shared_ptr<const v2::SubstanceRecord> record) const -> std::shared_ptr<const v2::SubstanceRecord>
{
    if (preferences.method_implementation.empty())
        return record;
    auto copy = std::make_shared<v2::SubstanceRecord>(*record);
    for (const auto& [id, implementation] : preferences.method_implementation)
        for (auto& a : copy->methods)
            if (a.method == v2::MethodCatalog::instance().find(id)->id)
                a.implementation = implementation;
    return copy;
}

/// The methods of a substance. Null if they give no one base method: then no method runs.
auto ThermoEngine::Impl::boundMethods(const Substance& substance) const -> std::shared_ptr<const v2::BoundSubstance>
{
    const auto record = substance.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->substances.find(record.get());
    if (it != bound_cache->substances.end())
        return it->second;
    const auto formula = record->formula;
    const v2::BindConventions bind{database.thermoDataSet().convention, [this, formula] { return elementEntropy(formula); }};
    auto bound = v2::canBind(*record) ? v2::bindSubstance(withImplementations(record), bind) : nullptr;
    bound_cache->substances[record.get()] = bound;
    bound_cache->records.push_back(record);
    return bound;
}

/// The state of the solvent that the methods of a record need, calculated with the methods of the solvent record.
/// A pressure of 0 is the saturation pressure: it is written to P.
auto ThermoEngine::Impl::solventInputs(const v2::BoundSubstance& bound, double T, double& P) const -> std::optional<v2::SolventInputs>
{
    if (!bound.needs.solvent)
        return std::nullopt;
    v2::SolventInputs in;
    const auto symbol = preferences.solvent_symbol;
    if (!database.containsSubstance(symbol) && std::any_of(bound.to_canonical.begin(), bound.to_canonical.end(), [](const auto& s) { return s.needsSolvent(); }))
        throw std::runtime_error("CNV-002: " + bound.record->symbol + " is in molarity, and the database has no solvent " + symbol + ".");
    in.properties = cachedSolvent(T, P, symbol, -1);
    if (bound.needs.dielectric)
        in.electro = cachedElectro(T, P, symbol, -1);
    if (bound.needs.solvent_at_reference)
    {
        const auto& solvent = database.getSubstance(symbol);
        double Pr = solvent.referenceP();
        const double Tr = solvent.referenceT();
        in.properties_at_reference = cachedSolvent(Tr, Pr, symbol, -1);
        if (bound.needs.ideal_gas_water)
        {
            in.water = cachedSubstance(T, P, symbol);
            in.ideal_gas_water = waterIdealGasProperties(T, P);
            in.water_at_reference = cachedSubstance(Tr, Pr, symbol);
            in.ideal_gas_water_at_reference = waterIdealGasProperties(Tr, Pr);
        }
    }
    return in;
}

/// The methods of a water record.
auto ThermoEngine::Impl::boundSolvent(const Substance& substance) const -> std::shared_ptr<const v2::BoundSolvent>
{
    const auto record = substance.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->solvents.find(record.get());
    if (it != bound_cache->solvents.end())
        return it->second;
    auto bound = v2::bindSolvent(withImplementations(record));
    bound_cache->solvents[record.get()] = bound;
    bound_cache->records.push_back(record);
    return bound;
}

auto ThermoEngine::Impl::substanceOptions() const -> v2::SubstanceOptions
{
    return {preferences.apply_pressure_correction_to_gas_props, preferences.cancel_fugacity_coefficient_in_gas_props};
}

/// The result of the executor in the form of the old engine: V in J/bar, and the status of each property
auto ThermoEngine::Impl::toThermoProperties(const v2::SubstanceResult& r) -> ThermoPropertiesSubstance
{
    auto one = [](const v2::PropertyValue& v, const v2::ValueInfo& i, double factor = 1.0) {
        Reaktoro_::ThermoProperty p;
        p.val = v.val * factor;
        p.ddt = v.ddt * factor;
        p.ddp = v.ddp * factor;
        p.err = i.err * factor;
        p.sta = {i.status == v2::ValueInfo::Status::NotDefined ? Reaktoro_::Status::notdefined
                 : i.status == v2::ValueInfo::Status::Assigned ? Reaktoro_::Status::assigned
                 : i.status == v2::ValueInfo::Status::Read ? Reaktoro_::Status::read : Reaktoro_::Status::calculated, i.message};
        return p;
    };
    ThermoPropertiesSubstance tps;
    tps.gibbs_energy = one(r.values.G, r.info.G);
    tps.enthalpy = one(r.values.H, r.info.H);
    tps.entropy = one(r.values.S, r.info.S);
    tps.heat_capacity_cp = one(r.values.Cp, r.info.Cp);
    tps.volume = one(r.values.V, r.info.V, 1e5);  // 1 J/bar = 1e-5 m3
    return tps;
}

/// The properties that follow from the others (exact derivatives and propagated errors), for the pressure P (Pa) and the temperature T (K):
///   U = H - P V,  A = U - T S,  Cv = Cp - T V alpha^2/beta = Cp - T V_T^2/(-V_P)
/// with V in J/bar (V_T, V_P: the derivatives of V). The derivatives of Cv are those of Cp (the second derivatives of the volume
/// are not available): they are exact when the volume does not depend on T (Cv = Cp). Cv of the solvent is the one of its model.
auto ThermoEngine::Impl::completeThermoProperties(ThermoPropertiesSubstance &tps, double T, double P, bool calculateCv) const -> void
{
    using Reaktoro_::Status;
    const double Pbar = P / bar_to_Pa;
    auto defined = [](const Reaktoro_::ThermoProperty& x) { return x.sta.first != Status::notdefined; };

    auto &H = tps.enthalpy, &V = tps.volume, &S = tps.entropy, &U = tps.internal_energy, &A = tps.helmholtz_energy;
    if (defined(H) && defined(V))
    {
        const double uv = H.val() - Pbar*V.val(), ut = H.ddt() - Pbar*V.ddt(), up = H.ddp() - V.val()/bar_to_Pa - Pbar*V.ddp();
        const auto sta = U.sta;
        U.propagateFrom(H, V);
        U.setError({{1.0, H.err}, {Pbar, V.err}});
        U.val = uv; U.ddt = ut; U.ddp = up;
        if (!Reaktoro_::kWithAutodiff) U.ddt = U.ddp = 0.0;
        if (sta.first == Status::notdefined) U.sta = {Status::calculated, std::string("")};
    }
    if (defined(U) && defined(S))
    {
        const double av = U.val() - T*S.val(), at = U.ddt() - S.val() - T*S.ddt(), ap = U.ddp() - T*S.ddp();
        A.propagateFrom(U, S);
        A.setError({{1.0, U.err}, {T, S.err}});
        A.val = av; A.ddt = at; A.ddp = ap;
        if (!Reaktoro_::kWithAutodiff) A.ddt = A.ddp = 0.0;
    }
    auto &Cp = tps.heat_capacity_cp, &Cv = tps.heat_capacity_cv;
    if (calculateCv && !Reaktoro_::kWithAutodiff)
    {   // Cv needs dV/dT and dV/dP
        Cv = Reaktoro_::ThermoProperty();
        Cv.sta = {Status::notdefined, "Cv needs the derivatives of the volume: not calculated (built without autodiff)"};
    }
    else if (calculateCv && defined(Cp) && defined(V))
    {
        double diff = 0.0;                                 // Cp - Cv
        if (V.ddp() < 0.0 && V.ddt() != 0.0)
            diff = 1e-5 * T * V.ddt()*V.ddt() / (-V.ddp());
        Cv.propagateFrom(Cp, V);
        Cv.setError({{1.0, Cp.err}});
        Cv.val = Cp.val() - diff; Cv.ddt = Cp.ddt(); Cv.ddp = Cp.ddp();
    }
}

auto ThermoEngine::Impl::kindOf(const Substance &substance) const -> SubstanceKind
{
    const auto record = substance.record();
    SubstanceKind kind;
    kind.isHydrogen = v2::isZeroHydrogenIon(*record, database.thermoDataSet().convention);
    kind.isH2Ovapor = compat::isWaterVapour(*record);
    kind.isH2OSolvent = record->class_ == v2::SubstanceClass::WaterSolvent;
    kind.isReacDC = substance.thermoCalculationType() == SubstanceThermoCalculationType::type::REACDC ||
                    record->preferred_property_source == v2::PropertySource::Reaction || record->methods.empty();

    // the source can replace the rule, if the record has what that source needs
    const auto source = sourceOf(record->symbol, record->preferred_property_source);
    const bool hasMethods = !record->methods.empty() && !(record->methods.size() == 1 && record->methods[0].method == "ConstantVolume");
    if (source == v2::PropertySource::Reaction && !record->defining_reaction.empty())
        kind.isReacDC = true;
    else if (source == v2::PropertySource::Direct && hasMethods)
        kind.isReacDC = false;

    preferences.solvent_state = v2::solventStateOf(*record);

    // see if the solvent of an aqueous solute is in the database; if not, search for a solvent
    if (record->class_ == v2::SubstanceClass::AqueousSolute && !database.containsSubstance(preferences.solvent_symbol))
    {
        std::map<std::string, v2::SubstanceClass> classes;
        for (const auto& pair : database.mapSubstances())
            classes[pair.first] = pair.second.substanceClass() == SubstanceClass::type::AQSOLVENT ? v2::SubstanceClass::WaterSolvent : v2::SubstanceClass::Other;
        const auto chosen = v2::chooseSolvent(preferences.solvent_symbol, "", classes);
        if (!chosen.empty())
            preferences.solvent_symbol = chosen;
    }
    return kind;
}

auto ThermoEngine::Impl::thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
{
    return thermoPropertiesSubstance(T, P, database.getSubstance(substance));
}

auto ThermoEngine::Impl::detailedSubstance(double T, double &P, const Substance &substance) const -> DetailedPropertiesSubstance
{
    DetailedPropertiesSubstance detailed;
    detailed.properties = toOutput(thermoPropertiesSubstance(T, P, substance, &detailed), T, P, substance, &detailed);
    return detailed;
}

/// The properties in the canonical convention. toOutput gives them in the convention that the user asks for; reactions use these.
auto ThermoEngine::Impl::thermoPropertiesSubstance(double T, double &P, const Substance &substance, DetailedPropertiesSubstance* detailed) const -> ThermoPropertiesSubstance
{
    // where the result came from, if it is asked for
    const auto cameFrom = [&](const char* source) { if (detailed) detailed->source = source; };
    cameFrom("none");
    const SubstanceKind kind = kindOf(substance);
    ThermoPropertiesSubstance tps;

    if (kind.isHydrogen)
    {
        tps.volume = 0.0;
        tps.entropy = 0.0;
        tps.enthalpy = 0.0;
        tps.gibbs_energy = 0.0;
        tps.internal_energy = 0.0;
        tps.heat_capacity_cp = 0.0;
        tps.heat_capacity_cv = 0.0;
        tps.helmholtz_energy = 0.0;
        cameFrom("hydrogen_ion");
        return tps;
    }

    const bool isWater = kind.isH2OSolvent || kind.isH2Ovapor;

    // without a reaction, a substance that should have one has its reference properties
    if (kind.isReacDC && !substance.reactionSymbol().empty())
    {
        tps = reacDCthermoProperties(T, P, substance);
        completeThermoProperties(tps, T, P, !isWater);
        cameFrom("reaction");
        if (detailed)
            detailed->reaction = substance.reactionSymbol();
    }
    else
    {
        bool water_g_contains_rt_ln_p = false; // true if G of the water model has the term RT ln(P/1 bar)

        if (kind.isReacDC)
        {
            tps = canonicalReference(substance);
            cameFrom("reference");
        }
        else if (isWater)
        {
            const auto solvent = boundSolvent(substance);
            if (solvent->equation_of_state)
            {
                tps = solvent->equation_of_state->substanceProperties(T, P, preferences.solvent_state, database.conventions().water_triple_point_reference);
                water_g_contains_rt_ln_p = solvent->equation_of_state->gibbsEnergyContainsRTlnP();
                cameFrom("water");
            }
            else if (solvent->reference_values)
            {
                tps = toThermoProperties(v2::executeSubstance(*solvent->reference_values, T, P, substanceOptions()));
                if (kind.isH2OSolvent && declaredConvention(*substance.record()).water_reference == v2::WaterReference::SteamTables)
                    steamTablesShift(tps, 1.0);
                cameFrom("reference");
            }
        }
        else if (const auto bound = boundMethods(substance))
        {
            const auto solvent = solventInputs(*bound, T, P);
            std::vector<v2::ExplainStep> steps;
            tps = toThermoProperties(v2::executeSubstance(*bound, T, P, substanceOptions(), detailed ? &steps : nullptr, solvent ? &*solvent : nullptr));
            cameFrom("methods");
            if (detailed)
                for (std::size_t i = 0; i < bound->order.size(); ++i)
                    if (!bound->to_canonical[i].identity())
                        detailed->conversions.push_back(bound->order[i]->spec().id + " to canonical");
            if (detailed)
                for (const auto& step : steps)
                    detailed->steps.push_back({step.method, step.values.G.val, step.values.H.val, step.values.S.val, step.values.Cp.val, step.values.V.val});
        }

        // With the fugacity coefficient kept (switch false), G of steam is G(T, 1 bar) + RT ln(phi) like the other fluids: RT ln(P) that the
        // HGK equation of state contains in G is removed (H and S are not changed)
        if (water_g_contains_rt_ln_p && kind.isH2Ovapor && !kind.isH2OSolvent && !preferences.cancel_fugacity_coefficient_in_gas_props &&
            substance.aggregateState() == AggregateState::type::GAS && P > 0.0)
        {
            const double Pb = P / bar_to_Pa;
            tps.gibbs_energy.val -= R_CONSTANT * T * std::log(Pb);
            tps.gibbs_energy.ddt -= R_CONSTANT * std::log(Pb);
            tps.gibbs_energy.ddp -= R_CONSTANT * T / P;
        }

        // the properties that follow from the others (U, A, Cv), in the convention of the models
        completeThermoProperties(tps, T, P, !isWater);
    }

    // check properties
    tps = fallbackThermoPropertiesSubstance(tps, substance);

    return tps;
}

auto ThermoEngine::Impl::fallbackThermoPropertiesSubstance(ThermoPropertiesSubstance tps, Substance subst) const -> ThermoPropertiesSubstance
{
    auto tpref = canonicalReference(subst);
    // leave volume as is in substance record if no function to calculate it exists
    if (preferences.fallback_to_reference_properties)
    {
        if (tps.volume.sta.first == Reaktoro_::Status::notdefined)
            tps.volume = tpref.volume;
        if (tps.gibbs_energy.sta.first == Reaktoro_::Status::notdefined)
            tps.gibbs_energy = tpref.gibbs_energy;
        if (tps.enthalpy.sta.first == Reaktoro_::Status::notdefined)
            tps.enthalpy = tpref.enthalpy;
        if (tps.entropy.sta.first == Reaktoro_::Status::notdefined)
            tps.entropy = tpref.entropy;
        if (tps.heat_capacity_cp.sta.first == Reaktoro_::Status::notdefined)
            tps.heat_capacity_cp = tpref.heat_capacity_cp;
        if (tps.heat_capacity_cv.sta.first == Reaktoro_::Status::notdefined)
            tps.heat_capacity_cv = tpref.heat_capacity_cv;
        if (tps.internal_energy.sta.first == Reaktoro_::Status::notdefined)
            tps.internal_energy = tpref.internal_energy;
        if (tps.helmholtz_energy.sta.first == Reaktoro_::Status::notdefined)
            tps.helmholtz_energy = tpref.helmholtz_energy;
    }
    return tps;
}

auto ThermoEngine::Impl::electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
{
    return electroPropertiesSolvent(T, P, database.getSubstance(solvent), state);
}

auto ThermoEngine::Impl::electroPropertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> ElectroPropertiesSolvent
{
    const SubstanceKind kind = kindOf(solvent);
    if (state>-1 && state<2)
        preferences.solvent_state = state;
    PropertiesSolvent ps = propertiesSolvent(T, P, solvent, preferences.solvent_state);
    ElectroPropertiesSolvent eps;

    if (kind.isH2OSolvent)
        if (const auto bound = boundSolvent(solvent); bound->dielectric)
            eps = bound->dielectric->properties(T, P, ps, preferences.solvent_state);
    return eps;
}

auto ThermoEngine::Impl::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    return propertiesSolvent(T, P, database.getSubstance(solvent), state);
}

auto ThermoEngine::Impl::propertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> PropertiesSolvent
{
    const SubstanceKind kind = kindOf(solvent);
    if (state > -1 && state <2)
        preferences.solvent_state = state;
    PropertiesSolvent ps;

    if (kind.isH2OSolvent)
        if (const auto bound = boundSolvent(solvent); bound->equation_of_state)
            ps = bound->equation_of_state->properties(T, P, preferences.solvent_state);
    return ps;
}

auto ThermoEngine::Impl::reacDCthermoProperties(double T, double &P, Substance subst) const -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps, reacTps;
    ThermoPropertiesReaction tpr;
    const std::string reactionSymbol = subst.reactionSymbol();
    const Reaction reaction = database.getReaction(reactionSymbol);
    std::map<std::string, double> reactants;

    const v2::DefiningReactionGuard no_cycle(subst.symbol());

    // if reaction involves gases we switch on the P correction to the thermo props
    FlagGuard gas_pressure_correction(preferences.apply_pressure_correction_to_gas_props, true);

    tpr = cachedReaction(T, P, reactionSymbol); /*thermoPropertiesReaction(T, P, reactionSymbol);*/

    reactants = reaction.reactants();

    // the properties of the other reactants
    std::vector<std::pair<ThermoPropertiesSubstance, double>> others;
    for (const auto& reactant : reactants)
    {
        if (reactant.first != subst.symbol())
            others.push_back({cachedSubstance(T, P, reactant.first) /* thermoPropertiesSubstance(T, P, reactant.first);*/, reactant.second});
    }

    const double coeff = reactants[subst.symbol()];

    // the properties of the substance from the properties of the reaction and of the other reactants
    tps = v2::substanceFromReaction(T, P, tpr, others, coeff);

    gas_pressure_correction.release();

    // a gas defined by a reaction and without a volume method has the ideal gas molar volume
    if (subst.aggregateState() == AggregateState::type::GAS &&
        subst.substanceClass() == SubstanceClass::type::GASFLUID && !subst.method_P() && P > 0.0 &&
        !(tpr.reaction_volume.sta.first != Reaktoro_::Status::notdefined && tpr.reaction_volume.val != 0.0))
    {
        v2::setIdealGasVolume(tps, T, P);
    }

    // pressure correction on rdc assuming constant molar volume
    if (hasMethod(subst.record()->methods, "ConstantVolume") &&
        !hasMethod(reaction.record()->methods, "ReactionConstantVolume") && !hasMethod(reaction.record()->methods, "ReactionVolumePolynomial"))
    {
        // check properites
        tps = fallbackThermoPropertiesSubstance(tps, subst);
        tps = v2::constantVolumeCorrection(T, P, tps, subst.thermoReferenceProperties(), subst.referenceP());
    }

    return tps;
}

} // namespace ThermoFun
