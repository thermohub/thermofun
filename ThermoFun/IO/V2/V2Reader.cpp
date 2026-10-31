// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/IO/V2/detail/V2ReaderParts.h>

namespace ThermoFun::v2::v2reader {

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

void versionCheck(Context& c, const json& root)
{
    if (!root.contains("thermohub") || !root["thermohub"].is_string())
    {
        report(c, Severity::Error, "V2-VER-003", "", "/", "The file has no key thermohub with the version of the format.");
        return;
    }
    const std::string version = root["thermohub"].get<std::string>();
    const int major = std::atoi(version.c_str());
    if (major != 2)
    {
        report(c, Severity::Error, "V2-VER-001", "", "/thermohub", "This file uses the format " + version + "; ThermoFun reads 2.x.");
        return;
    }
    const auto dot = version.find('.');
    if (dot != std::string::npos && std::atoi(version.c_str() + dot + 1) > knownMinor)
        report(c, Severity::Warning, "V2-VER-002", "", "/thermohub", "This file uses the format " + version + "; this ThermoFun knows 2." + std::to_string(knownMinor) + ". It keeps what it does not know.");
}

auto readDocument(const json& root, Diagnostics& diagnostics, const std::string& file) -> V2Document
{
    V2Document doc;
    Context c{diagnostics, nullptr, file};
    versionCheck(c, root);
    for (const auto& d : diagnostics.all())
        if (d.code == "V2-VER-001" || d.code == "V2-VER-003")
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

} // namespace ThermoFun::v2::v2reader

namespace ThermoFun::v2 {

using namespace v2reader;

auto readV2Document(const std::string& text, Diagnostics& diagnostics, const std::string& file) -> V2Document
{
    Diagnostics found;
    V2Document doc;
    Context c{found, nullptr, file};
    if (detail::looksLikeJson(text))
    {
        json root = json::parse(text, nullptr, false);
        if (root.is_discarded() || !root.is_object())
            report(c, Severity::Error, "V2-ID-001", "", "/", "The text is not a JSON object.");
        else
            doc = readDocument(root, found, file);
    }
    else
    {
        auto yaml = detail::readYaml(text);
        if (!yaml.ok)
        {
            Diagnostic d;
            d.severity = Severity::Error;
            d.code = "V2-YAML-001";
            d.file = file;
            d.path = "/";
            d.line = yaml.problem.line;
            d.column = yaml.problem.column;
            d.message = yaml.problem.message;
            diagnostics.add(d);
            return doc;
        }
        if (!yaml.tree.is_object())
            report(c, Severity::Error, "V2-ID-001", "", "/", "The text is not a mapping.");
        else
        {
            for (const auto& path : yaml.paths_written_twice)
                report(c, Severity::Warning, "V2-ID-003", "", path, "The key at " + path + " is written twice; the later value replaces the earlier.");
            doc = readDocument(yaml.tree, found, file);
        }
    }
    // the place in the text is looked for only if there is something to say: a parse with locations takes time
    if (!found.all().empty())
    {
        const detail::TextLocations locations(text);
        for (auto d : found.all())
        {
            std::tie(d.line, d.column) = locations.find(d.path);
            diagnostics.add(d);
        }
    }
    return doc;
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

} // namespace ThermoFun::v2
