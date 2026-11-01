// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads one record of each kind from a v2 file.

#include <ThermoFun/IO/V2/detail/V2ReaderParts.h>

namespace ThermoFun::v2::v2reader {

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
                                                "sourcetdb", "structural", "units", "record_type", "legacy_class"};
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
    r.legacy_class = optionalInt(node, "legacy_class");
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
        "applicability", "data_source", "data_sources", "structural", "deprecated", "deprecated_in_favour_of", "units", "record_type", "covariances"};
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
        "data_sources", "applicability", "convention", "preferred_property_source", "sourcetdb", "deprecated", "units", "record_type", "covariances"};
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
    static const std::set<std::string> known = {"symbol", "method", "implementation", "description", "parameters", "auxiliary", "reference", "units"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return set;
    set.symbol = symbol;
    set.method = optionalString(node, "method");
    set.implementation = optionalString(node, "implementation");
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
    set.description = optionalString(node, "description");
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

} // namespace ThermoFun::v2::v2reader
