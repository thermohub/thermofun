// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyWriter.h"

#include <algorithm>
#include <cmath>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/Methods/MethodCatalog.h>

#include "LegacyKeys.h"
#include "LegacyMethodMap.h"
#include "LegacyVocabulary.h"

namespace ThermoFun::v2 {

namespace {

using json = nlohmann::ordered_json;
using vd = std::vector<double>;
using LM = legacy::LegacyMethod;

void warn(Diagnostics* diagnostics, const std::string& record, const std::string& message)
{
    if (!diagnostics)
        return;
    Diagnostic d;
    d.severity = Severity::Warning;
    d.code = "LEG-WRT-001";
    d.record = record;
    d.message = message;
    diagnostics->add(d);
}

auto perMole(const std::string& unit) -> std::string
{
    return unit == "J/bar" ? "J/(mol*bar)" : unit;
}

auto siUnitOf(const MethodSpec& spec, const std::string& name) -> std::string
{
    for (const auto& p : spec.parameters)
        if (p.name == name)
            return p.si_unit;
    return "1";
}

// The legacy method of an application: its written name, if that method has the same id; else the first of the id and implementation
auto legacyOf(const MethodApplication& a, const std::vector<LM>& all) -> const LM*
{
    for (const auto& m : all)
        if (a.method_as_written == m.name && a.method == m.method)
            return &m;
    for (const auto& m : all)
        if (a.method == m.method && (a.implementation.empty() || a.implementation == m.implementation))
            return &m;
    return nullptr;
}

auto methodEntry(const LM& m) -> json
{
    json entry = json::object();
    entry[legacy::Method] = json{{std::to_string(m.id), m.name}};
    return entry;
}

// The arrays of a method from its named parameters: {values, errors, units}. A place without a value is 0.
void addArrays(json& entry, const LM& method, const MethodSpec& spec, const std::map<std::string, Quantity>& parameters)
{
    std::vector<std::string> keys;
    for (const auto& p : method.parameters)
        if (std::find(keys.begin(), keys.end(), p.key) == keys.end())
            keys.push_back(p.key);
    for (const auto& key : keys)
    {
        vd values, errors;
        std::vector<std::string> units;
        bool anyError = false;
        for (const auto& p : method.parameters)
        {
            if (key != p.key)
                continue;
            const size_t i = static_cast<size_t>(p.index);
            if (values.size() <= i)
            {
                values.resize(i + 1, 0.0);
                errors.resize(i + 1, 0.0);
                units.resize(i + 1, "1");
            }
            units[i] = p.unit;
            const auto it = parameters.find(p.name);
            if (it == parameters.end())
                continue;
            const std::string si = siUnitOf(spec, p.name);
            values[i] = units::convertAny(it->second.value, si, perMole(p.unit));
            if (it->second.uncertainty && it->second.uncertainty->standard > 0.0)
            {
                errors[i] = std::fabs(units::convertDifference(it->second.uncertainty->standard, si, perMole(p.unit)));
                anyError = true;
            }
        }
        // an array of which the record has no value: no key
        const bool any = std::any_of(method.parameters.begin(), method.parameters.end(), [&](const auto& p) { return key == p.key && parameters.count(p.name); });
        if (!any)
            continue;
        json array = json::object();
        array["values"] = values;
        if (anyError)
            array["errors"] = errors;
        array["units"] = units;
        entry[key] = array;
    }
}

auto limitsJson(const Applicability& a, bool range = false) -> json
{
    json l = json::object();
    if (a.t_min) l["lowerT"] = *a.t_min;
    if (a.t_max) l["upperT"] = *a.t_max;
    if (a.p_min) l["lowerP"] = *a.p_min;
    if (a.p_max) l["upperP"] = *a.p_max;
    l["range"] = range || a.is_hard;
    return l;
}

auto hasLimits(const Applicability& a) -> bool
{
    return a.t_min || a.t_max || a.p_min || a.p_max;
}

template <class Key>
void addProperty(json& out, const char* key, const std::map<Key, Quantity>& properties, Key name, const char* unit, double factor = 1.0)
{
    const auto it = properties.find(name);
    if (it == properties.end())
        return;
    json p = json::object();
    p["values"] = vd{it->second.value * factor};
    if (it->second.uncertainty && it->second.uncertainty->standard > 0.0)
        p["errors"] = vd{it->second.uncertainty->standard * factor};
    if (unit)
        p["units"] = std::vector<std::string>{unit};
    out[key] = p;
}

template <class Properties>
auto hasValueData(const Properties& properties) -> bool
{
    return std::any_of(properties.begin(), properties.end(), [](const auto& p) {
        const auto& m = p.second.meta;
        return m.quality || m.origin || !m.calculation_path.empty() || !m.source.empty();
    });
}

template <class Record>
void reportCommon(const Record& r, Diagnostics* diagnostics)
{
    if (!r.data_source.empty())
        warn(diagnostics, r.symbol, "The key data_source is not written.");
    if (r.convention)
        warn(diagnostics, r.symbol, "The convention of the record is not written.");
    if (!r.covariance.empty())
        warn(diagnostics, r.symbol, "The covariance blocks are not written.");
    if (r.preferred_property_source == PropertySource::Direct && !r.methods.empty())
        warn(diagnostics, r.symbol, "The key preferred_property_source is not written.");
}

auto sourcesOf(const std::vector<std::string>& sources) -> std::vector<std::string>
{
    return sources;
}

} // namespace

auto writeLegacySubstance(const SubstanceRecord& r, const LegacyWriteOptions& options, Diagnostics* diagnostics) -> std::string
{
    if (options.use_original_text && r.source_format == DataFormat::Legacy && !r.original_text.empty())
        return r.original_text;

    json out = json::object();
    out[legacy::Symbol] = r.symbol;
    out[legacy::Name] = r.name;
    out[legacy::Formula] = r.formula;
    if (r.charge) out[legacy::FormulaCharge] = *r.charge;
    out[legacy::AggregateState] = json{{std::to_string(static_cast<int>(toLegacy(r.aggregate_state))), toString(r.aggregate_state)}};
    out[legacy::Class] = json{{std::to_string(static_cast<int>(toLegacy(r.class_))), toString(r.class_)}};
    out[legacy::Tst] = r.reference_conditions.T;
    out[legacy::Pst] = r.reference_conditions.P;
    if (r.molar_mass) out[legacy::MassPerMole] = json{{"values", vd{r.molar_mass->value * 1e3}}, {"units", std::vector<std::string>{"g/mol"}}};
    if (!r.defining_reaction.empty()) out[legacy::Reaction] = r.defining_reaction;
    if (hasLimits(r.applicability)) out[legacy::LimitsTP] = limitsJson(r.applicability);
    if (!r.data_sources.empty()) out[legacy::Datasources] = sourcesOf(r.data_sources);

    json methods = json::array();
    std::optional<double> alpha, kappa;
    bool gasFlag = false;
    for (const auto& app : r.methods)
    {
        const LM* method = legacyOf(app, legacy::legacySubstanceMethods());
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, OperatesOn::Substance);
        if (!method || !spec)
        {
            warn(diagnostics, r.symbol, "The method " + app.method + " has no legacy name.");
            continue;
        }
        if (const auto it = app.parameters.find("alpha0"); it != app.parameters.end()) alpha = it->second.value;
        if (const auto it = app.parameters.find("kappa0"); it != app.parameters.end()) kappa = it->second.value;
        // the state of the gas of a water equation of state on a record that is not the solvent: the flag fug_critical_param
        if (const auto it = app.parameters.find("state"); it != app.parameters.end() && it->second.value == 1.0 && r.class_ != SubstanceClass::WaterSolvent &&
            r.aggregate_state != AggregateState::Gas)
            gasFlag = true;

        if (!app.segments.empty())
        {
            for (size_t i = 0; i < app.segments.size(); ++i)
            {
                const auto& segment = app.segments[i];
                json entry = methodEntry(*method);
                addArrays(entry, *method, *spec, segment.parameters);
                Applicability limits;
                if (std::isfinite(segment.t_min)) limits.t_min = segment.t_min;
                if (std::isfinite(segment.t_max)) limits.t_max = segment.t_max;
                if (hasLimits(limits)) entry[legacy::LimitsTP] = limitsJson(limits, true);
                if (i < app.transitions.size())
                {
                    static const std::pair<const char*, double> columns[] = {{"T", 1.0}, {"dS", 1.0}, {"dH", 1.0}, {"dV", 1e5}, {"dTdP", 1e5}};
                    vd row;
                    for (const auto& column : columns)
                    {
                        const auto it = app.transitions[i].find(column.first);
                        row.push_back(it == app.transitions[i].end() ? 0.0 : it->second.value * column.second);
                    }
                    entry[legacy::MPhaseTransProps] = json{{"values", row}, {"units", std::vector<std::string>{"K", "J/(mol*K)", "J/mol", "J/bar", "K/bar"}}};
                }
                methods.push_back(entry);
            }
            if (app.transitions.size() > app.segments.size())
                warn(diagnostics, r.symbol, "The method " + app.method + " has more phase transitions than segments: the last are not written.");
            continue;
        }
        json entry = methodEntry(*method);
        addArrays(entry, *method, *spec, app.parameters);
        if (app.table)
        {
            json table = json::object();
            table["temperatures"] = app.table->temperatures;
            table["pressures"] = app.table->pressures;
            static const std::pair<const char*, const char*> names[] = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"V", "volume"}, {"Cp", "heat_capacity_cp"}};
            for (const auto& n : names)
                if (const auto it = app.table->values.find(n.first); it != app.table->values.end())
                    table[n.second] = it->second;
            if (app.table->reference_pressure) table["reference_pressure"] = *app.table->reference_pressure;
            entry[legacy::TpTable] = table;
        }
        if (hasLimits(app.applicability)) entry[legacy::LimitsTP] = limitsJson(app.applicability);
        methods.push_back(entry);
    }
    if (gasFlag)
        methods.push_back(methodEntry(*legacy::legacySubstanceMethod(8)));
    if (!r.methods.empty() || !methods.empty())
        out[legacy::TPMethods] = methods;

    if (const auto it = r.structural.find("alpha0"); it != r.structural.end()) alpha = it->second.value;
    if (const auto it = r.structural.find("kappa0"); it != r.structural.end()) kappa = it->second.value;
    // the labels of these units are swapped in the legacy files, and the reader relies on it: the expansivity has the label kbar, in 1/K
    if (alpha) out[legacy::MExpansivity] = json{{"values", vd{*alpha}}, {"units", std::vector<std::string>{"kbar"}}};
    if (kappa) out[legacy::MCompressibility] = json{{"values", vd{*kappa * 1e-8}}, {"units", std::vector<std::string>{"1e-05/K"}}};

    const auto& p = r.standard_state_properties;
    addProperty(out, legacy::SmGibbsEnergy, p, PropertyName::G, "J/mol");
    addProperty(out, legacy::SmEnthalpy, p, PropertyName::H, "J/mol");
    addProperty(out, legacy::SmEntropyAbs, p, PropertyName::S, "J/(mol*K)");
    addProperty(out, legacy::SmHeatCapacityP, p, PropertyName::Cp, "J/(mol*K)");
    addProperty(out, legacy::SmVolume, p, PropertyName::V, "J/bar", 1e5);

    reportCommon(r, diagnostics);
    if (hasValueData(p))
        warn(diagnostics, r.symbol, "The quality, origin and source of the values are not written.");
    for (const auto& s : r.structural)
        if (s.first != "alpha0" && s.first != "kappa0")
            warn(diagnostics, r.symbol, "The structural value " + s.first + " is not written.");
    return out.dump();
}

auto writeLegacyReaction(const ReactionRecord& r, const LegacyWriteOptions& options, Diagnostics* diagnostics) -> std::string
{
    if (options.use_original_text && r.source_format == DataFormat::Legacy && !r.original_text.empty())
        return r.original_text;

    json out = json::object();
    out[legacy::Symbol] = r.symbol;
    out[legacy::Name] = r.name;
    if (!r.equation.empty()) out[legacy::Equation] = r.equation;
    json reactants = json::array();
    for (const auto& item : r.stoichiometry)
        reactants.push_back(json{{"symbol", item.first}, {"coefficient", item.second}});
    out[legacy::Reactants] = reactants;
    out[legacy::Tst] = r.reference_conditions.T;
    out[legacy::Pst] = r.reference_conditions.P;
    if (hasLimits(r.applicability)) out[legacy::LimitsTP] = limitsJson(r.applicability);
    if (!r.data_sources.empty()) out[legacy::Datasources] = sourcesOf(r.data_sources);

    json methods = json::array();
    for (const auto& app : r.methods)
    {
        const LM* method = legacyOf(app, legacy::legacyReactionMethods());
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, OperatesOn::Reaction);
        if (!method || !spec)
        {
            warn(diagnostics, r.symbol, "The method " + app.method + " has no legacy name.");
            continue;
        }
        if (app.table)
            warn(diagnostics, r.symbol, "The table of the method " + app.method + " is not written: the legacy format has no table of log K.");
        json entry = methodEntry(*method);
        addArrays(entry, *method, *spec, app.parameters);
        if (hasLimits(app.applicability)) entry[legacy::LimitsTP] = limitsJson(app.applicability);
        methods.push_back(entry);
        // the coefficients of log K beside a method with parameters of its own: an entry of their method, as in the file they came from
        const bool declared = std::any_of(method->parameters.begin(), method->parameters.end(), [](const auto& p) { return std::string(p.name) == "a0"; });
        if (!declared && app.parameters.count("a0"))
        {
            const LM* polynomial = legacy::legacyReactionMethod(0);
            const MethodSpec* own = MethodCatalog::instance().find(polynomial->method, OperatesOn::Reaction);
            std::map<std::string, Quantity> coefficients;
            for (const auto& item : app.parameters)
                if (item.first.size() == 2 && item.first[0] == 'a' && item.first[1] >= '0' && item.first[1] <= '9')
                    coefficients[item.first] = item.second;
            json extra = methodEntry(*polynomial);
            addArrays(extra, *polynomial, *own, coefficients);
            methods.push_back(extra);
        }
    }
    if (!r.methods.empty())
        out[legacy::TPMethods] = methods;

    const auto& p = r.reaction_properties;
    addProperty(out, legacy::LogKr, p, ReactionPropertyName::logK, nullptr);
    addProperty(out, legacy::DrsmGibbsEnergy, p, ReactionPropertyName::dG, "J/mol");
    addProperty(out, legacy::DrsmEnthalpy, p, ReactionPropertyName::dH, "J/mol");
    addProperty(out, legacy::DrsmEntropy, p, ReactionPropertyName::dS, "J/(mol*K)");
    addProperty(out, legacy::DrsmHeatCapacityP, p, ReactionPropertyName::dCp, "J/(mol*K)");
    addProperty(out, legacy::DrsmVolume, p, ReactionPropertyName::dV, "J/bar", 1e5);

    reportCommon(r, diagnostics);
    if (hasValueData(p))
        warn(diagnostics, r.symbol, "The quality, origin and source of the values are not written.");
    return out.dump();
}

auto writeLegacyElement(const ElementRecord& r, const LegacyWriteOptions& options, Diagnostics* diagnostics) -> std::string
{
    if (options.use_original_text && r.source_format == DataFormat::Legacy && !r.original_text.empty())
        return r.original_text;

    json out = json::object();
    out[legacy::Symbol] = r.symbol;
    out[legacy::Name] = r.name;
    const int code = r.legacy_class.value_or(toLegacy(r.class_));
    out[legacy::Class] = json{{std::to_string(code), toString(r.class_)}};
    if (r.number) out[legacy::Number] = *r.number;
    if (r.isotope_mass) out[legacy::IsotopeMass] = *r.isotope_mass;
    if (r.molar_mass) out[legacy::AtomicMass] = json{{"values", vd{r.molar_mass->value * 1e3}}};
    if (r.standard_entropy) out[legacy::Entropy] = json{{"values", vd{r.standard_entropy->value}}};
    if (r.heat_capacity) out[legacy::HeatCapacity] = json{{"values", vd{r.heat_capacity->value}}};
    if (r.volume) out[legacy::Volume] = json{{"values", vd{r.volume->value * 1e5}}};
    if (!r.data_sources.empty()) out[legacy::Datasources] = sourcesOf(r.data_sources);
    if (r.valence || !r.reference_form.empty() || !r.structural.empty())
        warn(diagnostics, r.symbol, "The valence, the reference form and the structural values are not written.");
    return out.dump();
}

auto writeLegacyDocument(const V2Records& records, const LegacyWriteOptions& options, Diagnostics* diagnostics) -> std::string
{
    json root = json::object();
    root["thermodataset"] = records.dataset.symbol;
    root["datasources"] = records.dataset.legacy_datasources;
    root["date"] = records.dataset.legacy_date;
    if (records.dataset.water_triple_point_reference != ThermoDataSetInfo{}.water_triple_point_reference)
        root["conventions"] = json{{"water_triple_point_reference", records.dataset.water_triple_point_reference}};
    const auto section = [&](const char* name, const auto& list, auto write) {
        json out = json::array();
        for (const auto& record : list)
            out.push_back(json::parse(write(record, options, diagnostics)));
        root[name] = out;
    };
    section("elements", records.elements, [](const ElementRecord& r, const LegacyWriteOptions& o, Diagnostics* d) { return writeLegacyElement(r, o, d); });
    section("substances", records.substances, [](const SubstanceRecord& r, const LegacyWriteOptions& o, Diagnostics* d) { return writeLegacySubstance(r, o, d); });
    section("reactions", records.reactions, [](const ReactionRecord& r, const LegacyWriteOptions& o, Diagnostics* d) { return writeLegacyReaction(r, o, d); });
    return root.dump(1);
}

} // namespace ThermoFun::v2
