// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads one record of each kind from a v2 file.

#include <algorithm>
#include <ThermoFun/Common/PhysicalConstants.h>
#include <cmath>
#include <ThermoFun/IO/ReferenceCode.h>
#include <ThermoFun/IO/V2/detail/V2ReaderParts.h>

namespace ThermoFun::model::v2reader {

// The reference temperature, and the reference pressure, which is the standard pressure p° (decision 0031, row 7): a P that a
// file gives is checked against p°, never read as another pressure
auto readReferenceConditions(Context& c, const json& node, const UnitScope& scope, const std::string& record,
                             const std::string& path, ReferenceConditions fallback, double standardPressure) -> ReferenceConditions
{
    fallback.P = standardPressure;
    if (node.contains("reference_conditions") && node["reference_conditions"].is_object())
    {
        const json& rc = node["reference_conditions"];
        if (auto q = quantity(c, rc, "T", "K", scope, record, path + "/reference_conditions"))
            fallback.T = q->value;
        if (auto q = quantity(c, rc, "P", "Pa", scope, record, path + "/reference_conditions"))
            if (std::fabs(q->value - standardPressure) > 1e-9 * standardPressure)
                report(c, Severity::Error, "CNV-010", record, path + "/reference_conditions/P",
                       "The reference pressure " + std::to_string(q->value) + " Pa is not the standard pressure " + standardPressureText(standardPressure) +
                           ": the values are at p°. Give the pressure in standard_state: {pressure: ...}, and leave out reference_conditions.P.");
    }
    return fallback;
}

// The standard pressure of a record: its own, else that of the dataset, else 1 bar
auto standardPressureOf(const Context& c, const std::optional<Convention>& own) -> double
{
    if (own && own->standard_state.pressure)
        return *own->standard_state.pressure;
    if (c.dataset && c.dataset->convention.standard_state.pressure)
        return *c.dataset->convention.standard_state.pressure;
    return 1e5;
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
                                                "standard_entropy", "heat_capacity", "volume", "reference_state", "data_sources",
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
    r.reference_state = optionalString(node, "reference_state");
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
    r.file.format = DataFormat::V2;
    r.file.text = node.dump();
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
        "standard_state", "energy_reference", "reference_conditions", "standard_state_properties", "methods", "defining_reaction", "preferred_property_source",
        "applicability", "data_sources", "structural", "deprecated", "deprecated_in_favour_of", "units", "record_type", "covariances"};
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
    r.convention = readConvention(c, node, scope, symbol, path);
    standardStateLabel(c, node, r.convention, symbol, path);
    r.reference_conditions = readReferenceConditions(c, node, scope, symbol, path,
                                                     c.dataset ? c.dataset->reference_conditions : ReferenceConditions{}, standardPressureOf(c, r.convention));
    bool formationG = false;   // dfG: the NEA name of G in benson_helgeson (decision 0031, row 5)
    if (node.contains("standard_state_properties") && node["standard_state_properties"].is_object())
        for (const auto& item : node["standard_state_properties"].items())
        {
            // dfG and dfH, the NEA names, are G and H; H at Tr is dfH in every energy reference, G is dfG only in some
            if (item.key() == "dfG")
                formationG = true;
            auto name = item.key() == "dfG" ? std::optional<PropertyName>(PropertyName::G)
                        : item.key() == "dfH" ? std::optional<PropertyName>(PropertyName::H)
                                              : fromString<PropertyName>(item.key());
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
    if (formationG)
    {
        const auto own = r.convention ? r.convention->energy_reference.elements : std::nullopt;
        const auto reference = own ? own : (c.dataset ? c.dataset->convention.energy_reference.elements : std::nullopt);
        if (reference && *reference != EnergyReference::BensonHelgeson && *reference != EnergyReference::GibbsHelmholtzFormation)
            report(c, Severity::Warning, "CNV-009", symbol, path + "/standard_state_properties/dfG",
                   "dfG is the Gibbs energy of formation, but the energy reference " + toString(*reference) +
                       " counts G from another zero: the value is read as G of that reference.");
    }
    if (node.contains("methods"))
        r.methods = readMethods(c, node["methods"], OperatesOn::Substance, scope, symbol, path + "/methods");
    r.defining_reaction = optionalString(node, "defining_reaction");
    r.preferred_property_source = readPropertySource(c, node, symbol, path);
    if (node.contains("applicability"))
        r.applicability = readApplicability(c, node["applicability"], scope, symbol, path + "/applicability");
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
    r.file.format = DataFormat::V2;
    r.file.text = node.dump();
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
        "symbol", "name", "equation", "stoichiometry", "reference_conditions", "reaction_properties", "methods", "level",
        "data_sources", "applicability", "standard_state", "energy_reference", "preferred_property_source", "sourcetdb", "deprecated", "units", "record_type", "covariances"};
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
    r.convention = readConvention(c, node, scope, symbol, path);
    r.reference_conditions = readReferenceConditions(c, node, scope, symbol, path,
                                                     c.dataset ? c.dataset->reference_conditions : ReferenceConditions{}, standardPressureOf(c, r.convention));
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
    r.preferred_property_source = readPropertySource(c, node, symbol, path);
    r.data_sources = stringList(node, "data_sources");
    r.deprecated = node.contains("deprecated") && node["deprecated"].is_boolean() && node["deprecated"].get<bool>();
    r.covariance = readCovariances(c, node, symbol, path);
    r.extra_json = extraKeys(c, node, known, symbol, path);
    r.file.format = DataFormat::V2;
    r.file.text = node.dump();
    return r;
}

auto readMethodRecord(Context& c, const json& node, const std::string& key, const UnitScope&, const std::string& path) -> MethodDefinition
{
    MethodDefinition m;
    if (!node.is_object())
    {
        report(c, Severity::Error, "V2-ID-001", key, path, "A method must be a mapping.");
        return m;
    }
    static const std::set<std::string> known = {"symbol", "kind", "operates_on", "applies_to_classes", "contributes_to",
                                                "parameter_definitions", "implementation_id", "formula", "expression", "generator",
                                                "formula_hash", "tests", "data_sources"};
    const std::string symbol = symbolOf(c, node, key, path);
    if (symbol.empty() || (!key.empty() && node.contains("symbol") && node["symbol"].is_string() && node["symbol"].get<std::string>() != key))
        return m;
    m.symbol = symbol;
    if (node.contains("kind") && node["kind"].is_string())
        m.kind = vocabulary<MethodKind>(c, node["kind"].get<std::string>(), symbol, path + "/kind").value_or(m.kind);
    if (node.contains("operates_on") && node["operates_on"].is_string())
        m.operates_on = vocabulary<OperatesOn>(c, node["operates_on"].get<std::string>(), symbol, path + "/operates_on").value_or(m.operates_on);
    for (const auto& word : stringList(node, "applies_to_classes"))
        if (const auto cls = vocabulary<SubstanceClass>(c, word, symbol, path + "/applies_to_classes"))
            m.applies_to_classes.push_back(*cls);
    // contributes_to: a map property -> sets / adds / uses, or a short list of properties that each add
    if (node.contains("contributes_to"))
    {
        const json& block = node["contributes_to"];
        const std::string where = path + "/contributes_to";
        if (block.is_array() || block.is_string())
        {
            m.contributes_to_list = true;
            for (const auto& name : stringList(node, "contributes_to"))
                m.contributes_to.emplace_back(name, "adds");
        }
        else if (block.is_object())
            for (const auto& item : block.items())
            {
                const std::string word = item.value().is_string() ? item.value().get<std::string>() : std::string();
                if (word != "sets" && word != "adds" && word != "uses")
                    report(c, Severity::Error, "V2-VOC-001", symbol, where + "/" + item.key(),
                           "\"" + word + "\" is not a value of contributes_to. Allowed: sets, adds, uses.");
                else
                    m.contributes_to.emplace_back(item.key(), word);
            }
        else
            report(c, Severity::Error, "V2-VOC-001", symbol, where, "contributes_to is a mapping (property: sets, adds or uses) or a list of properties.");
    }
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
    if (node.contains("formula") && node["formula"].is_object())
    {
        m.formula_form = optionalString(node["formula"], "form");
        m.formula_expression = optionalString(node["formula"], "expression");
        m.formula_json = node["formula"].dump();
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
    m.formula_hash = optionalString(node, "formula_hash");
    if (node.contains("tests") && node["tests"].is_array())
        for (const auto& t : node["tests"])
        {
            ReferenceTestCase test;
            if (t.contains("T") && t["T"].is_number()) test.T = t["T"].get<double>();
            if (t.contains("P") && t["P"].is_number()) test.P = t["P"].get<double>();
            if (t.contains("piece") && t["piece"].is_number()) test.piece = t["piece"].get<int>();
            if (t.contains("reference") && t["reference"].is_object())
                for (const auto& item : t["reference"].items())
                    if (item.value().is_number())
                        test.reference[item.key()] = item.value().get<double>();
            if (t.contains("relative_tolerance") && t["relative_tolerance"].is_number())
                test.relative_tolerance = t["relative_tolerance"].get<double>();
            for (const char* block : {"parameters", "expected"})
                if (t.contains(block) && t[block].is_object())
                    for (const auto& item : t[block].items())
                        if (item.value().is_number())
                            (std::string(block) == "parameters" ? test.parameters : test.expected)[item.key()] = item.value().get<double>();
            m.tests.push_back(test);
        }
    m.data_sources = stringList(node, "data_sources");
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
    static const std::set<std::string> known = {"symbol", "method", "implementation", "default", "formula_units", "description", "parameters", "data_sources", "units"};
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
    set.description = optionalString(node, "description");
    if (node.contains("formula_units") && node["formula_units"].is_object())
        for (const auto& item : node["formula_units"].items())
            set.formula_units += (set.formula_units.empty() ? "" : ", ") + item.key() + ": " + (item.value().is_string() ? item.value().get<std::string>() : item.value().dump());
    set.is_default = !node.contains("default") || !node["default"].is_boolean() || node["default"].get<bool>();
    set.data_sources = stringList(node, "data_sources");
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
    if (!isReferenceCode(symbol))
        report(c, Severity::Warning, "V2-SRC-001", symbol, path, "The symbol " + symbol + " is not an NEA reference code such as 1995ROB/HEM or 1995ROB/HEM2.");
    return d;
}

auto readDataset(Context& c, const json& node, const UnitScope& scope) -> ThermoDataSetInfo
{
    ThermoDataSetInfo info;
    if (!node.is_object())
        return info;
    static const std::set<std::string> known = {"symbol", "name", "version", "status", "standard_state_convention", "reference_conditions",
                                                "sourcetdb", "solvent", "data_sources", "defaults", "standard_state", "energy_reference", "physical_constants"};
    info.symbol = optionalString(node, "symbol");
    info.name = optionalString(node, "name");
    info.version = optionalString(node, "version");
    if (node.contains("status") && node["status"].is_string())
        info.status = vocabulary<DatasetStatus>(c, node["status"].get<std::string>(), info.symbol, "/thermodataset/status").value_or(info.status);
    if (info.status == DatasetStatus::Published && info.version.empty())
        report(c, Severity::Error, "V2-DS-001", info.symbol, "/thermodataset", "A published dataset needs a version.");
    if (auto conv = readConvention(c, node, scope, info.symbol, "/thermodataset"))
        info.convention = *conv;
    {
        std::optional<Convention> conv = info.convention;
        standardStateLabel(c, node, conv, info.symbol, "/thermodataset");
        info.convention = *conv;
    }
    info.reference_conditions = readReferenceConditions(c, node, scope, info.symbol, "/thermodataset", ReferenceConditions{},
                                                        info.convention.standard_state.pressure.value_or(1e5));
    info.sourcetdb = stringList(node, "sourcetdb");
    info.solvent = optionalString(node, "solvent");
    info.data_sources = stringList(node, "data_sources");
    info.physical_constants = optionalString(node, "physical_constants");
    if (!info.physical_constants.empty())
    {
        const auto names = physicalConstantsNames();
        if (std::find(names.begin(), names.end(), info.physical_constants) == names.end())
        {
            std::string allowed;
            for (const auto& n : names)
                allowed += (allowed.empty() ? "" : ", ") + n;
            report(c, Severity::Error, "V2-VOC-001", info.symbol, "/thermodataset/physical_constants",
                   "\"" + info.physical_constants + "\" is not a set of constants of physics. Allowed: " + allowed + ".");
        }
    }
    if (node.contains("defaults") && node["defaults"].is_object())
    {
        info.defaults.quality = optionalString(node["defaults"], "quality");
        info.defaults.origin = vocabulary<Origin>(c, optionalString(node["defaults"], "origin"), info.symbol, "/thermodataset/defaults/origin");
    }
    info.extra_json = extraKeys(c, node, known, info.symbol, "/thermodataset");
    return info;
}

} // namespace ThermoFun::model::v2reader
