// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Binding.h"

#include <algorithm>
#include <string>
#include <tuple>

#include <ThermoFun/Conventions/HydrogenIon.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodRegistry.h>

namespace ThermoFun::v2 {

namespace {

// The group of a method in the order of execution
auto groupOf(const MethodSpec& spec) -> int
{
    const auto& e = spec.effects;
    if (e.base)
        return 0;
    if (e.transform)
        return 1;
    const bool sets = e.G == Effect::Set || e.H == Effect::Set || e.S == Effect::Set || e.Cp == Effect::Set || e.V == Effect::Set;
    return sets ? 2 : 3;
}

// "aqueous_solute" -> "aqueous solute"
auto classWords(SubstanceClass c) -> std::string
{
    auto w = toString(c);
    std::replace(w.begin(), w.end(), '_', ' ');
    return w;
}

auto problem(const SubstanceRecord& record, const std::string& code, const std::string& message, const std::string& fix) -> Diagnostic
{
    Diagnostic d;
    d.severity = diagnosticInfo(code)->severity;
    d.code = code;
    d.record = record.symbol;
    d.path = "/substances/" + record.symbol + "/methods";
    d.message = message;
    d.fix = fix;
    return d;
}

} // namespace

auto canBind(const SubstanceRecord& record) -> bool
{
    int bases = 0;
    for (const auto& a : record.methods)
    {
        if (!MethodRegistry::instance().hasSubstanceMethod(a.method))
            return false;
        const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Substance);
        if (!spec)
            return false;
        bases += spec->effects.base;
    }
    return bases == 1;
}

auto checkMethods(const SubstanceRecord& record) -> Diagnostics
{
    Diagnostics out;
    std::vector<const MethodSpec*> specs;
    for (const auto& a : record.methods)
        if (const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Substance))
            specs.push_back(spec);

    std::vector<std::string> bases;
    for (const auto* spec : specs)
        if (spec->effects.base)
            bases.push_back(spec->id);
    const bool fromReaction = !record.defining_reaction.empty() || record.preferred_property_source == PropertySource::Reaction;
    // the geochemical convention defines the properties of the hydrogen ion as 0: it needs no values there
    if (bases.empty() && !fromReaction && record.standard_state_properties.empty() && !isZeroHydrogenIon(record, Convention{}))
        out.add(problem(record, "V2-MTH-012", record.symbol + " has no base method and no reference values.",
                        "Add a base method (for example CpPolynomial) or the standard-state properties G, H, S, Cp, V."));
    if (bases.size() > 1)
        out.add(problem(record, "V2-MTH-012", record.symbol + " has two base methods: " + bases[0] + " and " + bases[1] + ".", "Remove one of them."));

    // the corrections, in the order of execution; a base method sets all properties and is not one of them
    std::vector<const MethodSpec*> corrections;
    for (const auto* spec : specs)
        if (!spec->effects.base)
            corrections.push_back(spec);
    std::sort(corrections.begin(), corrections.end(), [](const auto* a, const auto* b) {
        return std::make_tuple(groupOf(*a), a->effects.rank, a->id) < std::make_tuple(groupOf(*b), b->effects.rank, b->id);
    });
    struct Property { const char* name; Effect MethodEffects::*effect; };
    const Property properties[] = {{"Gibbs energy", &MethodEffects::G}, {"enthalpy", &MethodEffects::H}, {"entropy", &MethodEffects::S},
                                   {"heat capacity", &MethodEffects::Cp}, {"volume", &MethodEffects::V}};
    for (const auto& p : properties)
    {
        std::vector<std::string> setters;
        for (const auto* spec : corrections)
            if (spec->effects.*p.effect == Effect::Set)
                setters.push_back(spec->id);
        if (setters.size() > 1)
            out.add(problem(record, "V2-MTH-013", setters[0] + " and " + setters[1] + " both set the " + p.name + " of " + record.symbol + ".",
                            std::string("Remove one, or use a method that adds to the ") + p.name + "."));
    }

    bool volumeDefined = record.standard_state_properties.count(PropertyName::V) > 0;
    for (const auto* spec : corrections)
    {
        if (spec->effects.V == Effect::Add && !volumeDefined)
            out.add(problem(record, "V2-MTH-014", spec->id + " adds to a volume that is not defined.", "Give V in the standard-state properties."));
        volumeDefined = volumeDefined || spec->effects.V == Effect::Set;
    }

    for (const auto* spec : specs)
    {
        const auto& allowed = spec->applies_to_classes;
        if (allowed.empty() || std::find(allowed.begin(), allowed.end(), record.class_) != allowed.end())
            continue;
        std::string classes;
        for (size_t i = 0; i < allowed.size(); ++i)
            classes += (i ? " or " : "") + classWords(allowed[i]) + "s";
        const auto own = classWords(record.class_);
        const char* article = std::string("aeiou").find(own[0]) == std::string::npos ? "a " : "an ";
        out.add(problem(record, "V2-MTH-005", spec->id + " is for " + classes + "; " + record.symbol + " is " + article + own + ".",
                        "Use a method for this class, or change the class."));
    }
    return out;
}

auto bindSubstance(std::shared_ptr<const SubstanceRecord> record, const BindConventions& conventions) -> std::shared_ptr<const BoundSubstance>
{
    auto bound = std::make_shared<BoundSubstance>();
    for (const auto& a : record->methods)
        bound->order.push_back(MethodRegistry::instance().create(a, *record));
    std::stable_sort(bound->order.begin(), bound->order.end(), [](const auto& a, const auto& b) {
        return std::make_tuple(groupOf(a->spec()), a->spec().effects.rank, a->spec().id) <
               std::make_tuple(groupOf(b->spec()), b->spec().effects.rank, b->spec().id);
    });

    ConversionSubject subject;
    subject.Tr = record->reference_conditions.T;
    subject.gas = record->aggregate_state == AggregateState::Gas;
    subject.solute = record->class_ == SubstanceClass::AqueousSolute;
    subject.element_entropy = conventions.element_entropy;
    const Convention inherited = mergeConvention(record->convention.value_or(Convention{}), conventions.dataset);
    for (const auto& method : bound->order)
    {
        const auto& spec = method->spec();
        Convention declared;
        declared.energy_reference = spec.native_convention;
        if (spec.effects.base)
            declared = mergeConvention(declared, inherited);
        bound->to_canonical.push_back(spec.effects.G == Effect::Set ? shiftToCanonical(declared, subject) : ConventionShift{});
        bound->needs.solvent = bound->needs.solvent || bound->to_canonical.back().needsSolvent();
    }

    for (const auto& method : bound->order)
    {
        const auto& n = method->spec().needs;
        bound->needs.solvent = bound->needs.solvent || n.solvent;
        bound->needs.dielectric = bound->needs.dielectric || n.dielectric;
        bound->needs.solvent_at_reference = bound->needs.solvent_at_reference || n.solvent_at_reference;
        bound->needs.ideal_gas_water = bound->needs.ideal_gas_water || n.ideal_gas_water;
    }

    bound->reference.T = record->reference_conditions.T;
    bound->reference.P = record->reference_conditions.P;
    auto load = [&](PropertyName name, std::optional<double>& value, ValueInfo& info) {
        const auto it = record->standard_state_properties.find(name);
        if (it == record->standard_state_properties.end())
            return;
        value = it->second.value;
        info.status = ValueInfo::Status::Read;
        if (it->second.uncertainty)
            info.err = it->second.uncertainty->standard;
    };
    load(PropertyName::G, bound->reference.G, bound->reference_info.G);
    load(PropertyName::H, bound->reference.H, bound->reference_info.H);
    load(PropertyName::S, bound->reference.S, bound->reference_info.S);
    load(PropertyName::Cp, bound->reference.Cp, bound->reference_info.Cp);
    load(PropertyName::V, bound->reference.V, bound->reference_info.V);
    bound->record = std::move(record);
    return bound;
}

} // namespace ThermoFun::v2
