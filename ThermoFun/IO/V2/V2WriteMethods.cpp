// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Writes the methods of a record, method records and parameter sets.

#include <ThermoFun/IO/V2/detail/V2WriterParts.h>

namespace ThermoFun::v2::v2writer {

// The `units` mapping of a method: the one that lets most values of the application be bare numbers in their unit
auto chooseScope(const std::map<std::string, std::string>& wanted, const MethodSpec* spec) -> std::map<std::string, std::string>
{
    std::map<std::string, std::string> best;
    if (!spec)
        return best;
    auto matches = [&](const std::map<std::string, std::string>& defaults) {
        UnitScope scope;
        scope.defaults = defaults;
        int count = 0;
        for (const auto& p : spec->parameters)
        {
            const auto it = wanted.find(p.name);
            if (it == wanted.end())
                continue;
            const std::string target = it->second.empty() ? p.si_unit : it->second;
            count += sameUnit(scopedUnit(p.si_unit, scope), molar(target, p.si_unit));
        }
        return count;
    };
    int bestCount = matches({});
    for (const char* energy : {"J", "cal", "kJ"})
        for (const char* pressure : {"Pa", "bar", "kbar", "MPa"})
        {
            std::map<std::string, std::string> defaults;
            if (std::string(energy) != "J") defaults["energy"] = energy;
            if (std::string(pressure) != "Pa") defaults["pressure"] = pressure;
            const int count = matches(defaults);
            if (count > bestCount)
            {
                bestCount = count;
                best = defaults;
            }
        }
    return best;
}

auto parametersJson(const std::map<std::string, Quantity>& parameters, const MethodSpec* spec, const std::map<std::string, std::string>& scopeDefaults,
                    const std::map<std::string, std::string>& wanted, const V2WriterOptions& options) -> json
{
    UnitScope scope;
    scope.defaults = scopeDefaults;
    json out = json::object();
    if (spec)
        for (const auto& p : spec->parameters)
        {
            const auto it = parameters.find(p.name);
            if (it == parameters.end())
                continue;
            // an optional parameter with its default value says nothing; without a default, a missing one counts as 0
            if (!p.required && it->second.value == p.default_si.value_or(0.0) && (!it->second.uncertainty || it->second.uncertainty->standard == 0.0))
                continue;
            const auto want = wanted.find(p.name);
            const std::string unit = want == wanted.end() ? "" : want->second;
            const std::string target = unit.empty() ? p.si_unit : unit;
            const std::string inScope = scopedUnit(p.si_unit, scope);
            Written w;
            if (sameUnit(inScope, molar(target, p.si_unit)))
                w = {inScope == p.si_unit ? "" : inScope, true};
            else
                w = {target, false};
            out[p.name] = quantityJson(it->second, p.si_unit, w, options);
        }
    // a value that the method does not declare, for example the coefficients of log K beside a density model, has no use in a file;
    // a method that the catalog does not know keeps all
    if (!spec)
        for (const auto& item : parameters)
            out[item.first] = quantityJson(item.second, "1", {}, options);
    return out;
}

auto methodsJson(const std::vector<MethodApplication>& methods, OperatesOn operatesOn, DataFormat source, const V2WriterOptions& options) -> json
{
    json list = json::array();
    for (const auto& app : methods)
    {
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, operatesOn);
        // the unit of each parameter: as written; a legacy record is in the units of the source of the method
        std::map<std::string, std::string> wanted;
        auto note = [&](const std::map<std::string, Quantity>& parameters) {
            if (!spec)
                return;
            for (const auto& p : spec->parameters)
                if (const auto it = parameters.find(p.name); it != parameters.end() && !wanted.count(p.name))
                    wanted[p.name] = writtenUnit(it->second, p.si_unit, source == DataFormat::Legacy ? p.source_unit : "");
        };
        note(app.parameters);
        for (const auto& s : app.segments)
            note(s.parameters);
        const auto scope = chooseScope(wanted, spec);

        json out = json::object();
        out["method"] = app.method;
        if (!app.implementation.empty()) out["implementation"] = app.implementation;
        if (!scope.empty())
        {
            json units = json::object();
            for (const auto& item : scope)
                units[item.first] = item.second;
            out["units"] = units;
        }
        if (!app.parameter_set.empty()) out["parameter_set"] = app.parameter_set;
        if (!app.parameters.empty())
        {
            const json values = parametersJson(app.parameters, spec, scope, wanted, options);
            // a method with only defaults: nothing to say, except that `parameters` was there
            if (!values.empty())
                out["parameters"] = values;
        }
        if (!app.segments.empty())
        {
            json segments = json::array();
            for (const auto& s : app.segments)
            {
                json one = json::object();
                if (std::isfinite(s.t_min)) one["t_min"] = limit(s.t_min, options);
                if (std::isfinite(s.t_max)) one["t_max"] = limit(s.t_max, options);
                one["parameters"] = parametersJson(s.parameters, spec, scope, wanted, options);
                segments.push_back(one);
            }
            out["segments"] = segments;
        }
        if (!app.transitions.empty())
        {
            static const std::pair<const char*, const char*> columns[] = {
                {"T", "K"}, {"dS", "J/(mol.K)"}, {"dH", "J/mol"}, {"dV", "m3/mol"}, {"dTdP", "K/Pa"}};
            json rows = json::array();
            for (const auto& row : app.transitions)
            {
                json one = json::object();
                for (const auto& column : columns)
                    if (const auto it = row.find(column.first); it != row.end())
                    {
                        const std::string source_unit = source != DataFormat::Legacy ? "" : std::string(column.first) == "dV" ? "J/bar" : std::string(column.first) == "dTdP" ? "K/bar" : "";
                        one[column.first] = simpleQuantity(it->second, column.second, source_unit, options);
                    }
                rows.push_back(one);
            }
            out["transitions"] = rows;
        }
        if (app.interpolation) out["interpolation"] = toString(*app.interpolation);
        if (app.interpolation_order != 3) out["order"] = app.interpolation_order;
        if (app.table)
        {
            json table = json::object();
            table["temperatures"] = app.table->temperatures;
            table["pressures"] = app.table->pressures;
            for (const char* name : {"G", "H", "V", "Cp", "logK"})
                if (const auto it = app.table->values.find(name); it != app.table->values.end())
                    table[name] = it->second;
            if (app.table->reference_pressure) table["reference_pressure"] = *app.table->reference_pressure;
            out["table"] = table;
        }
        const json a = applicabilityJson(app.applicability, options);
        if (!a.empty()) out["applicability"] = a;
        addExtra(out, app.extra_json);
        list.push_back(out);
    }
    return list;
}

auto methodRecordJson(const MethodDefinition& m) -> json
{
    json out = json::object();
    out["kind"] = toString(m.kind);
    out["tier"] = m.tier;
    out["operates_on"] = toString(m.operates_on);
    if (!m.applies_to_classes.empty())
    {
        json classes = json::array();
        for (const auto c : m.applies_to_classes)
            classes.push_back(toString(c));
        out["applies_to_classes"] = classes;
    }
    if (!m.contributes_to.empty()) out["contributes_to"] = m.contributes_to;
    if (!m.effects_json.empty()) out["effects"] = json::parse(m.effects_json, nullptr, false);
    if (!m.parameter_definitions.empty())
    {
        json defs = json::object();
        for (const auto& p : m.parameter_definitions)
        {
            json one = json::object();
            if (!p.unit.empty()) one["unit"] = p.unit;
            if (!p.description.empty()) one["description"] = p.description;
            if (!p.required) one["required"] = false;
            if (p.default_value) one["default"] = *p.default_value;
            defs[p.name] = one;
        }
        out["parameter_definitions"] = defs;
    }
    if (!m.implementation_id.empty()) out["implementation_id"] = m.implementation_id;
    if (!m.source_form.empty() || !m.source_expression.empty())
    {
        json source = json::object();
        if (!m.source_form.empty()) source["form"] = m.source_form;
        if (!m.source_expression.empty()) source["expression"] = m.source_expression;
        out["source"] = source;
    }
    if (!m.expression.empty())
    {
        json e = json::object();
        for (const auto& item : m.expression)
            e[item.first] = item.second;
        out["expression"] = e;
    }
    else if (!m.expression_segments.empty())
    {
        json list = json::array();
        for (const auto& piece : m.expression_segments)
        {
            json e = json::object();
            for (const auto& item : piece)
                e[item.first] = item.second;
            list.push_back(e);
        }
        out["expression"] = list;
    }
    if (!m.generator.empty()) out["generator"] = m.generator;
    if (!m.source_hash.empty()) out["source_hash"] = m.source_hash;
    if (!m.tests.empty())
    {
        json tests = json::array();
        for (const auto& t : m.tests)
        {
            json one = json{{"T", t.T}, {"P", t.P}};
            if (!t.parameters.empty())
            {
                json p = json::object();
                for (const auto& item : t.parameters) p[item.first] = item.second;
                one["parameters"] = p;
            }
            json e = json::object();
            for (const auto& item : t.expected) e[item.first] = item.second;
            one["expected"] = e;
            one["relative_tolerance"] = t.relative_tolerance;
            tests.push_back(one);
        }
        out["tests"] = tests;
    }
    if (!m.citation.empty()) out["citation"] = m.citation;
    addExtra(out, m.extra_json);
    return out;
}

auto parameterSetJson(const MethodParameterSet& p, const std::vector<MethodDefinition>& methods, const V2WriterOptions& options) -> json
{
    std::map<std::string, std::string> slots;
    if (const MethodSpec* spec = MethodCatalog::instance().find(p.method))
        for (const auto& q : spec->parameters)
            slots[q.name] = q.si_unit;
    for (const auto& m : methods)
        if (m.symbol == p.method)
            for (const auto& q : m.parameter_definitions)
                slots[q.name] = q.unit.empty() ? "1" : q.unit;
    json out = json::object();
    out["method"] = p.method;
    if (!p.parameters.empty())
    {
        json values = json::object();
        for (const auto& item : p.parameters)
        {
            const auto it = slots.find(item.first);
            values[item.first] = simpleQuantity(item.second, it == slots.end() ? "1" : it->second, "", options);
        }
        out["parameters"] = values;
    }
    if (!p.auxiliary.empty()) out["auxiliary"] = structuralJson(p.auxiliary, options);
    if (!p.reference.empty()) out["reference"] = p.reference;
    addExtra(out, p.extra_json);
    return out;
}

} // namespace ThermoFun::v2::v2writer
