// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/IO/Legacy/detail/LegacyReaderParts.h>

namespace ThermoFun::model::legacyreader {

void report(const Context& c, Severity severity, const std::string& code, const std::string& path, const std::string& message)
{
    if (!c.diagnostics)
        return;
    Diagnostic d;
    d.severity = severity;
    d.code = code;
    d.record = c.record;
    d.path = path;
    d.message = message;
    c.diagnostics->add(std::move(d));
}

auto isPresent(const json& j, const char* key) -> bool
{
    return j.is_object() && j.contains(key) && !j[key].is_null();
}

auto stringOf(const json& j, const char* key) -> std::string
{
    return isPresent(j, key) && j[key].is_string() ? j[key].get<std::string>() : "";
}

// The key of the first member of a map {"<code>": "<name>"}
auto codeOf(const json& j, const char* key, int fallback) -> int
{
    if (!isPresent(j, key) || !j[key].is_object() || j[key].empty())
        return fallback;
    try
    {
        return std::stoi(j[key].begin().key());
    }
    catch (const std::exception&)
    {
        return fallback;
    }
}

// The legacy record is the content of "properties" if the record is wrapped (an export of ArangoDB)
auto unwrap(json j, const Context& c) -> json
{
    if (isPresent(j, legacy::Properties) && j[legacy::Properties].is_object())
    {
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-007", where(c), "the record is wrapped in \"properties\"");
        return j[legacy::Properties];
    }
    return j;
}

auto parse(const std::string& text, const Context& c) -> json
{
    json j = json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object())
    {
        report(c, Severity::Error, "LEG-KEY-001", "", "the text is not a JSON object");
        return json::object();
    }
    return j;
}

// True if the number is exactly a float32: the legacy GEMS exports write such numbers (162.3000030517578)
auto isSinglePrecision(double value) -> bool
{
    // exports of GEMS print a float with 13 or 14 digits, for example -56.484001159668: the value is near the float, not equal to it.
    // A short decimal that a float holds is near it too, for example 0.5: its shortest form does not change on writing.
    // A whole number says nothing about its source.
    const auto f = static_cast<float>(value);
    return std::isfinite(f) && value != std::floor(value) && std::fabs(value - static_cast<double>(f)) <= 2e-12 * std::fabs(value);
}

// A value with its unit: {"values": [x], "units": [u], "errors": [e]}
// The slot has a unit of the legacy file (unitDefault) and one in SI (si). A volume in J/bar is per mole: its
// unit in the conversion is m3 (siConvert), and the model keeps m3/mol.
auto readQuantity(const json& j, const char* key, const std::string& unitDefault, const std::string& siConvert, const Context& c)
    -> std::optional<Quantity>
{
    if (!isPresent(j, key) || !j[key].is_object())
        return std::nullopt;
    const auto& entry = j[key];

    std::string unit = unitDefault;
    std::string written;
    if (entry.contains("units") && entry["units"].is_array() && !entry["units"].empty() && entry["units"][0].is_string())
    {
        unit = entry["units"][0].get<std::string>();
        written = unit;
    }

    if (!entry.contains("values") || !entry["values"].is_array() || entry["values"].empty() ||
        !entry["values"][0].is_number())
    {
        if (entry.contains("values") && entry["values"].is_array() && !entry["values"].empty())
            legacy::reportQuirk(c.diagnostics, "LEG-QRK-008", where(c, key), "the value is null: the property is missing");
        return std::nullopt;
    }

    // 7777777 is the mark of GEMS exports for no value (slop98: G and H of 40 minerals)
    if (entry["values"][0].get<double>() == 7777777.0)
    {
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-013", where(c, key), "7777777 is the mark for no value: the property is missing");
        return std::nullopt;
    }

    Quantity q;
    q.unit_as_written = written;
    q.single_precision_source = isSinglePrecision(entry["values"][0].get<double>());
    try
    {
        q.value = units::convertAny(entry["values"][0].get<double>(), unit, siConvert);
        if (entry.contains("errors") && entry["errors"].is_array() && !entry["errors"].empty() && entry["errors"][0].is_number())
        {
            Uncertainty u;
            u.standard = std::fabs(units::convertDifference(entry["errors"][0].get<double>(), unit, siConvert)) /
                         c.options.error_coverage_factor;
            u.coverage_factor_as_written = c.options.error_coverage_factor;
            u.convention_unknown = true;
            q.uncertainty = u;
        }
    }
    catch (const std::exception& e)
    {
        report(c, Severity::Error, "V2-UNIT-001", key, e.what());
        return std::nullopt;
    }
    return q;
}

// Keys that ThermoFun does not read are reported, not kept
void reportUnknownKeys(const json& j, const std::set<std::string>& known, const Context& c)
{
    for (auto it = j.begin(); it != j.end(); ++it)
        if (!known.count(it.key()))
            report(c, Severity::Warning, "LEG-KEY-001", it.key(), "the key " + it.key() + " is not read");
}

// "datasources" and "references": a string or a list of strings
auto readSources(const json& j) -> std::vector<std::string>
{
    std::vector<std::string> out;
    for (const char* key : {legacy::Datasources, "datasources ", legacy::References})
    {
        if (!isPresent(j, key))
            continue;
        if (j[key].is_string())
            out.push_back(j[key].get<std::string>());
        else if (j[key].is_array())
            for (const auto& v : j[key])
                if (v.is_string())
                    out.push_back(v.get<std::string>());
    }
    return out;
}

} // namespace ThermoFun::model::legacyreader

namespace ThermoFun::model {

using namespace legacyreader;

void checkReading(const Diagnostics& diagnostics)
{
    for (const auto& d : diagnostics.errors())
    {
        // the one error that refuses the record, as the old parser did: the others leave a record with what could be read
        if (d.code == "LEG-QRK-009")
            funError("Too many coefficient values", toString(d), __LINE__, __FILE__);
        thfun_logger->error("{}", toString(d));
    }
}

auto readLegacySubstance(const std::string& text, const LegacyReadOptions& options, Diagnostics* diagnostics) -> SubstanceRecord
{
    SubstanceRecord s;
    s.source_format = DataFormat::Legacy;
    s.original_text = text;
    Context c{options, diagnostics, ""};
    json j = unwrap(parse(text, c), c);

    s.symbol = stringOf(j, legacy::Symbol);
    c.record = s.symbol;
    s.name = stringOf(j, legacy::Name);
    s.formula = stringOf(j, legacy::Formula);
    if (isPresent(j, legacy::FormulaCharge) && j[legacy::FormulaCharge].is_number())
        s.charge = j[legacy::FormulaCharge].get<int>();
    s.defining_reaction = stringOf(j, legacy::Reaction);

    // molar mass: g/mol in the file, kg/mol in the model; 0 g/mol if the file gives none, as the legacy parser does
    s.molar_mass = readQuantity(j, legacy::MassPerMole, "g/mol", "kg/mol", c);

    if (!isPresent(j, legacy::AggregateState) || !j[legacy::AggregateState].is_object() || j[legacy::AggregateState].empty())
    {
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-006", where(c, legacy::AggregateState), "no aggregate state: \"other\" is used");
        s.aggregate_state = AggregateState::Other;
    }
    else
        s.aggregate_state = aggregateStateFromLegacy(codeOf(j, legacy::AggregateState, 101));

    if (isPresent(j, legacy::Class) && j[legacy::Class].is_object() && !j[legacy::Class].empty())
        s.class_ = substanceClassFromLegacy(codeOf(j, legacy::Class, 101));

    s.applicability = readLimits(j);
    if (isPresent(j, legacy::Tst) && j[legacy::Tst].is_number())
        s.reference_conditions.T = j[legacy::Tst].get<double>();
    if (isPresent(j, legacy::Pst) && j[legacy::Pst].is_number())
        s.reference_conditions.P = j[legacy::Pst].get<double>();

    struct Slot
    {
        const char* key;
        PropertyName property;
        const char* unit;
        const char* si;
    };
    for (const Slot& slot : {Slot{legacy::SmGibbsEnergy, PropertyName::G, "J/mol", "J/mol"},
                             Slot{legacy::SmEnthalpy, PropertyName::H, "J/mol", "J/mol"},
                             Slot{legacy::SmEntropyAbs, PropertyName::S, "J/(mol*K)", "J/(mol.K)"},
                             Slot{legacy::SmHeatCapacityP, PropertyName::Cp, "J/(mol*K)", "J/(mol.K)"},
                             Slot{legacy::SmVolume, PropertyName::V, "J/bar", "m3"}})
        if (auto q = readQuantity(j, slot.key, slot.unit, slot.si, c))
            s.standard_state_properties[slot.property] = *q;

    readSubstanceMethods(j, s, c);

    s.data_sources = readSources(j);
    reportUnknownKeys(j, knownSubstanceKeys(), c);
    // In the old format a gas defined by a reaction has the volume of an ideal gas, without saying so. In a record the method says it.
    if (!s.defining_reaction.empty() && s.methods.empty() && s.class_ == SubstanceClass::GasFluid && s.aggregate_state == AggregateState::Gas)
    {
        MethodApplication ideal;
        ideal.method = "IdealGas";
        ideal.implied = true;
        s.methods.push_back(ideal);
    }
    return s;
}

auto readLegacyReaction(const std::string& text, const LegacyReadOptions& options, Diagnostics* diagnostics) -> ReactionRecord
{
    ReactionRecord r;
    r.source_format = DataFormat::Legacy;
    r.original_text = text;
    Context c{options, diagnostics, ""};
    json j = unwrap(parse(text, c), c);

    r.symbol = stringOf(j, legacy::Symbol);
    c.record = r.symbol;
    r.name = stringOf(j, legacy::Name);
    r.equation = stringOf(j, legacy::Equation);

    if (r.symbol.empty())
        r.symbol = r.equation;

    if (isPresent(j, legacy::Reactants))
    {
        if (j[legacy::Reactants].is_array())
            for (const auto& item : j[legacy::Reactants])
                if (item.is_object() && isPresent(item, legacy::Symbol) && isPresent(item, "coefficient") && item["coefficient"].is_number())
                    r.stoichiometry.emplace_back(item[legacy::Symbol].get<std::string>(), item["coefficient"].get<double>());
    }
    else if (!r.equation.empty())
        r.stoichiometry = stoichiometryOfEquation(r.equation);

    r.applicability = readLimits(j);
    if (isPresent(j, legacy::Tst) && j[legacy::Tst].is_number())
        r.reference_conditions.T = j[legacy::Tst].get<double>();
    if (isPresent(j, legacy::Pst) && j[legacy::Pst].is_number())
        r.reference_conditions.P = j[legacy::Pst].get<double>();

    struct Slot
    {
        const char* key;
        ReactionPropertyName property;
        const char* unit;
        const char* si;
    };
    for (const Slot& slot : {Slot{legacy::LogKr, ReactionPropertyName::logK, "1", "1"},
                             Slot{legacy::DrsmGibbsEnergy, ReactionPropertyName::dG, "J/mol", "J/mol"},
                             Slot{legacy::DrsmEnthalpy, ReactionPropertyName::dH, "J/mol", "J/mol"},
                             Slot{legacy::DrsmEntropy, ReactionPropertyName::dS, "J/(mol*K)", "J/(mol.K)"},
                             Slot{legacy::DrsmHeatCapacityP, ReactionPropertyName::dCp, "J/(mol*K)", "J/(mol.K)"},
                             Slot{legacy::DrsmVolume, ReactionPropertyName::dV, "J/bar", "m3"}})
        if (auto q = readQuantity(j, slot.key, slot.unit, slot.si, c))
            r.reaction_properties[slot.property] = *q;

    readReactionMethods(j, r, c);
    r.data_sources = readSources(j);
    reportUnknownKeys(j, knownReactionKeys(), c);
    return r;
}

auto readLegacyElement(const std::string& text, const LegacyReadOptions& options, Diagnostics* diagnostics) -> ElementRecord
{
    ElementRecord e;
    e.source_format = DataFormat::Legacy;
    e.original_text = text;
    Context c{options, diagnostics, ""};
    json j = unwrap(parse(text, c), c);

    e.symbol = stringOf(j, legacy::Symbol);
    c.record = e.symbol;
    e.name = stringOf(j, legacy::Name);
    if (isPresent(j, legacy::Number) && j[legacy::Number].is_number())
        e.number = j[legacy::Number].get<int>();
    if (isPresent(j, legacy::IsotopeMass) && j[legacy::IsotopeMass].is_number())
        e.isotope_mass = j[legacy::IsotopeMass].get<int>();

    const int code = (isPresent(j, legacy::Class) && j[legacy::Class].is_object()) ? codeOf(j, legacy::Class, 0) : 0;
    e.class_ = elementClassFromLegacy(code);
    if (e.class_ == ElementClass::Other)
        e.legacy_class = code;
    if (e.class_ == ElementClass::Other)
        report(c, Severity::Warning, "LEG-KEY-001", legacy::Class, "the element class " + std::to_string(code) + " has no word: `other` is used");

    e.molar_mass = readQuantity(j, legacy::AtomicMass, "g/mol", "kg/mol", c);
    e.standard_entropy = readQuantity(j, legacy::Entropy, "J/(mol*K)", "J/(mol.K)", c);
    e.heat_capacity = readQuantity(j, legacy::HeatCapacity, "J/(mol*K)", "J/(mol.K)", c);
    e.volume = readQuantity(j, legacy::Volume, "J/bar", "m3", c);
    e.data_sources = readSources(j);

    reportUnknownKeys(j, knownElementKeys(), c);
    return e;
}

// The text of a list of entries, as a file has it
auto tpMethodsOf(const std::vector<LegacyMethodEntry>& entries) -> json
{
    json out = json::array();
    for (const auto& e : entries)
    {
        json entry;
        entry[legacy::Method] = {{std::to_string(e.id), e.name}};
        for (const auto& [key, values] : e.arrays)
        {
            entry[key] = {{"values", values}};
            if (const auto it = e.errors.find(key); it != e.errors.end() && !it->second.empty())
                entry[key]["errors"] = it->second;
        }
        if (e.temperature_limits.size() == 2)
            entry[legacy::LimitsTP] = {{"lowerT", e.temperature_limits[0]}, {"upperT", e.temperature_limits[1]}};
        if (!e.phase_transition.empty())
            entry[legacy::MPhaseTransProps] = {{"values", e.phase_transition}};
        if (e.has_table)
        {
            json table = {{"temperatures", e.table_temperatures}, {"pressures", e.table_pressures}};
            for (const auto& [key, values] : e.table_values)
                table[key] = values;
            if (e.table_reference_pressure != 0.0)
                table["reference_pressure"] = e.table_reference_pressure;
            entry[legacy::TpTable] = table;
        }
        out.push_back(entry);
    }
    return out;
}

void readLegacyMethods(const std::vector<LegacyMethodEntry>& entries, double expansivity, double compressibility, SubstanceRecord& record, Diagnostics* diagnostics)
{
    json j;
    j[legacy::TPMethods] = tpMethodsOf(entries);
    if (expansivity != 0.0)
        j[legacy::MExpansivity] = {{"values", {expansivity}}};
    if (compressibility != 0.0)
        j[legacy::MCompressibility] = {{"values", {compressibility}}};
    const LegacyReadOptions options;
    Context c{options, diagnostics, record.symbol};
    record.structural.erase("alpha0");
    record.structural.erase("kappa0");
    record.preferred_property_source.reset();
    readSubstanceMethods(j, record, c);
}

void readLegacyMethods(const std::vector<LegacyMethodEntry>& entries, ReactionRecord& record, Diagnostics* diagnostics)
{
    json j;
    j[legacy::TPMethods] = tpMethodsOf(entries);
    const LegacyReadOptions options;
    Context c{options, diagnostics, record.symbol};
    record.preferred_property_source.reset();
    readReactionMethods(j, record, c);
}

auto readLegacyDocumentInfo(const std::string& text) -> ThermoDataSetInfo
{
    ThermoDataSetInfo info;
    json j = json::parse(text, nullptr, false);
    if (j.is_discarded() || !j.is_object())
        return info;
    info.symbol = stringOf(j, legacy::ThermoDataset);
    info.name = info.symbol;
    info.legacy_date = stringOf(j, legacy::Date);
    if (isPresent(j, legacy::Datasources) && j[legacy::Datasources].is_array())
        for (const auto& v : j[legacy::Datasources])
            if (v.is_string())
                info.legacy_datasources.push_back(v.get<std::string>());
    if (isPresent(j, legacy::Conventions) && j[legacy::Conventions].is_object())
    {
        const std::string ref = stringOf(j[legacy::Conventions], "water_triple_point_reference");
        if (!ref.empty())
            info.water_triple_point_reference = ref;
    }
    return info;
}

} // namespace ThermoFun::model
