// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "V2Reader.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <regex>
#include <set>

#include <nlohmann/json.hpp>

#include <ThermoFun/IO/V2/V2Keys.h>
#include <ThermoFun/IO/V2/detail/Normalize.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/Equation.h>

namespace ThermoFun::v2 {

namespace {

using json = nlohmann::ordered_json;

struct Context
{
    Diagnostics& diag;
    const ThermoDataSetInfo* dataset = nullptr;
    std::string file;
    std::map<std::string, MethodSpec> symbolic_specs;   ///< the symbolic Method records of the file, to read the applications that name them
};

void report(Context& c, Severity severity, const char* code, const std::string& record, const std::string& path,
            const std::string& message)
{
    Diagnostic d;
    d.severity = severity;
    d.code = code;
    d.file = c.file;
    d.record = record;
    d.path = path;
    d.message = message;
    c.diag.add(d);
}

// A word of a vocabulary: an unknown word gives a warning with the words allowed, and no value
template <class E>
auto vocabulary(Context& c, const std::string& word, const std::string& record, const std::string& path) -> std::optional<E>
{
    if (word.empty())
        return std::nullopt;
    const auto value = fromString<E>(word);
    if (!value)
        report(c, Severity::Warning, "V2-VOC-001", record, path, unknownWordMessage<E>(word));
    return value;
}

// the nlohmann type of Normalize is json; this file uses ordered_json to keep the order of a map: convert
auto plain(const json& node) -> nlohmann::json
{
    return nlohmann::json::parse(node.dump());
}

auto optionalString(const json& node, const char* key) -> std::string
{
    return node.contains(key) && node[key].is_string() ? node[key].get<std::string>() : std::string();
}

auto stringList(const json& node, const char* key) -> std::vector<std::string>
{
    std::vector<std::string> out;
    if (node.contains(key) && node[key].is_array())
        for (const auto& item : node[key])
            if (item.is_string())
                out.push_back(item.get<std::string>());
    return out;
}

// The keys that the reader knows. Other keys are kept in extra_json; one that starts with "_" is kept silently.
auto extraKeys(Context& c, const json& node, const std::set<std::string>& known, const std::string& record,
               const std::string& path) -> std::string
{
    json extra = json::object();
    for (const auto& item : node.items())
    {
        if (known.count(item.key()))
            continue;
        if (item.key().empty() || item.key()[0] != '_')
            report(c, Severity::Warning, "V2-KEY-001", record, path + "/" + item.key(), "Unknown key: " + item.key());
        extra[item.key()] = item.value();
    }
    return extra.empty() ? std::string() : extra.dump();
}

auto quantity(Context& c, const json& node, const char* key, const std::string& slot, const UnitScope& scope,
              const std::string& record, const std::string& path) -> std::optional<Quantity>
{
    if (!node.contains(key))
        return std::nullopt;
    Diagnostics local;
    auto q = readQuantity(plain(node[key]), slot, scope, local, path + "/" + key);
    for (auto d : local.all())
    {
        d.record = record;
        d.file = c.file;
        c.diag.add(d);
    }
    return q;
}

auto optionalInt(const json& node, const char* key) -> std::optional<int>
{
    if (node.contains(key) && node[key].is_number())
        return static_cast<int>(node[key].get<double>());
    return std::nullopt;
}

auto readScope(Context& c, const json& node, const UnitScope* parent, const std::string& record, const std::string& path) -> UnitScope
{
    if (!node.contains(v2key::units))
    {
        UnitScope scope;
        scope.parent = parent;
        return scope;
    }
    Diagnostics local;
    UnitScope scope = readUnitScope(plain(node[v2key::units]), parent, local, path + "/" + v2key::units);
    for (auto d : local.all())
    {
        d.record = record;
        d.file = c.file;
        c.diag.add(d);
    }
    return scope;
}

auto readApplicability(Context& c, const json& node, const UnitScope& scope, const std::string& record, const std::string& path)
    -> Applicability
{
    Applicability a;
    if (!node.is_object())
        return a;
    for (const auto& item : node.items())
    {
        const std::string& k = item.key();
        if (k != "t_min" && k != "t_max" && k != "p_min" && k != "p_max" && k != "hard")
            report(c, Severity::Warning, "V2-KEY-001", record, path + "/" + k, "Unknown key in applicability: " + k);
    }
    auto get = [&](const char* key, const char* unit) -> std::optional<double> {
        auto q = quantity(c, node, key, unit, scope, record, path);
        return q ? std::optional<double>(q->value) : std::nullopt;
    };
    a.t_min = get("t_min", "K");
    a.t_max = get("t_max", "K");
    a.p_min = get("p_min", "Pa");
    a.p_max = get("p_max", "Pa");
    a.is_hard = node.contains("hard") && node["hard"].is_boolean() && node["hard"].get<bool>();
    return a;
}

auto closest(const MethodSpec& spec, const std::string& name) -> std::string
{
    // the declared name that differs least from the written name (the same first letter, the same length first)
    std::string best;
    size_t bestScore = 1000;
    for (const auto& p : spec.parameters)
    {
        size_t score = std::max(p.name.size(), name.size()) - std::min(p.name.size(), name.size());
        for (size_t i = 0; i < std::min(p.name.size(), name.size()); ++i)
            score += p.name[i] != name[i];
        if (score < bestScore)
        {
            bestScore = score;
            best = p.name;
        }
    }
    return best;
}

// the named parameters of a method (or of one segment)
auto readParameters(Context& c, const json& node, const MethodSpec& spec, const UnitScope& scope, const std::string& record,
                    const std::string& path, bool checkRequired) -> std::map<std::string, Quantity>
{
    std::map<std::string, Quantity> out;
    if (!node.is_object())
        return out;
    for (const auto& item : node.items())
    {
        const ParameterSpec* declared = nullptr;
        for (const auto& p : spec.parameters)
            if (p.name == item.key())
                declared = &p;
        if (!declared)
        {
            const std::string near = closest(spec, item.key());
            report(c, Severity::Error, "V2-MTH-002", record, path + "/" + item.key(),
                   "The method " + spec.id + " has no parameter " + item.key() + (near.empty() ? "" : ". Did you mean " + near + "?"));
            continue;
        }
        Diagnostics local;
        auto q = readQuantity(plain(item.value()), declared->si_unit, scope, local, path + "/" + item.key());
        for (auto d : local.all())
        {
            d.record = record;
            d.file = c.file;
            c.diag.add(d);
        }
        if (q)
            out[item.key()] = *q;
    }
    // A block with no parameter at all is a warning: a legacy record can have a method with no coefficients (the calculation then
    // warns). A block with some parameters but not all of the required ones is an error.
    if (checkRequired)
        for (const auto& p : spec.parameters)
            if (p.required && !out.count(p.name) && !p.default_si)
                report(c, out.empty() ? Severity::Warning : Severity::Error, "V2-MTH-004", record, path,
                       "The method " + spec.id + " needs the parameter " + p.name);
    return out;
}

auto readTable(Context& c, const json& node, const UnitScope& scope, const std::string& record, const std::string& path)
    -> std::optional<TableData>
{
    TableData t;
    auto numbers = [&](const char* key, const char* slot) {
        std::vector<double> out;
        if (!node.contains(key))
            return out;
        const json& n = node[key];
        const json* values = &n;
        std::string unit;
        if (n.is_object() && n.contains("values"))
        {
            values = &n["values"];
            unit = optionalString(n, "unit");
        }
        if (!values->is_array())
            return out;
        for (const auto& v : *values)
            if (v.is_number())
            {
                Diagnostics local;
                json one = unit.empty() ? v : json{{"value", v}, {"unit", unit}};
                auto q = readQuantity(plain(one), slot, scope, local, path + "/" + key);
                for (auto d : local.all())
                {
                    d.record = record;
                    d.file = c.file;
                    c.diag.add(d);
                }
                if (q)
                    out.push_back(q->value);
            }
        return out;
    };
    t.temperatures = numbers("temperatures", "K");
    t.pressures = numbers("pressures", "Pa");
    t.values["G"] = numbers("G", "J/mol");
    t.values["H"] = numbers("H", "J/mol");
    t.values["V"] = numbers("V", "m3/mol");
    t.values["Cp"] = numbers("Cp", "J/(mol.K)");
    t.values["logK"] = numbers("logK", "1");
    for (auto it = t.values.begin(); it != t.values.end();)
        it = it->second.empty() ? t.values.erase(it) : std::next(it);
    if (auto q = quantity(c, node, "reference_pressure", "Pa", scope, record, path))
        t.reference_pressure = q->value;
    const size_t n = t.temperatures.size() * std::max<size_t>(t.pressures.size(), 1);
    bool good = !t.temperatures.empty() && std::is_sorted(t.temperatures.begin(), t.temperatures.end()) &&
                std::is_sorted(t.pressures.begin(), t.pressures.end());
    for (const auto& v : t.values)
        good = good && v.second.size() == n;
    if (!good)
    {
        report(c, Severity::Error, "V2-MTH-018", record, path, "The table has arrays whose size is not nT x nP, or grids that are not ascending.");
        return std::nullopt;
    }
    return t;
}

auto readMethods(Context& c, const json& list, OperatesOn operatesOn, const UnitScope& parent, const std::string& record,
                 const std::string& path) -> std::vector<MethodApplication>
{
    std::vector<MethodApplication> out;
    if (!list.is_array())
        return out;
    static const std::set<std::string> known = {"method", "parameters", "parameter_set", "segments", "transitions", "applicability",
                                                "table", "units", "implementation", "interpolation", "order"};
    int index = 0;
    for (const auto& node : list)
    {
        const std::string where = path + "/" + std::to_string(index++);
        if (!node.is_object() || !node.contains("method") || !node["method"].is_string())
        {
            report(c, Severity::Error, "V2-MTH-001", record, where, "A method application needs the key method.");
            continue;
        }
        MethodApplication app;
        app.method_as_written = node["method"].get<std::string>();
        const MethodSpec* spec = MethodCatalog::instance().find(app.method_as_written, operatesOn);
        if (!spec && operatesOn == OperatesOn::Substance)
            if (auto symbolic = c.symbolic_specs.find(app.method_as_written); symbolic != c.symbolic_specs.end())
                spec = &symbolic->second;
        if (!spec)
        {
            const std::string near = MethodCatalog::instance().suggest(app.method_as_written);
            report(c, Severity::Error, "V2-MTH-001", record, where + "/method",
                   "Unknown method " + app.method_as_written + (near.empty() ? "" : ". Did you mean " + near + "?"));
            continue;
        }
        app.method = spec->id;
        app.extra_json = extraKeys(c, node, known, record, where);
        const UnitScope scope = readScope(c, node, &parent, record, where);
        app.parameter_set = optionalString(node, "parameter_set");
        app.implementation = optionalString(node, "implementation");
        if (!app.implementation.empty() && std::find(spec->implementations.begin(), spec->implementations.end(), app.implementation) == spec->implementations.end())
        {
            std::string allowed;
            for (const auto& i : spec->implementations)
                allowed += " " + i;
            report(c, Severity::Error, "V2-MTH-009", record, where + "/implementation",
                   "The method " + spec->id + " has no implementation " + app.implementation + ". Allowed:" + (allowed.empty() ? " none" : allowed) + ".");
            continue;
        }
        app.interpolation = vocabulary<Interpolation>(c, optionalString(node, "interpolation"), record, where + "/interpolation");
        if (auto order = optionalInt(node, "order"))
            app.interpolation_order = *order;

        const bool hasParameters = node.contains("parameters"), hasSegments = node.contains("segments"),
                   hasTable = node.contains("table");
        if (hasParameters + hasSegments + hasTable > 1)
        {
            report(c, Severity::Error, "V2-MTH-003", record, where, "Only one of parameters, segments and table may be given.");
            continue;
        }
        if (hasParameters)
            app.parameters = readParameters(c, node["parameters"], *spec, scope, record, where + "/parameters", app.parameter_set.empty());
        if (hasSegments && node["segments"].is_array())
        {
            int s = 0;
            for (const auto& segment : node["segments"])
            {
                const std::string at = where + "/segments/" + std::to_string(s++);
                Segment seg;
                seg.t_min = -std::numeric_limits<double>::infinity();   // no bound written: no bound
                seg.t_max = std::numeric_limits<double>::infinity();
                const UnitScope segScope = readScope(c, segment, &scope, record, at);
                if (auto q = quantity(c, segment, "t_min", "K", segScope, record, at))
                    seg.t_min = q->value;
                if (auto q = quantity(c, segment, "t_max", "K", segScope, record, at))
                    seg.t_max = q->value;
                if (segment.contains("parameters"))
                    seg.parameters = readParameters(c, segment["parameters"], *spec, segScope, record, at + "/parameters", true);
                app.segments.push_back(seg);
            }
        }
        if (node.contains("transitions") && node["transitions"].is_array())
        {
            static const std::pair<const char*, const char*> columns[] = {
                {"T", "K"}, {"dS", "J/(mol.K)"}, {"dH", "J/mol"}, {"dV", "m3/mol"}, {"dTdP", "K/Pa"}};
            for (const auto& row : node["transitions"])
            {
                std::map<std::string, Quantity> values;
                for (const auto& column : columns)
                    if (auto q = quantity(c, row, column.first, column.second, scope, record, where + "/transitions"))
                        values[column.first] = *q;
                app.transitions.push_back(values);
            }
        }
        if (hasTable)
            app.table = readTable(c, node["table"], scope, record, where + "/table");
        if (node.contains("applicability"))
            app.applicability = readApplicability(c, node["applicability"], scope, record, where + "/applicability");
        out.push_back(app);
    }
    return out;
}

auto readConvention(Context& c, const json& node, const std::string& record, const std::string& path) -> std::optional<Convention>
{
    if (!node.is_object())
        return std::nullopt;
    Convention conv;
    conv.energy_reference = vocabulary<EnergyReference>(c, optionalString(node, "energy_reference"), record, path + "/energy_reference");
    conv.standard_pressure = vocabulary<StandardPressure>(c, optionalString(node, "standard_pressure"), record, path + "/standard_pressure");
    conv.concentration_scale = vocabulary<ConcentrationScale>(c, optionalString(node, "concentration_scale"), record, path + "/concentration_scale");
    conv.water_reference = vocabulary<WaterReference>(c, optionalString(node, "water_reference"), record, path + "/water_reference");
    return conv;
}

// The source that a record prefers; `record` is a setting of the engine, not of a file
auto readPropertySource(Context& c, const json& node, const std::string& record, const std::string& path) -> std::optional<PropertySource>
{
    auto source = vocabulary<PropertySource>(c, optionalString(node, "preferred_property_source"), record, path + "/preferred_property_source");
    if (source == PropertySource::Record)
    {
        report(c, Severity::Warning, "V2-VOC-001", record, path + "/preferred_property_source", "The word record is a setting of the engine: it is not used in a file.");
        return std::nullopt;
    }
    return source;
}

// The covariance of values of the record. The checks of the values are in validate (V2-COV-001, 002, 004).
auto readCovariances(Context& c, const json& node, const std::string& record, const std::string& path) -> std::vector<CovarianceBlock>
{
    std::vector<CovarianceBlock> out;
    if (!node.contains("covariances") || !node["covariances"].is_array())
        return out;
    int index = 0;
    for (const auto& item : node["covariances"])
    {
        const std::string where = path + "/covariances/" + std::to_string(index++);
        CovarianceBlock block;
        block.paths = stringList(item, "paths");
        if (item.contains("matrix") && item["matrix"].is_array())
            for (const auto& row : item["matrix"])
            {
                if (row.is_number())
                    block.matrix.push_back(row.get<double>());
                else if (row.is_array())
                    for (const auto& x : row)
                        if (x.is_number())
                            block.matrix.push_back(x.get<double>());
            }
        if (block.matrix.size() != block.paths.size() * block.paths.size())
        {
            report(c, Severity::Error, "V2-COV-003", record, where,
                   "The matrix has " + std::to_string(block.matrix.size()) + " values for " + std::to_string(block.paths.size()) + " paths.");
            continue;
        }
        out.push_back(block);
    }
    return out;
}

auto readReferenceConditions(Context& c, const json& node, const UnitScope& scope, const std::string& record,
                             const std::string& path, ReferenceConditions fallback) -> ReferenceConditions
{
    if (node.contains("reference_conditions") && node["reference_conditions"].is_object())
    {
        const json& rc = node["reference_conditions"];
        if (auto q = quantity(c, rc, "T", "K", scope, record, path + "/reference_conditions"))
            fallback.T = q->value;
        if (auto q = quantity(c, rc, "P", "Pa", scope, record, path + "/reference_conditions"))
            fallback.P = q->value;
    }
    return fallback;
}

auto symbolOf(Context& c, const json& node, const std::string& key, const std::string& path) -> std::string
{
    std::string symbol = key;
    if (node.contains("symbol") && node["symbol"].is_string())
    {
        const std::string inside = node["symbol"].get<std::string>();
        if (!key.empty() && inside != key)
            report(c, Severity::Error, "V2-ID-002", key, path + "/symbol", "The symbol " + inside + " differs from the key " + key);
        if (key.empty())
            symbol = inside;
    }
    if (symbol.empty())
        report(c, Severity::Error, "V2-ID-001", "", path, "The record has no symbol.");
    return symbol;
}

auto readElement(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> ElementRecord
{
    ElementRecord r;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "An element must be a mapping.");
        return r;
    }
    static const std::set<std::string> known = {"symbol", "name", "class", "number", "isotope_mass", "valence", "molar_mass",
                                                "standard_entropy", "heat_capacity", "volume", "reference_form", "data_sources",
                                                "sourcetdb", "structural", "units"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return r;
    const UnitScope scope = readScope(c, node, &parent, symbol, path);
    UnitScope mass;
    mass.defaults["mass"] = "g";   // a bare molar mass is in g/mol
    mass.parent = &scope;
    r.symbol = symbol;
    r.name = optionalString(node, "name");
    r.class_ = optionalString(node, "class").empty() ? ElementClass::Element : vocabulary<ElementClass>(c, optionalString(node, "class"), symbol, path + "/class").value_or(ElementClass::Other);
    r.isotope_mass = optionalInt(node, "isotope_mass");
    r.number = optionalInt(node, "number");
    r.valence = optionalInt(node, "valence");
    r.molar_mass = quantity(c, node, "molar_mass", "kg/mol", mass, symbol, path);
    r.standard_entropy = quantity(c, node, "standard_entropy", "J/(mol.K)", scope, symbol, path);
    r.heat_capacity = quantity(c, node, "heat_capacity", "J/(mol.K)", scope, symbol, path);
    r.volume = quantity(c, node, "volume", "m3/mol", scope, symbol, path);
    r.reference_form = optionalString(node, "reference_form");
    r.data_sources = stringList(node, "data_sources");
    r.sourcetdb = optionalString(node, "sourcetdb");
    if (node.contains("structural") && node["structural"].is_object())
        for (const auto& item : node["structural"].items())
        {
            Diagnostics local;
            if (auto q = readQuantity(plain(item.value()), "1", scope, local, path + "/structural/" + item.key()))
                r.structural[item.key()] = *q;
        }
    r.extra_json = extraKeys(c, node, known, symbol, path);
    r.source_format = DataFormat::V2;
    r.original_text = node.dump();
    return r;
}

auto readSubstance(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> SubstanceRecord
{
    SubstanceRecord r;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A substance must be a mapping.");
        return r;
    }
    static const std::set<std::string> known = {
        "symbol", "name", "formula", "charge", "class", "aggregate_state", "sourcetdb", "molar_mass", "standard_state_convention",
        "convention", "reference_conditions", "standard_state_properties", "methods", "defining_reaction", "preferred_property_source",
        "applicability", "data_source", "data_sources", "structural", "deprecated", "deprecated_in_favour_of", "units", "covariances"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return r;
    const UnitScope scope = readScope(c, node, &parent, symbol, path);
    UnitScope mass;
    mass.defaults["mass"] = "g";
    mass.parent = &scope;
    r.symbol = symbol;
    r.name = optionalString(node, "name");
    r.formula = optionalString(node, "formula");
    r.charge = optionalInt(node, "charge");
    r.class_ = vocabulary<SubstanceClass>(c, optionalString(node, "class"), symbol, path + "/class").value_or(SubstanceClass::Other);
    r.aggregate_state = vocabulary<AggregateState>(c, optionalString(node, "aggregate_state"), symbol, path + "/aggregate_state").value_or(AggregateState::Other);
    r.sourcetdb = optionalString(node, "sourcetdb");
    r.molar_mass = quantity(c, node, "molar_mass", "kg/mol", mass, symbol, path);
    r.standard_state_convention = optionalString(node, "standard_state_convention");
    if (node.contains("convention"))
        r.convention = readConvention(c, node["convention"], symbol, path + "/convention");
    r.reference_conditions = readReferenceConditions(c, node, scope, symbol, path,
                                                     c.dataset ? c.dataset->reference_conditions : ReferenceConditions{});
    if (node.contains("standard_state_properties") && node["standard_state_properties"].is_object())
        for (const auto& item : node["standard_state_properties"].items())
        {
            auto name = fromString<PropertyName>(item.key());
            if (!name)
            {
                report(c, Severity::Warning, "V2-KEY-001", symbol, path + "/standard_state_properties/" + item.key(),
                       "Unknown property: " + item.key());
                continue;
            }
            Diagnostics local;
            auto q = readQuantity(plain(item.value()), siUnit(*name), scope, local, path + "/standard_state_properties/" + item.key());
            for (auto d : local.all())
            {
                d.record = symbol;
                d.file = c.file;
                c.diag.add(d);
            }
            if (q)
                r.standard_state_properties[*name] = *q;
        }
    if (node.contains("methods"))
        r.methods = readMethods(c, node["methods"], OperatesOn::Substance, scope, symbol, path + "/methods");
    r.defining_reaction = optionalString(node, "defining_reaction");
    r.preferred_property_source = readPropertySource(c, node, symbol, path);
    if (node.contains("applicability"))
        r.applicability = readApplicability(c, node["applicability"], scope, symbol, path + "/applicability");
    r.data_source = optionalString(node, "data_source");
    r.data_sources = stringList(node, "data_sources");
    if (node.contains("structural") && node["structural"].is_object())
        for (const auto& item : node["structural"].items())
        {
            Diagnostics local;
            if (auto q = readQuantity(plain(item.value()), "1", scope, local, path + "/structural/" + item.key()))
                r.structural[item.key()] = *q;
        }
    r.deprecated = node.contains("deprecated") && node["deprecated"].is_boolean() && node["deprecated"].get<bool>();
    r.deprecated_in_favour_of = optionalString(node, "deprecated_in_favour_of");
    r.covariance = readCovariances(c, node, symbol, path);
    r.extra_json = extraKeys(c, node, known, symbol, path);
    r.source_format = DataFormat::V2;
    r.original_text = node.dump();
    return r;
}

auto readReaction(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> ReactionRecord
{
    ReactionRecord r;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A reaction must be a mapping.");
        return r;
    }
    static const std::set<std::string> known = {
        "symbol", "name", "equation", "stoichiometry", "reference_conditions", "reaction_properties", "methods", "level", "data_source",
        "data_sources", "applicability", "convention", "preferred_property_source", "sourcetdb", "deprecated", "units", "covariances"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return r;
    const UnitScope scope = readScope(c, node, &parent, symbol, path);
    r.symbol = symbol;
    r.name = optionalString(node, "name");
    r.sourcetdb = optionalString(node, "sourcetdb");
    r.equation = optionalString(node, "equation");
    if (node.contains("stoichiometry") && node["stoichiometry"].is_object())
        for (const auto& item : node["stoichiometry"].items())
            if (item.value().is_number())
                r.stoichiometry.emplace_back(item.key(), item.value().get<double>());
    if (!r.equation.empty())
    {
        // if the stoichiometry is also written, both must agree
        const auto fromEquation = stoichiometryOfEquation(r.equation);
        if (r.stoichiometry.empty())
            r.stoichiometry = fromEquation;
        else
        {
            std::map<std::string, double> written(r.stoichiometry.begin(), r.stoichiometry.end());
            bool agree = written.size() == fromEquation.size();
            for (const auto& item : fromEquation)
                agree = agree && written.count(item.first) && std::fabs(written[item.first] - item.second) < 1e-12;
            if (!agree)
            {
                report(c, Severity::Error, "V2-RXN-001", symbol, path, "The equation and the stoichiometry disagree.");
                return ReactionRecord{};
            }
        }
    }
    r.reference_conditions = readReferenceConditions(c, node, scope, symbol, path,
                                                     c.dataset ? c.dataset->reference_conditions : ReferenceConditions{});
    if (node.contains("reaction_properties") && node["reaction_properties"].is_object())
        for (const auto& item : node["reaction_properties"].items())
        {
            auto name = fromString<ReactionPropertyName>(item.key());
            if (!name)
            {
                report(c, Severity::Warning, "V2-KEY-001", symbol, path + "/reaction_properties/" + item.key(),
                       "Unknown property: " + item.key());
                continue;
            }
            Diagnostics local;
            auto q = readQuantity(plain(item.value()), siUnit(*name), scope, local, path + "/reaction_properties/" + item.key());
            for (auto d : local.all())
            {
                d.record = symbol;
                d.file = c.file;
                c.diag.add(d);
            }
            if (q)
                r.reaction_properties[*name] = *q;
        }
    if (node.contains("methods"))
        r.methods = readMethods(c, node["methods"], OperatesOn::Reaction, scope, symbol, path + "/methods");
    if (auto level = optionalInt(node, "level"))
        r.level = *level;
    if (node.contains("applicability"))
        r.applicability = readApplicability(c, node["applicability"], scope, symbol, path + "/applicability");
    if (node.contains("convention"))
        r.convention = readConvention(c, node["convention"], symbol, path + "/convention");
    r.preferred_property_source = readPropertySource(c, node, symbol, path);
    r.data_source = optionalString(node, "data_source");
    r.data_sources = stringList(node, "data_sources");
    r.deprecated = node.contains("deprecated") && node["deprecated"].is_boolean() && node["deprecated"].get<bool>();
    r.covariance = readCovariances(c, node, symbol, path);
    r.extra_json = extraKeys(c, node, known, symbol, path);
    r.source_format = DataFormat::V2;
    r.original_text = node.dump();
    return r;
}

auto effectsText(const json& node) -> std::string
{
    return node.contains("effects") ? node["effects"].dump() : std::string();
}

auto readMethodRecord(Context& c, const json& node, const std::string& key, const UnitScope&, const std::string& path) -> MethodDefinition
{
    MethodDefinition m;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A method must be a mapping.");
        return m;
    }
    static const std::set<std::string> known = {"symbol", "kind", "tier", "operates_on", "applies_to_classes", "contributes_to",
                                                "parameter_definitions", "implementation_id", "source", "expression", "generator",
                                                "source_hash", "tests", "effects", "citation"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return m;
    m.symbol = symbol;
    if (node.contains("kind") && node["kind"].is_string())
        m.kind = vocabulary<MethodKind>(c, node["kind"].get<std::string>(), symbol, path + "/kind").value_or(m.kind);
    if (auto tier = optionalInt(node, "tier"))
        m.tier = *tier;
    if (node.contains("operates_on") && node["operates_on"].is_string())
        m.operates_on = vocabulary<OperatesOn>(c, node["operates_on"].get<std::string>(), symbol, path + "/operates_on").value_or(m.operates_on);
    for (const auto& word : stringList(node, "applies_to_classes"))
        if (const auto cls = vocabulary<SubstanceClass>(c, word, symbol, path + "/applies_to_classes"))
            m.applies_to_classes.push_back(*cls);
    m.contributes_to = stringList(node, "contributes_to");
    if (node.contains("parameter_definitions") && node["parameter_definitions"].is_object())
        for (const auto& item : node["parameter_definitions"].items())
        {
            ParameterDefinition p;
            p.name = item.key();
            if (item.value().is_object())
            {
                p.unit = optionalString(item.value(), "unit");
                p.description = optionalString(item.value(), "description");
                if (item.value().contains("required") && item.value()["required"].is_boolean())
                    p.required = item.value()["required"].get<bool>();
                if (item.value().contains("default") && item.value()["default"].is_number())
                    p.default_value = item.value()["default"].get<double>();
            }
            m.parameter_definitions.push_back(p);
        }
    m.implementation_id = optionalString(node, "implementation_id");
    if (node.contains("source") && node["source"].is_object())
    {
        m.source_form = optionalString(node["source"], "form");
        m.source_expression = optionalString(node["source"], "expression");
    }
    if (node.contains("expression") && node["expression"].is_object())
        for (const auto& item : node["expression"].items())
            if (item.value().is_string())
                m.expression[item.key()] = item.value().get<std::string>();
    // a list: one expression for each piece in T of the substance
    if (node.contains("expression") && node["expression"].is_array())
        for (const auto& piece : node["expression"])
        {
            std::map<std::string, std::string> one;
            if (piece.is_object())
                for (const auto& item : piece.items())
                    if (item.value().is_string())
                        one[item.key()] = item.value().get<std::string>();
            m.expression_segments.push_back(one);
        }
    m.generator = optionalString(node, "generator");
    m.source_hash = optionalString(node, "source_hash");
    if (node.contains("tests") && node["tests"].is_array())
        for (const auto& t : node["tests"])
        {
            ReferenceTestCase test;
            if (t.contains("T") && t["T"].is_number()) test.T = t["T"].get<double>();
            if (t.contains("P") && t["P"].is_number()) test.P = t["P"].get<double>();
            if (t.contains("relative_tolerance") && t["relative_tolerance"].is_number())
                test.relative_tolerance = t["relative_tolerance"].get<double>();
            for (const char* block : {"parameters", "expected"})
                if (t.contains(block) && t[block].is_object())
                    for (const auto& item : t[block].items())
                        if (item.value().is_number())
                            (std::string(block) == "parameters" ? test.parameters : test.expected)[item.key()] = item.value().get<double>();
            m.tests.push_back(test);
        }
    m.effects_json = effectsText(node);
    m.citation = optionalString(node, "citation");
    m.extra_json = extraKeys(c, node, known, symbol, path);
    return m;
}

auto readParameterSet(Context& c, const json& node, const std::string& key, const UnitScope& scope, const std::string& path,
                      const std::vector<MethodDefinition>& methods) -> MethodParameterSet
{
    MethodParameterSet set;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A parameter set must be a mapping.");
        return set;
    }
    static const std::set<std::string> known = {"symbol", "method", "parameters", "auxiliary", "reference", "units"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return set;
    set.symbol = symbol;
    set.method = optionalString(node, "method");
    const UnitScope own = readScope(c, node, &scope, symbol, path);
    // the unit of a value: the parameter of the built-in method, or of the method record of the file, else none
    std::map<std::string, std::string> units;
    if (const MethodSpec* spec = MethodCatalog::instance().find(set.method))
        for (const auto& p : spec->parameters)
            units[p.name] = p.si_unit;
    for (const auto& m : methods)
        if (m.symbol == set.method)
            for (const auto& p : m.parameter_definitions)
                units[p.name] = p.unit.empty() ? "1" : p.unit;
    auto fill = [&](const char* block, std::map<std::string, Quantity>& out, bool useUnits) {
        if (!node.contains(block) || !node[block].is_object())
            return;
        for (const auto& item : node[block].items())
        {
            const std::string slot = useUnits && units.count(item.key()) ? units[item.key()] : "1";
            Diagnostics local;
            auto q = readQuantity(plain(item.value()), slot, own, local, path + "/" + block + "/" + item.key());
            for (auto d : local.all())
            {
                d.record = symbol;
                d.file = c.file;
                c.diag.add(d);
            }
            if (q)
                out[item.key()] = *q;
        }
    };
    fill("parameters", set.parameters, true);
    fill("auxiliary", set.auxiliary, false);
    set.reference = optionalString(node, "reference");
    set.extra_json = extraKeys(c, node, known, symbol, path);
    return set;
}

auto readDataSource(Context& c, const json& node, const std::string& key, const std::string& path) -> DataSource
{
    DataSource d;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A data source must be a mapping.");
        return d;
    }
    static const std::set<std::string> known = {"symbol", "title", "authors", "year", "journal", "doi", "bibtex"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return d;
    d.symbol = symbol;
    d.title = optionalString(node, "title");
    d.authors = optionalString(node, "authors");
    d.journal = optionalString(node, "journal");
    d.doi = optionalString(node, "doi");
    d.bibtex = optionalString(node, "bibtex");
    d.year = optionalInt(node, "year");
    d.extra_json = extraKeys(c, node, known, symbol, path);
    static const std::regex nea("^[0-9]{4}[A-Z]{3}(/[A-Z]{3})?$");
    if (!std::regex_match(symbol, nea))
        report(c, Severity::Warning, "V2-SRC-001", symbol, path, "The symbol " + symbol + " is not of the form 1999ABC/DEF.");
    return d;
}

auto readDataset(Context& c, const json& node, const UnitScope& scope) -> ThermoDataSetInfo
{
    ThermoDataSetInfo info;
    info.water_triple_point_reference = "HKF_HGK_1974";
    if (!node.is_object())
        return info;
    static const std::set<std::string> known = {"symbol", "name", "version", "status", "standard_state_convention", "reference_conditions",
                                                "sourcetdb", "solvent", "data_source", "defaults", "conventions", "convention"};
    info.symbol = optionalString(node, "symbol");
    info.name = optionalString(node, "name");
    info.version = optionalString(node, "version");
    if (node.contains("status") && node["status"].is_string())
        info.status = vocabulary<DatasetStatus>(c, node["status"].get<std::string>(), info.symbol, "/thermodataset/status").value_or(info.status);
    if (info.status == DatasetStatus::Published && info.version.empty())
        report(c, Severity::Error, "V2-DS-001", info.symbol, "/thermodataset", "A published dataset needs a version.");
    const std::string convention = optionalString(node, "standard_state_convention");
    if (!convention.empty())
        info.standard_state_convention = convention;
    info.reference_conditions = readReferenceConditions(c, node, scope, info.symbol, "/thermodataset", ReferenceConditions{});
    info.sourcetdb = stringList(node, "sourcetdb");
    info.solvent = optionalString(node, "solvent");
    info.default_data_source = optionalString(node, "data_source");
    if (node.contains("defaults") && node["defaults"].is_object())
    {
        info.defaults.quality = vocabulary<Quality>(c, optionalString(node["defaults"], "quality"), info.symbol, "/thermodataset/defaults/quality");
        info.defaults.origin = vocabulary<Origin>(c, optionalString(node["defaults"], "origin"), info.symbol, "/thermodataset/defaults/origin");
    }
    if (node.contains("conventions") && node["conventions"].is_object())
    {
        const std::string w = optionalString(node["conventions"], "water_triple_point_reference");
        if (!w.empty())
            info.water_triple_point_reference = w;
    }
    if (node.contains("convention"))
        if (auto conv = readConvention(c, node["convention"], info.symbol, "/thermodataset/convention"))
            info.convention = *conv;
    info.extra_json = extraKeys(c, node, known, info.symbol, "/thermodataset");
    return info;
}

// a map symbol -> record, or a list of records with a symbol
template <class Record, class Reader>
void readRecords(Context& c, const json& node, const UnitScope& scope, const std::string& path, std::vector<Record>& out, Reader reader)
{
    std::set<std::string> seen;
    auto add = [&](Record&& r) {
        if (r.symbol.empty())
            return;
        if (!seen.insert(r.symbol).second)
            report(c, Severity::Warning, "V2-ID-003", r.symbol, path, "The symbol " + r.symbol + " is used twice; the later record replaces the earlier.");
        out.push_back(std::move(r));
    };
    if (node.is_object())
        for (const auto& item : node.items())
            add(reader(c, item.value(), item.key(), scope, path + "/" + item.key()));
    else if (node.is_array())
    {
        int i = 0;
        for (const auto& item : node)
            add(reader(c, item, std::string(), scope, path + "/" + std::to_string(i++)));
    }
}

void versionCheck(Context& c, const json& root)
{
    if (!root.contains("thermohub") || !root["thermohub"].is_string())
        return;
    const std::string version = root["thermohub"].get<std::string>();
    const int major = std::atoi(version.c_str());
    if (major != 2)
        report(c, Severity::Error, "V2-VER-001", "", "/thermohub", "This file uses the format " + version + "; ThermoFun reads 2.x.");
}

}

auto readV2Document(const std::string& text, Diagnostics& diagnostics, const std::string& file) -> V2Document
{
    V2Document doc;
    Context c{diagnostics, nullptr, file};
    json root = json::parse(text, nullptr, false);
    if (root.is_discarded() || !root.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", "", "/", "The text is not a JSON object.");
        return doc;
    }
    versionCheck(c, root);
    for (const auto& d : diagnostics.all())
        if (d.code == "V2-VER-001")
            return doc;
    static const std::set<std::string> topKnown = {"thermohub", "generator", "thermodataset", "units", "include", "elements",
                                                    "substances", "reactions", "methods", "parameter_sets", "data_sources"};
    for (const auto& item : root.items())
        if (!topKnown.count(item.key()) && (item.key().empty() || item.key()[0] != '_'))
            report(c, Severity::Warning, "V2-KEY-001", "", "/" + item.key(), "Unknown top-level key: " + item.key());

    UnitScope fileScope = readScope(c, root, nullptr, "", "");
    if (root.contains("thermodataset"))
    {
        doc.info = readDataset(c, root["thermodataset"], fileScope);
        doc.has_dataset = true;
    }
    doc.info.schema_version = root.contains("thermohub") && root["thermohub"].is_string() ? root["thermohub"].get<std::string>() : "";
    c.dataset = &doc.info;
    if (root.contains("methods") && root["methods"].is_object())
        for (const auto& item : root["methods"].items())
        {
            const json& m = item.value();
            if (!m.is_object() || !m.contains("kind") || !m["kind"].is_string() || m["kind"].get<std::string>() != "symbolic")
                continue;
            if (m.contains("operates_on") && m["operates_on"].is_string() && m["operates_on"].get<std::string>() != "substance")
                continue;
            MethodSpec spec;
            spec.id = item.key();
            if (m.contains("parameter_definitions") && m["parameter_definitions"].is_object())
                for (const auto& p : m["parameter_definitions"].items())
                {
                    ParameterSpec ps;
                    ps.name = p.key();
                    ps.si_unit = p.value().is_object() ? optionalString(p.value(), "unit") : std::string();
                    if (ps.si_unit.empty())
                        ps.si_unit = "1";
                    ps.required = false;
                    spec.parameters.push_back(ps);
                }
            c.symbolic_specs.emplace(item.key(), spec);
        }
    if (root.contains("elements"))
        readRecords(c, root["elements"], fileScope, "/elements", doc.elements, readElement);
    if (root.contains("substances"))
        readRecords(c, root["substances"], fileScope, "/substances", doc.substances, readSubstance);
    if (root.contains("reactions"))
        readRecords(c, root["reactions"], fileScope, "/reactions", doc.reactions, readReaction);
    if (root.contains("methods"))
        readRecords(c, root["methods"], fileScope, "/methods", doc.methods, readMethodRecord);
    if (root.contains("parameter_sets"))
        readRecords(c, root["parameter_sets"], fileScope, "/parameter_sets", doc.parameter_sets,
                    [&doc](Context& cc, const json& n, const std::string& k, const UnitScope& sc, const std::string& p) {
                        return readParameterSet(cc, n, k, sc, p, doc.methods);
                    });
    if (root.contains("data_sources"))
        readRecords(c, root["data_sources"], fileScope, "/data_sources", doc.data_sources,
                    [](Context& cc, const json& n, const std::string& k, const UnitScope&, const std::string& p) {
                        return readDataSource(cc, n, k, p);
                    });
    for (const auto& m : doc.methods)
        checkMethodDefinition(m, diagnostics);
    return doc;
}

namespace {

template <class Record, class Reader>
auto readOne(const std::string& text, Diagnostics& diagnostics, Reader reader) -> Record
{
    Context c{diagnostics, nullptr, ""};
    json node = json::parse(text, nullptr, false);
    if (node.is_discarded())
    {
        report(c, Severity::Error, "V2-ID-001", "", "/", "The text is not JSON.");
        return Record{};
    }
    UnitScope scope;
    return reader(c, node, std::string(), scope, "");
}

}

auto readV2Element(const std::string& text, Diagnostics& diagnostics) -> ElementRecord
{
    return readOne<ElementRecord>(text, diagnostics, readElement);
}

auto readV2Substance(const std::string& text, Diagnostics& diagnostics) -> SubstanceRecord
{
    return readOne<SubstanceRecord>(text, diagnostics, readSubstance);
}

auto readV2Reaction(const std::string& text, Diagnostics& diagnostics) -> ReactionRecord
{
    return readOne<ReactionRecord>(text, diagnostics, readReaction);
}

auto checkMethodDefinition(const MethodDefinition& method, Diagnostics& diagnostics) -> void
{
    Context c{diagnostics, nullptr, ""};
    const MethodSpec* spec = MethodCatalog::instance().find(method.symbol);
    if (!spec)
        return;
    const std::string path = "/methods/" + method.symbol;
    // V2-MTH-010: the names and the dimensions of the parameters
    std::set<std::string> declared, defined;
    for (const auto& p : spec->parameters)
        declared.insert(p.name);
    for (const auto& p : method.parameter_definitions)
        defined.insert(p.name);
    if (!method.parameter_definitions.empty() && declared != defined)
        report(c, Severity::Error, "V2-MTH-010", method.symbol, path + "/parameter_definitions",
               "The parameters of the method record are not those of the built-in method " + spec->id);
    for (const auto& p : method.parameter_definitions)
        for (const auto& q : spec->parameters)
            if (p.name == q.name && !p.unit.empty())
                try
                {
                    if (units::dimension(p.unit) != units::dimension(q.si_unit))
                        report(c, Severity::Error, "V2-MTH-010", method.symbol, path + "/parameter_definitions/" + p.name,
                               "The unit " + p.unit + " has not the dimension of " + q.si_unit);
                }
                catch (const std::exception&)
                {
                    report(c, Severity::Error, "V2-UNIT-001", method.symbol, path + "/parameter_definitions/" + p.name,
                           "The unit " + p.unit + " cannot be read.");
                }
    // V2-MTH-011: what the method operates on and contributes to
    if (method.operates_on != spec->operates_on)
        report(c, Severity::Error, "V2-MTH-011", method.symbol, path + "/operates_on",
               "The method " + spec->id + " operates on " + toString(spec->operates_on) + ", not on " + toString(method.operates_on));
    if (!method.contributes_to.empty())
    {
        std::set<std::string> contributes;
        auto add = [&](Effect effect, const char* name) {
            if (effect == Effect::Set || effect == Effect::Add)
                contributes.insert(name);
        };
        const auto& e = spec->effects;
        const auto& r = spec->reaction_effects;
        add(e.G, "G"), add(e.H, "H"), add(e.S, "S"), add(e.Cp, "Cp"), add(e.V, "V");
        add(r.logK, "logK"), add(r.dG, "dG"), add(r.dH, "dH"), add(r.dS, "dS"), add(r.dCp, "dCp"), add(r.dV, "dV");
        if (std::set<std::string>(method.contributes_to.begin(), method.contributes_to.end()) != contributes)
            report(c, Severity::Error, "V2-MTH-011", method.symbol, path + "/contributes_to",
                   "The method " + spec->id + " does not contribute to the same properties.");
    }
}

auto resolveV2Document(const V2Document& document, Diagnostics& diagnostics) -> void
{
    Context c{diagnostics, nullptr, ""};
    std::set<std::string> substances, reactions;
    for (const auto& s : document.substances)
        substances.insert(s.symbol);
    for (const auto& r : document.reactions)
        reactions.insert(r.symbol);
    for (const auto& s : document.substances)
        if (!s.defining_reaction.empty() && !reactions.count(s.defining_reaction))
            // a warning here: the reaction may be loaded from another file later (appendData); validate() reports it as an error
            report(c, Severity::Warning, "V2-REF-001", s.symbol, "/substances/" + s.symbol + "/defining_reaction",
                   "The reaction " + s.defining_reaction + " is not in the document.");
    // parameter sets and data sources (warnings in the load path, as the defining reaction: the record may come from another file)
    std::map<std::string, std::string> sets;
    for (const auto& p : document.parameter_sets)
        sets[p.symbol] = p.method;
    std::set<std::string> sources;
    for (const auto& d : document.data_sources)
        sources.insert(d.symbol);
    auto checkApplications = [&](const std::string& record, const std::vector<MethodApplication>& methods) {
        for (const auto& app : methods)
            if (!app.parameter_set.empty())
            {
                auto it = sets.find(app.parameter_set);
                if (it == sets.end() || it->second != app.method)
                    report(c, Severity::Warning, "V2-REF-004", record, "/" + record + "/methods",
                           "The parameter set " + app.parameter_set + " is not in the document, or belongs to another method.");
            }
    };
    for (const auto& s : document.substances)
    {
        checkApplications(s.symbol, s.methods);
        if (!s.data_source.empty() && !sources.empty() && !sources.count(s.data_source))
            report(c, Severity::Warning, "V2-REF-005", s.symbol, "/substances/" + s.symbol + "/data_source",
                   "The data source " + s.data_source + " is not in the document.");
    }
    for (const auto& r : document.reactions)
        checkApplications(r.symbol, r.methods);
    for (const auto& r : document.reactions)
        for (const auto& item : r.stoichiometry)
            if (!substances.count(item.first))
                report(c, Severity::Warning, "V2-REF-002", r.symbol, "/reactions/" + r.symbol + "/stoichiometry",
                       "The substance " + item.first + " is not in the document.");
}

}
