// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The properties of substances and of the solvent, in the canonical convention.

#include "ThermoEngineImpl.h"
#include <ThermoFun/Methods/Water/Reaktoro/WaterUtils.hpp>
#include <ThermoFun/Methods/Water/Reaktoro/WaterHelmholtzConstants.hpp>

namespace ThermoFun
{

/// Where the properties of a record come from: the source of the symbol, else the preference of the engine, else the
/// record. Empty: the rule of the record.
auto ThermoEngine::Impl::sourceOf(const std::map<std::string, model::PropertySource>& own, const std::string& symbol, const std::optional<model::PropertySource>& preferred) const -> std::optional<model::PropertySource>
{
    const auto it = own.find(symbol);
    if (it != own.end())
        return it->second;
    if (preferences.property_source != model::PropertySource::Record)
        return preferences.property_source;
    return preferred;
}

/// A record with the implementations of the preferences
auto ThermoEngine::Impl::withImplementations(std::shared_ptr<const model::SubstanceRecord> record) const -> std::shared_ptr<const model::SubstanceRecord>
{
    record = model::withParameterSets(std::move(record), database.parameterSets());
    if (preferences.method_implementation.empty())
        return record;
    auto copy = std::make_shared<model::SubstanceRecord>(*record);
    for (const auto& [id, implementation] : preferences.method_implementation)
        for (auto& a : copy->methods)
            if (a.method == model::MethodCatalog::instance().find(id)->id)
                a.implementation = implementation;
    return copy;
}

/// The methods of a substance. Null if they give no one base method: then no method runs.
auto ThermoEngine::Impl::boundMethods(const Substance& substance) const -> std::shared_ptr<const model::BoundSubstance>
{
    const auto record = substance.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->substances.find(record.get());
    if (it != bound_cache->substances.end())
        return it->second;
    const auto formula = record->formula;
    const model::BindConventions bind{database.thermoDataSet().convention, &database.methods(), [this, formula] { return elementEntropy(formula); }};
    auto bound = model::canBind(*record, &database.methods()) ? model::bindSubstance(withImplementations(record), bind) : nullptr;
    bound_cache->substances[record.get()] = bound;
    bound_cache->records.push_back(record);
    return bound;
}

/// The state of the solvent that the methods of a record need, calculated with the methods of the solvent record.
/// A pressure of 0 is the saturation pressure: it is written to P.
auto ThermoEngine::Impl::solventInputs(const model::BoundSubstance& bound, double T, double& P) const -> std::optional<model::SolventInputs>
{
    if (!bound.needs.solvent)
        return std::nullopt;
    model::SolventInputs in;
    const auto symbol = solvent_in_use;
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
auto ThermoEngine::Impl::boundSolvent(const Substance& substance) const -> std::shared_ptr<const model::BoundSolvent>
{
    const auto record = substance.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->solvents.find(record.get());
    if (it != bound_cache->solvents.end())
        return it->second;
    auto bound = model::bindSolvent(withImplementations(record));
    bound_cache->solvents[record.get()] = bound;
    bound_cache->records.push_back(record);
    return bound;
}

auto ThermoEngine::Impl::substanceOptions() const -> model::SubstanceOptions
{
    return {preferences.apply_pressure_correction_to_gas_props, preferences.cancel_fugacity_coefficient_in_gas_props};
}

/// The result of the executor in the form of the old engine: V in J/bar, and the status of each property
auto ThermoEngine::Impl::toThermoProperties(const model::SubstanceResult& r) -> ThermoPropertiesSubstance
{
    auto one = [](const model::PropertyValue& v, const model::ValueInfo& i, double factor = 1.0) {
        Reaktoro_::ThermoProperty p;
        p.val = v.val * factor;
        p.ddt = v.ddt * factor;
        p.ddp = v.ddp * factor;
        p.err = i.err * factor;
        p.sta = {i.status == model::ValueInfo::Status::NotDefined ? Reaktoro_::Status::notdefined
                 : i.status == model::ValueInfo::Status::Assigned ? Reaktoro_::Status::assigned
                 : i.status == model::ValueInfo::Status::Read ? Reaktoro_::Status::read : Reaktoro_::Status::calculated, i.message};
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
        if (sta.first == Status::notdefined) U.sta = {Status::calculated, std::string("")};
    }
    if (defined(U) && defined(S))
    {
        const double av = U.val() - T*S.val(), at = U.ddt() - S.val() - T*S.ddt(), ap = U.ddp() - T*S.ddp();
        A.propagateFrom(U, S);
        A.setError({{1.0, U.err}, {T, S.err}});
        A.val = av; A.ddt = at; A.ddp = ap;
    }
    auto &Cp = tps.heat_capacity_cp, &Cv = tps.heat_capacity_cv;
    if (calculateCv && defined(Cp) && defined(V))
    {
        double diff = 0.0;                                 // Cp - Cv
        if (V.ddp() < 0.0 && V.ddt() != 0.0)
            diff = 1e-5 * T * V.ddt()*V.ddt() / (-V.ddp());
        Cv.propagateFrom(Cp, V);
        Cv.setError({{1.0, Cp.err}});
        Cv.val = Cp.val() - diff; Cv.ddt = Cp.ddt(); Cv.ddp = Cp.ddp();
    }
}

auto ThermoEngine::Impl::comesFromReaction(const Substance &substance) const -> bool
{
    const auto record = substance.record();
    bool fromReaction = substance.thermoCalculationType() == SubstanceThermoCalculationType::type::REACDC ||
                        record->preferred_property_source == model::PropertySource::Reaction || record->methods.empty();
    // no base method: only corrections, which work on what the reaction gives
    if (!record->defining_reaction.empty() && !record->methods.empty() && !hasBaseMethod(*record))
        fromReaction = true;

    // the source can replace the rule, if the record has what that source needs
    const auto source = sourceOf(substance_sources, record->symbol, record->preferred_property_source);
    if (source == model::PropertySource::Reaction && !record->defining_reaction.empty())
        fromReaction = true;
    else if (source == model::PropertySource::Direct && hasBaseMethod(*record))
        fromReaction = false;
    return fromReaction;
}

auto ThermoEngine::Impl::kindOf(const Substance &substance) const -> SubstanceKind
{
    const auto record = substance.record();
    SubstanceKind kind;
    kind.isHydrogen = model::isZeroHydrogenIon(*record, database.thermoDataSet().convention);
    kind.isH2Ovapor = compat::isWaterVapour(*record);
    kind.isH2OSolvent = record->class_ == model::SubstanceClass::WaterSolvent;
    kind.isReacDC = comesFromReaction(substance);
    kind.state = model::solventStateOf(*record);
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
                tps = solvent->equation_of_state->substanceProperties(T, P, kind.state, database.conventions().water_triple_point_reference);
                water_g_contains_rt_ln_p = solvent->equation_of_state->gibbsEnergyContainsRTlnP();
                cameFrom("water");
            }
            else if (solvent->reference_values)
            {
                tps = toThermoProperties(model::executeSubstance(*solvent->reference_values, T, P, substanceOptions()));
                if (kind.isH2OSolvent && declaredConvention(*substance.record()).water_reference == model::WaterReference::SteamTables)
                    steamTablesShift(tps, T, 1.0);
                cameFrom("reference");
            }
        }
        else if (const auto bound = boundMethods(substance))
        {
            // P = 0 is the saturation pressure of water for every record: a record that does not use water takes it here
            // (one that does gets it from the water, in solventInputs), and writes it to P like the water does
            if (P == 0.0 && !bound->needs.solvent)
                P = saturationPressure(T);
            const auto solvent = solventInputs(*bound, T, P);
            std::vector<model::ExplainStep> steps;
            tps = toThermoProperties(model::executeSubstance(*bound, T, P, substanceOptions(), detailed ? &steps : nullptr, solvent ? &*solvent : nullptr));
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
        if (water_g_contains_rt_ln_p && kind.isH2Ovapor && !kind.isH2OSolvent && !preferences.apply_pressure_correction_to_gas_props && !preferences.cancel_fugacity_coefficient_in_gas_props &&
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
    if (state < 0 || state > 1)
        state = kind.state;  // the state of the record
    PropertiesSolvent ps = propertiesSolvent(T, P, solvent, state);
    ElectroPropertiesSolvent eps;

    if (kind.isH2OSolvent)
        if (const auto bound = boundSolvent(solvent); bound->dielectric)
            eps = bound->dielectric->properties(T, P, ps, state);
    return eps;
}

auto ThermoEngine::Impl::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    return propertiesSolvent(T, P, database.getSubstance(solvent), state);
}

auto ThermoEngine::Impl::propertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> PropertiesSolvent
{
    const SubstanceKind kind = kindOf(solvent);
    if (state < 0 || state > 1)
        state = kind.state;  // the state of the record
    PropertiesSolvent ps;

    if (kind.isH2OSolvent)
        if (const auto bound = boundSolvent(solvent); bound->equation_of_state)
            ps = bound->equation_of_state->properties(T, P, state);
    return ps;
}

auto ThermoEngine::Impl::reactionCarriesPressure(const Reaction& reaction) const -> bool
{
    const auto record = reaction.record();
    if (reactionFromReactants(reaction))
        return true;  // its species do
    const auto volume = record->reaction_properties.find(model::ReactionPropertyName::dV);
    const bool hasVolume = volume != record->reaction_properties.end() && volume->second.value != 0.0;
    for (const auto& a : record->methods)
    {
        const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Reaction);
        if (!spec || (spec->depends_on != model::Dependence::P && spec->depends_on != model::Dependence::TP))
            continue;
        // a model of log K has P in it; a method of the volume works on the reference volume of the reaction
        if (spec->reaction_effects.base || hasVolume)
            return true;
    }
    return false;
}

auto ThermoEngine::Impl::dataPressure(double T, const Substance& substance) const -> double
{
    double Psat = 0.0;
    if (!database.containsSubstance(solvent_in_use))
        return substance.referenceP();
    if (T < waterCriticalT())
        cachedSolvent(T, Psat, solvent_in_use, -1);
    else
        Psat = waterCriticalP();  // where the saturation curve ends
    const double P = std::max(substance.referenceP(), Psat);
    static const double rho_min = model::builtinConstants("DataPressure")->parameters.at("rho_min").value;  // kg/m3
    return rho_min > 0.0 ? pressureAtDensity(T, rho_min, P) : P;
}

auto ThermoEngine::Impl::saturationPressure(double T) const -> double
{
    if (database.containsSubstance(solvent_in_use))
    {
        double P = 0.0;
        cachedSolvent(T, P, solvent_in_use, -1);  // the water model writes its saturation pressure to P
        return P;
    }
    static const auto curve = [] {
        const auto& c = model::builtinConstants("WaterSaturation")->parameters;
        WaterSaturationConstants s;
        for (int i = 1; i <= 6; ++i)
        {
            s.a.push_back(c.at("sat_a" + std::to_string(i)).value);
            s.b.push_back(c.at("sat_b" + std::to_string(i)).value);
            s.c.push_back(c.at("sat_c" + std::to_string(i)).value);
        }
        s.Tcr = c.at("Tcr").value; s.Pcr = c.at("Pcr").value; s.Dcr = c.at("Dcr").value; s.M = c.at("M").value;
        return s;
    }();
    return static_cast<double>(waterSaturatedPressureWagnerPruss(real(T), curve));
}

auto ThermoEngine::Impl::pressureAtDensity(double T, double rho, double P) const -> double
{
    const auto bound = boundSolvent(database.getSubstance(solvent_in_use));
    if (!bound->equation_of_state)
        return P;
    auto density = [&](double p) { return static_cast<double>(bound->equation_of_state->properties(T, p, 0).density.val); };  // p is a copy: a model writes P
    // Below Tcr P is the saturation pressure: dense enough as long as the saturated liquid is. Above Tcr P is the critical
    // pressure, where the density is the critical one: an iteration for the density may not converge there, so it is not asked.
    static const double Dcr = model::builtinConstants("WaterSaturation")->parameters.at("Dcr").value;
    if ((T < waterCriticalT() || rho <= Dcr) && density(P) >= rho)
        return P;
    // a Helmholtz model gives P at (T, rho) directly; here rho is in the one-phase region
    if (const auto direct = bound->equation_of_state->pressureAtDensity(T, rho))
        return std::max(P, *direct);
    // else bisection: the density grows with P in one phase, also where a model switches equations (critical region)
    double low = P, high = 2.0 * P;
    while (density(high) < rho)
    {
        low = high;
        high *= 2.0;
        if (high > 1e11)
            return P;  // the water model does not reach this density
    }
    while (high - low > 1e-10 * high)
    {
        const double middle = 0.5 * (low + high);
        (density(middle) < rho ? low : high) = middle;
    }
    return high;
}

auto ThermoEngine::Impl::hasBaseMethod(const model::SubstanceRecord& record) -> bool
{
    return std::any_of(record.methods.begin(), record.methods.end(), [](const auto& a) {
        const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance);
        return spec && spec->effects.base;
    });
}

auto ThermoEngine::Impl::pressureFromRecord(const Substance& substance, const Reaction& reaction) const -> bool
{
    const auto& methods = substance.record()->methods;
    const bool own = std::any_of(methods.begin(), methods.end(), [](const auto& a) {
        const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance);
        return spec && !spec->effects.base && (spec->depends_on == model::Dependence::P || spec->depends_on == model::Dependence::TP);
    });
    return own && !reactionCarriesPressure(reaction);
}

auto ThermoEngine::Impl::pressureCorrections(const Substance& substance) const -> std::shared_ptr<const model::BoundSubstance>
{
    const auto record = substance.record();
    std::lock_guard<std::mutex> lock(bound_cache->mutex);
    const auto it = bound_cache->corrections.find(record.get());
    if (it != bound_cache->corrections.end())
        return it->second;
    // ConstantVolume starts from the pressure of the data: see reacDCthermoProperties
    auto only = std::make_shared<model::SubstanceRecord>(*withImplementations(record));
    only->methods.erase(std::remove_if(only->methods.begin(), only->methods.end(), [](const auto& a) {
        const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance);
        return !spec || spec->effects.base || a.method == "ConstantVolume" || !model::MethodRegistry::instance().hasSubstanceMethod(a.method) ||
               (spec->depends_on != model::Dependence::P && spec->depends_on != model::Dependence::TP);
    }), only->methods.end());
    auto bound = model::bindSubstance(only);
    bound_cache->corrections[record.get()] = bound;
    bound_cache->records.push_back(record);
    return bound;
}

auto ThermoEngine::Impl::valuesOf(const ThermoPropertiesSubstance& tps) -> model::MethodResult
{
    auto one = [](const Reaktoro_::ThermoProperty& p, double factor = 1.0) { return model::PropertyValue{p.val * factor, p.ddt * factor, p.ddp * factor}; };
    model::MethodResult out;
    out.G = one(tps.gibbs_energy);
    out.H = one(tps.enthalpy);
    out.S = one(tps.entropy);
    out.Cp = one(tps.heat_capacity_cp);
    out.V = one(tps.volume, 1e-5);  // J/bar to m3/mol
    return out;
}

auto ThermoEngine::Impl::reacDCthermoProperties(double T, double &P, Substance subst) const -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    ThermoPropertiesReaction tpr;
    const std::string reactionSymbol = subst.reactionSymbol();
    const Reaction reaction = database.getReaction(reactionSymbol);
    std::map<std::string, double> reactants;

    const model::DefiningReactionGuard no_cycle(subst.symbol());

    // A record that gives its own pressure term, and a reaction that does not: all at the pressure of the data of the reaction, then that term. Else all at P.
    // Not the other species at P and then the term: their pressure effect would come into the substance twice.
    const bool ownTerm = pressureFromRecord(subst, reaction);
    if (ownTerm && P == 0.0 && T < waterCriticalT() && database.containsSubstance(solvent_in_use))
        cachedSolvent(T, P, solvent_in_use, -1);  // the saturation pressure
    double Pdata = dataPressure(T, subst);
    const bool fromData = ownTerm;
    double& Pcalc = fromData ? Pdata : P;  // the calculation writes the saturation pressure to P

    tpr = cachedReaction(T, Pcalc, reactionSymbol);

    reactants = reaction.reactants();

    // the properties of the other reactants
    std::vector<std::pair<ThermoPropertiesSubstance, double>> others;
    for (const auto& reactant : reactants)
    {
        if (reactant.first != subst.symbol())
            others.push_back({cachedSubstance(T, Pcalc, reactant.first), reactant.second});
    }

    const double coeff = reactants[subst.symbol()];

    // the properties of the substance from the properties of the reaction and of the other reactants
    tps = model::substanceFromReaction(T, Pcalc, tpr, others, coeff);
    if (!ownTerm)
        return tps;

    if (fromData)
        for (auto* field : fields(tps))
            field->ddp = 0.0;  // the values of the data do not change with P
    tps = fallbackThermoPropertiesSubstance(tps, subst);
    if (hasMethod(subst.record()->methods, "ConstantVolume"))
        tps = model::constantVolumeCorrection(T, P, tps, subst.thermoReferenceProperties(), fromData ? Pdata : subst.referenceP());
    if (const auto corrections = pressureCorrections(subst); corrections && !corrections->order.empty())
    {
        const auto solvent = solventInputs(*corrections, T, P);
        const auto out = model::executeOn(valuesOf(tps), *corrections, T, P, substanceOptions(), solvent ? &*solvent : nullptr);
        auto keep = [](Reaktoro_::ThermoProperty& to, const model::PropertyValue& value, double factor = 1.0) {
            to.val = value.val * factor;
            to.ddt = value.ddt * factor;
            to.ddp = value.ddp * factor;
        };
        keep(tps.gibbs_energy, out.G);
        keep(tps.enthalpy, out.H);
        keep(tps.entropy, out.S);
        keep(tps.heat_capacity_cp, out.Cp);
        keep(tps.volume, out.V, 1e5);  // 1 J/bar = 1e-5 m3
        tps.volume.sta = {Reaktoro_::Status::calculated, ""};
    }
    return tps;
}

} // namespace ThermoFun
