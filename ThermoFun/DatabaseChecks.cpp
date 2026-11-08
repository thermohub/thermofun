// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Writes the records of a database, and checks them.

#include "DatabaseImpl.h"
#include <ThermoFun/Methods/Symbolic/SymbolicMethod.h>

namespace ThermoFun {

namespace {

auto allRecords(const Database& db) -> model::V2Records
{
    model::V2Records out;
    out.dataset = db.thermoDataSet();
    out.dataset.water_triple_point_reference = db.conventions().water_triple_point_reference;
    for (const auto& item : db.mapElements())
        out.elements.push_back(*item.second.record());
    for (const auto& item : db.mapSubstances())
        out.substances.push_back(*item.second.record());
    for (const auto& item : db.mapReactions())
        out.reactions.push_back(*item.second.record());
    for (const auto& item : db.methods())
        out.methods.push_back(item.second);
    for (const auto& item : db.parameterSets())
        out.parameter_sets.push_back(item.second);
    for (const auto& item : db.dataSources())
        out.data_sources.push_back(item.second);
    return out;
}

} // namespace

auto Database::toJson(bool readable) const -> std::string
{
    return model::writeV2Json(allRecords(*this), {readable});
}

auto Database::toYaml(bool readable) const -> std::string
{
    return model::writeV2Yaml(allRecords(*this), {readable});
}

auto Database::toLegacyJson(bool useOriginalText) const -> std::string
{
    Diagnostics problems;
    const std::string text = model::writeLegacyDocument(allRecords(*this), {useOriginalText}, &problems);
    for (const auto& d : problems.all())
        thfun_logger->warn("{} {}: {}", d.code, d.record, d.message);
    return text;
}

auto Database::legacyWriteProblems() const -> Diagnostics
{
    Diagnostics problems;
    model::writeLegacyDocument(allRecords(*this), {false}, &problems);
    return problems;
}

namespace {

/// The checks of the conventions of the records (14 section 5 and section 7)
void checkConventions(const Database& db, const model::V2Records& records, Diagnostics& all)
{
    const auto add = [&](Severity severity, const char* code, const std::string& record, const std::string& path, const std::string& message) {
        Diagnostic d;
        d.severity = severity;
        d.code = code;
        d.record = record;
        d.path = path;
        d.message = message;
        all.add(d);
    };
    const auto join = [](const std::vector<std::string>& words) {
        std::string text;
        for (const auto& w : words)
            text += (text.empty() ? "" : ", ") + w;
        return text;
    };
    const auto datasetReference = records.dataset.convention.energy_reference.value_or(model::EnergyReference::BensonHelgeson);
    bool hasSolvent = false;
    std::map<std::string, model::EnergyReference> references;
    for (const auto& r : records.substances)
    {
        hasSolvent = hasSolvent || r.class_ == model::SubstanceClass::WaterSolvent;
        const model::Convention own = r.convention.value_or(model::Convention{});
        std::optional<model::EnergyReference> native;
        for (const auto& a : r.methods)
            if (const auto* spec = model::MethodCatalog::instance().find(a.method, model::OperatesOn::Substance); spec && spec->effects.base && spec->native_convention)
                native = spec->native_convention;
        if (native && !own.energy_reference && *native != datasetReference)
            add(Severity::Warning, "CNV-003", r.symbol, "/substances/" + r.symbol + "/methods",
                "The method gives " + model::toString(*native) + ", the dataset uses " + model::toString(datasetReference) + ", and the record declares no convention. The values are converted.");
        const auto reference = native ? *native : own.energy_reference.value_or(datasetReference);
        references[r.symbol] = reference;
        if (reference != model::EnergyReference::BensonHelgeson && !r.formula.empty())
        {
            const auto sources = db.elementEntropySources(r.formula);
            if (!sources.missing.empty())
                add(Severity::Error, "CNV-001", r.symbol, "/substances/" + r.symbol + "/formula", "A conversion needs the entropies of the elements " + join(sources.missing) + ", and no source has them.");
            if (!sources.from_table.empty())
                add(Severity::Warning, "CNV-006", r.symbol, "/substances/" + r.symbol + "/formula", "The entropies of the elements " + join(sources.from_table) + " come from the element table of ChemicalFun, not from the database.");
        }
    }
    for (const auto& r : records.substances)
        if (r.class_ == model::SubstanceClass::AqueousSolute && !hasSolvent &&
            mergeConvention(r.convention.value_or(model::Convention{}), records.dataset.convention).concentration_scale == model::ConcentrationScale::Molarity)
            add(Severity::Error, "CNV-002", r.symbol, "/substances/" + r.symbol + "/convention", "A solute in molarity needs a solvent, and the database has none.");
    for (const auto& r : records.reactions)
    {
        std::set<model::EnergyReference> seen;
        for (const auto& [symbol, coefficient] : r.stoichiometry)
            if (const auto it = references.find(symbol); it != references.end())
                seen.insert(it->second);
        if (seen.size() > 1)
            add(Severity::Warning, "CNV-004", r.symbol, "/reactions/" + r.symbol + "/equation", "The species of this reaction use different energy references. The values are converted.");
    }
}

} // namespace

auto Database::validate() const -> Diagnostics
{
    const auto parse = [this](const std::string& formula) {
        std::map<std::string, double> out;
        for (const auto& element : parseSubstanceFormula(formula))
            out[element.first.symbol()] += element.second;
        return out;
    };
    const auto records = allRecords(*this);
    Diagnostics all = model::validateRecords(records, parse);
    for (const auto& definition : records.methods)
        model::checkSymbolicDefinition(definition, all);
    for (const auto& record : records.substances)
    {
        const Diagnostics conflicts = model::checkMethods(record);
        for (const auto& d : conflicts.all())
            all.add(d);
        if (model::isHydrogenIon(record) && model::hydrogenIonConventionIsZero(record, records.dataset.convention) && model::recordHasOwnValues(record))
        {
            Diagnostic d;
            d.severity = Severity::Warning;
            d.code = "CNV-007";
            d.record = record.symbol;
            d.path = "/substances/" + record.symbol;
            d.message = "The geochemical convention defines the properties of the hydrogen ion as 0, and this record gives other values. They are used.";
            all.add(d);
        }
    }
    checkConventions(*this, records, all);
    return all;
}

auto Database::save(const std::string& path, model::DataFormat format, bool readable) const -> void
{
    const auto endsWith = [&](const std::string& tail) { return path.size() >= tail.size() && path.compare(path.size() - tail.size(), tail.size(), tail) == 0; };
    const std::string text = format == model::DataFormat::Legacy ? toLegacyJson(false) : endsWith(".yaml") || endsWith(".yml") ? toYaml(readable) : toJson(readable);
    std::ofstream out(path);
    if (!out.good())
        funError("File writing error", "The file " + path + " cannot be written.", __LINE__, __FILE__);
    out << text;
}

auto checkRecord(const std::string& section, const std::string& symbol, const std::string& record) -> Diagnostics
{
    nlohmann::json document = {{"thermohub", "2.0.0-draft.1"}, {section, {{symbol, nlohmann::json::parse(record)}}}};
    const std::string text = document.dump();
    Diagnostics found, out;
    model::readV2Document(text, found);
    if (found.ok())
    {
        const Diagnostics problems = Database(text).validate();
        for (const auto& d : problems.all())
            found.add(d);
    }
    for (const auto& d : found.all())
        if (d.code.rfind("V2-REF", 0) != 0)
            out.add(d);
    return out;
}

auto legacyFileDiagnostics(const std::string& file) -> Diagnostics
{
    Diagnostics diagnostics;
    std::ifstream in(file);
    if (!in.good())
        return diagnostics;
    std::stringstream text;
    text << in.rdbuf();
    const auto j = nlohmann::json::parse(text.str(), nullptr, false);
    if (j.is_discarded() || model::detail::detectDocumentFormat(j) == model::DataFormat::V2)
        return diagnostics;
    const auto read = [&](const nlohmann::json& record, const std::string& kind) {
        if (kind == "element")
            model::readLegacyElement(record.dump(), {}, &diagnostics);
        else if (kind == "substance")
            model::readLegacySubstance(record.dump(), {}, &diagnostics);
        else if (kind == "reaction")
            model::readLegacyReaction(record.dump(), {}, &diagnostics);
    };
    for (const auto* section : {"element", "substance", "reaction"})
        if (const auto key = std::string(section) + "s"; j.is_object() && j.contains(key))
            for (const auto& record : j[key])
                read(record, section);
    if (j.is_array())
        for (const auto& record : j)
            if (record.contains("_label"))
                read(record, record["_label"].get<std::string>());
    return diagnostics;
}

} // namespace ThermoFun
