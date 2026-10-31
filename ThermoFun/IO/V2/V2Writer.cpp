// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/IO/V2/detail/V2WriterParts.h>

namespace ThermoFun::v2::v2writer {

// a legacy record can have an empty citation
auto sourcesOf(const std::vector<std::string>& sources) -> std::vector<std::string>
{
    std::vector<std::string> out;
    std::copy_if(sources.begin(), sources.end(), std::back_inserter(out), [](const std::string& s) { return !s.empty(); });
    return out;
}

auto applicabilityJson(const Applicability& a, const V2WriterOptions& options) -> json
{
    json out = json::object();
    if (a.t_min) out["t_min"] = limit(*a.t_min, options);
    if (a.t_max) out["t_max"] = limit(*a.t_max, options);
    if (a.p_min) out["p_min"] = limit(*a.p_min, options);
    if (a.p_max) out["p_max"] = limit(*a.p_max, options);
    if (a.is_hard) out["hard"] = true;
    return out;
}

auto conventionJson(const Convention& c) -> json
{
    json out = json::object();
    if (c.energy_reference) out["energy_reference"] = toString(*c.energy_reference);
    if (c.standard_pressure) out["standard_pressure"] = toString(*c.standard_pressure);
    if (c.concentration_scale) out["concentration_scale"] = toString(*c.concentration_scale);
    if (c.water_reference) out["water_reference"] = toString(*c.water_reference);
    return out;
}

// the unknown fields of a record, after the canonical ones; a canonical key wins
void addExtra(json& out, const std::string& extra)
{
    if (extra.empty())
        return;
    const json e = json::parse(extra, nullptr, false);
    if (e.is_discarded() || !e.is_object())
        return;
    for (const auto& item : e.items())
        if (!out.contains(item.key()))
            out[item.key()] = item.value();
}

auto referenceJson(const ReferenceConditions& rc) -> json
{
    return json{{"T", rc.T}, {"P", rc.P}};
}

auto covariancesJson(const std::vector<CovarianceBlock>& blocks) -> json
{
    json out = json::array();
    for (const auto& b : blocks)
    {
        json matrix = json::array();
        const size_t n = b.paths.size();
        for (size_t i = 0; i < n; ++i)
            matrix.push_back(std::vector<double>(b.matrix.begin() + i * n, b.matrix.begin() + (i + 1) * n));
        out.push_back(json{{"paths", b.paths}, {"matrix", matrix}});
    }
    return out;
}

auto structuralJson(const std::map<std::string, Quantity>& structural, const V2WriterOptions& options) -> json
{
    json out = json::object();
    for (const auto& item : structural)
        out[item.first] = quantityJson(item.second, "1", {}, options);
    return out;
}

// the molar mass: a bare number is in g/mol
auto molarMassJson(const Quantity& q, const V2WriterOptions& options) -> json
{
    const std::string unit = q.unit_as_written.empty() || sameUnit(q.unit_as_written, "g/mol") ? "g/mol" : q.unit_as_written;
    return quantityJson(q, "kg/mol", {unit, unit == "g/mol"}, options);
}

auto elementJson(const ElementRecord& r, const V2WriterOptions& options) -> json
{
    json out = json::object();
    if (!r.name.empty()) out["name"] = r.name;
    if (r.class_ != ElementClass::Element) out["class"] = toString(r.class_);
    if (r.legacy_class) out["legacy_class"] = *r.legacy_class;
    if (r.number) out["number"] = *r.number;
    if (r.isotope_mass) out["isotope_mass"] = *r.isotope_mass;
    if (r.valence) out["valence"] = *r.valence;
    if (r.molar_mass) out["molar_mass"] = molarMassJson(*r.molar_mass, options);
    if (r.standard_entropy) out["standard_entropy"] = simpleQuantity(*r.standard_entropy, "J/(mol.K)", "", options);
    if (r.heat_capacity) out["heat_capacity"] = simpleQuantity(*r.heat_capacity, "J/(mol.K)", "", options);
    if (r.volume) out["volume"] = simpleQuantity(*r.volume, "m3/mol", r.source_format == DataFormat::Legacy ? "J/bar" : "", options);
    if (!r.reference_form.empty()) out["reference_form"] = r.reference_form;
    if (const auto sources = sourcesOf(r.data_sources); !sources.empty()) out["data_sources"] = sources;
    if (!r.sourcetdb.empty()) out["sourcetdb"] = r.sourcetdb;
    if (!r.structural.empty()) out["structural"] = structuralJson(r.structural, options);
    addExtra(out, r.extra_json);
    return out;
}

auto substanceJson(const SubstanceRecord& r, const ThermoDataSetInfo& dataset, const V2WriterOptions& options) -> json
{
    json out = json::object();
    if (!r.name.empty()) out["name"] = r.name;
    if (!r.formula.empty()) out["formula"] = r.formula;
    if (r.charge) out["charge"] = *r.charge;
    out["class"] = toString(r.class_);
    out["aggregate_state"] = toString(r.aggregate_state);
    if (!r.sourcetdb.empty()) out["sourcetdb"] = r.sourcetdb;
    if (r.molar_mass) out["molar_mass"] = molarMassJson(*r.molar_mass, options);
    if (!r.standard_state_convention.empty()) out["standard_state_convention"] = r.standard_state_convention;
    if (r.convention) out["convention"] = conventionJson(*r.convention);
    if (!(r.reference_conditions == dataset.reference_conditions)) out["reference_conditions"] = referenceJson(r.reference_conditions);
    if (!r.standard_state_properties.empty())
    {
        json props = json::object();
        for (const PropertyName name : {PropertyName::G, PropertyName::H, PropertyName::S, PropertyName::Cp, PropertyName::V, PropertyName::U, PropertyName::A, PropertyName::Cv})
            if (const auto it = r.standard_state_properties.find(name); it != r.standard_state_properties.end())
                props[toString(name)] = simpleQuantity(it->second, siUnit(name), name == PropertyName::V && r.source_format == DataFormat::Legacy ? "J/bar" : "", options);
        out["standard_state_properties"] = props;
    }
    if (!r.methods.empty()) out["methods"] = methodsJson(r.methods, OperatesOn::Substance, r.source_format, options);
    if (!r.defining_reaction.empty()) out["defining_reaction"] = r.defining_reaction;
    if (r.preferred_property_source) out["preferred_property_source"] = toString(*r.preferred_property_source);
    const json a = applicabilityJson(r.applicability, options);
    if (!a.empty()) out["applicability"] = a;
    if (!r.data_source.empty()) out["data_source"] = r.data_source;
    if (const auto sources = sourcesOf(r.data_sources); !sources.empty()) out["data_sources"] = sources;
    if (!r.structural.empty()) out["structural"] = structuralJson(r.structural, options);
    if (!r.covariance.empty()) out["covariances"] = covariancesJson(r.covariance);
    if (r.deprecated) out["deprecated"] = true;
    if (!r.deprecated_in_favour_of.empty()) out["deprecated_in_favour_of"] = r.deprecated_in_favour_of;
    addExtra(out, r.extra_json);
    return out;
}

auto reactionJson(const ReactionRecord& r, const ThermoDataSetInfo& dataset, const V2WriterOptions& options) -> json
{
    json out = json::object();
    if (!r.name.empty()) out["name"] = r.name;
    if (!r.sourcetdb.empty()) out["sourcetdb"] = r.sourcetdb;
    // the equation, if it says what the stoichiometry says: the equation of a legacy record can name the formulas
    std::map<std::string, double> written(r.stoichiometry.begin(), r.stoichiometry.end());
    bool agree = !r.equation.empty();
    if (agree)
    {
        const auto parsed = stoichiometryOfEquation(r.equation);
        agree = parsed.size() == written.size();
        for (const auto& item : parsed)
            agree = agree && written.count(item.first) && std::fabs(written[item.first] - item.second) < 1e-12;
    }
    if (agree)
        out["equation"] = r.equation;
    else if (!r.stoichiometry.empty())
    {
        json s = json::object();
        for (const auto& item : r.stoichiometry)
            s[item.first] = item.second;
        out["stoichiometry"] = s;
    }
    if (!(r.reference_conditions == dataset.reference_conditions)) out["reference_conditions"] = referenceJson(r.reference_conditions);
    if (!r.reaction_properties.empty())
    {
        json props = json::object();
        for (const ReactionPropertyName name : {ReactionPropertyName::logK, ReactionPropertyName::lnK, ReactionPropertyName::dG, ReactionPropertyName::dH, ReactionPropertyName::dS,
                                                ReactionPropertyName::dCp, ReactionPropertyName::dV, ReactionPropertyName::dU, ReactionPropertyName::dA})
            if (const auto it = r.reaction_properties.find(name); it != r.reaction_properties.end())
                props[toString(name)] = simpleQuantity(it->second, siUnit(name), name == ReactionPropertyName::dV && r.source_format == DataFormat::Legacy ? "J/bar" : "", options);
        out["reaction_properties"] = props;
    }
    if (!r.methods.empty()) out["methods"] = methodsJson(r.methods, OperatesOn::Reaction, r.source_format, options);
    if (r.level != 0) out["level"] = r.level;
    const json a = applicabilityJson(r.applicability, options);
    if (!a.empty()) out["applicability"] = a;
    if (r.convention) out["convention"] = conventionJson(*r.convention);
    if (r.preferred_property_source) out["preferred_property_source"] = toString(*r.preferred_property_source);
    if (!r.data_source.empty()) out["data_source"] = r.data_source;
    if (const auto sources = sourcesOf(r.data_sources); !sources.empty()) out["data_sources"] = sources;
    if (!r.covariance.empty()) out["covariances"] = covariancesJson(r.covariance);
    if (r.deprecated) out["deprecated"] = true;
    addExtra(out, r.extra_json);
    return out;
}

auto dataSourceJson(const DataSource& d) -> json
{
    json out = json::object();
    if (!d.title.empty()) out["title"] = d.title;
    if (!d.authors.empty()) out["authors"] = d.authors;
    if (d.year) out["year"] = *d.year;
    if (!d.journal.empty()) out["journal"] = d.journal;
    if (!d.doi.empty()) out["doi"] = d.doi;
    if (!d.bibtex.empty()) out["bibtex"] = d.bibtex;
    addExtra(out, d.extra_json);
    return out;
}

auto datasetJson(const ThermoDataSetInfo& d) -> json
{
    const ThermoDataSetInfo defaults;
    json out = json::object();
    if (!d.symbol.empty()) out["symbol"] = d.symbol;
    if (!d.name.empty()) out["name"] = d.name;
    if (!d.version.empty()) out["version"] = d.version;
    if (d.status != defaults.status) out["status"] = toString(d.status);
    if (d.standard_state_convention != defaults.standard_state_convention) out["standard_state_convention"] = d.standard_state_convention;
    if (!(d.reference_conditions == defaults.reference_conditions)) out["reference_conditions"] = referenceJson(d.reference_conditions);
    if (!d.sourcetdb.empty()) out["sourcetdb"] = d.sourcetdb;
    if (!d.solvent.empty()) out["solvent"] = d.solvent;
    if (!d.default_data_source.empty()) out["data_source"] = d.default_data_source;
    json values = json::object();
    if (d.defaults.quality) values["quality"] = toString(*d.defaults.quality);
    if (d.defaults.origin) values["origin"] = toString(*d.defaults.origin);
    if (!values.empty()) out["defaults"] = values;
    out["conventions"] = json{{"water_triple_point_reference", d.water_triple_point_reference}};
    const json c = conventionJson(d.convention);
    if (!c.empty()) out["convention"] = c;
    addExtra(out, d.extra_json);
    return out;
}

auto documentJson(const V2Records& r, const V2WriterOptions& options) -> json
{
    json root = json::object();
    root["thermohub"] = formatVersion;
    root["generator"] = "ThermoFun";
    root["thermodataset"] = datasetJson(r.dataset);
    auto section = [&](const char* key, const auto& records, auto write) {
        if (records.empty())
            return;
        json out = json::object();
        for (const auto& record : records)
            out[record.symbol] = write(record);
        root[key] = out;
    };
    section("elements", r.elements, [&](const ElementRecord& x) { return elementJson(x, options); });
    section("substances", r.substances, [&](const SubstanceRecord& x) { return substanceJson(x, r.dataset, options); });
    section("reactions", r.reactions, [&](const ReactionRecord& x) { return reactionJson(x, r.dataset, options); });
    section("methods", r.methods, [&](const MethodDefinition& x) { return methodRecordJson(x); });
    section("parameter_sets", r.parameter_sets, [&](const MethodParameterSet& x) { return parameterSetJson(x, r.methods, options); });
    section("data_sources", r.data_sources, [&](const DataSource& x) { return dataSourceJson(x); });
    return root;
}

} // namespace ThermoFun::v2::v2writer

namespace ThermoFun::v2 {

using namespace v2writer;

auto writeV2Element(const ElementRecord& record, const V2WriterOptions& options) -> std::string
{
    json out = json::object();
    out["symbol"] = record.symbol;
    const json fields = elementJson(record, options);
    for (const auto& item : fields.items())
        out[item.key()] = item.value();
    return out.dump();
}

auto writeV2Substance(const SubstanceRecord& record, const V2WriterOptions& options) -> std::string
{
    json out = json::object();
    out["symbol"] = record.symbol;
    const json fields = substanceJson(record, ThermoDataSetInfo{}, options);
    for (const auto& item : fields.items())
        out[item.key()] = item.value();
    return out.dump();
}

auto writeV2Reaction(const ReactionRecord& record, const V2WriterOptions& options) -> std::string
{
    json out = json::object();
    out["symbol"] = record.symbol;
    const json fields = reactionJson(record, ThermoDataSetInfo{}, options);
    for (const auto& item : fields.items())
        out[item.key()] = item.value();
    return out.dump();
}

auto writeV2Json(const V2Records& records, const V2WriterOptions& options) -> std::string
{
    return documentJson(records, options).dump(1);
}

auto writeV2Yaml(const V2Records& records, const V2WriterOptions& options) -> std::string
{
    std::string out;
    block(documentJson(records, options), 0, out);
    return out;
}

} // namespace ThermoFun::v2
