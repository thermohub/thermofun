// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads the methods of a record, and what belongs to them, from a v2 file.

#include <ThermoFun/IO/V2/detail/V2ReaderParts.h>

namespace ThermoFun::model::v2reader {

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
        if (!spec)
            if (auto symbolic = c.symbolic_specs.find(app.method_as_written); symbolic != c.symbolic_specs.end() && symbolic->second.operates_on == operatesOn)
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
    // a wrong word here would give wrong values without a sign: an error, not a warning
    const auto word = [&]<typename E>(std::type_identity<E>, const char* key) -> std::optional<E> {
        const std::string text = optionalString(node, key);
        if (text.empty())
            return std::nullopt;
        const auto value = fromString<E>(text);
        if (!value)
            report(c, Severity::Error, "CNV-005", record, path + "/" + key, unknownWordMessage<E>(text));
        return value;
    };
    Convention conv;
    conv.energy_reference = word(std::type_identity<EnergyReference>{}, "energy_reference");
    if (conv.energy_reference == EnergyReference::DFTFormation0K)
        report(c, Severity::Warning, "CNV-008", record, path + "/energy_reference",
               "dft_formation_0K: the energy at 0 K is used as the enthalpy of formation at 298.15 K; the values are estimates.");
    conv.standard_pressure = word(std::type_identity<StandardPressure>{}, "standard_pressure");
    conv.concentration_scale = word(std::type_identity<ConcentrationScale>{}, "concentration_scale");
    conv.water_reference = word(std::type_identity<WaterReference>{}, "water_reference");
    return conv;
}

auto standardStateLabel(Context& c, const json& node, std::optional<Convention>& convention, const std::string& record, const std::string& path) -> void
{
    const std::string label = optionalString(node, "standard_state_convention");
    if (!node.contains("standard_state_convention"))
        return;
    report(c, Severity::Warning, "V2-KEY-003", record, path + "/standard_state_convention",
           "standard_state_convention \"" + label + "\" is not read; give convention: {standard_pressure: ...}.");
    if (label.find("1atm") != std::string::npos)
    {
        if (!convention)
            convention = Convention{};
        if (!convention->standard_pressure)
            convention->standard_pressure = StandardPressure::OneAtm;
    }
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

} // namespace ThermoFun::model::v2reader
