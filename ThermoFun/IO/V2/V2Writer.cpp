// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "V2Writer.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <optional>
#include <regex>
#include <set>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Numbers.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/Equation.h>
#include <ThermoFun/IO/V2/detail/Normalize.h>
#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::v2 {

namespace {

using json = nlohmann::ordered_json;

constexpr const char* formatVersion = "2.0.0-draft.1";

// J/bar on a molar volume is for one mole, as in the reader
auto molar(const std::string& unit, const std::string& slot) -> std::string
{
    return unit == "J/bar" && slot == "m3/mol" ? "J/(mol.bar)" : unit;
}

// True if the two units are the same unit, in any spelling
auto sameUnit(const std::string& a, const std::string& b) -> bool
{
    if (a == b)
        return true;
    try
    {
        return units::dimension(a) == units::dimension(b) && units::convertAny(1.0, a, b) == 1.0 && units::convertAny(0.0, a, b) == 0.0;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

// True if a value of the slot can be written in the unit
auto usable(const std::string& unit, const std::string& slot) -> bool
{
    try
    {
        return units::dimension(molar(unit, slot)) == units::dimension(slot);
    }
    catch (const std::exception&)
    {
        return false;
    }
}

// The shortest decimal that reads back as the same float
auto shortestFloat(double x) -> double
{
    const auto f = static_cast<float>(x);
    if (!std::isfinite(f))
        return x;
    char text[64];
    const auto r = std::to_chars(text, text + sizeof(text), f);
    return numbers::parseDouble(std::string_view(text, static_cast<size_t>(r.ptr - text)));
}

// A limit of T or P has no flag of its source: a value that is a float printed with 13 or 14 digits is one, for example 700.00000610352
auto limit(double x, const V2WriterOptions& options) -> double
{
    const auto f = static_cast<float>(x);
    const bool nearFloat = std::isfinite(f) && std::fabs(x - static_cast<double>(f)) <= 2e-12 * std::fabs(x);
    return options.readable && nearFloat ? shortestFloat(x) : x;
}

// The shortest number near `start` that the reader turns into `si`: the output depends on the value, not on how the file wrote it
template <class ReadBack>
auto shortestThatReadsBack(double si, double start, ReadBack read) -> std::optional<double>
{
    std::optional<double> best;
    size_t bestLength = 0;
    auto consider = [&](double x) {
        if (read(x) != si)
            return;
        const size_t length = nlohmann::json(x).dump().size();
        if (!best || length < bestLength)
        {
            best = x;
            bestLength = length;
        }
    };
    double up = start, down = start;
    consider(start);
    for (int i = 0; i < 8; ++i)
    {
        consider(up = std::nextafter(up, HUGE_VAL));
        consider(down = std::nextafter(down, -HUGE_VAL));
    }
    return best;
}

// The number to write in `unit` for the SI value, so that the reader gets the same double, if a number near it does that
auto numberIn(double si, const std::string& unit, const std::string& slot) -> std::optional<double>
{
    if (unit.empty() || unit == slot)
        return si;
    const std::string from = molar(unit, slot);
    return shortestThatReadsBack(si, units::convertAny(si, slot, from), [&](double x) { return units::convertAny(x, from, slot); });
}

// The same for an uncertainty; if no number reads back the same, the nearest one
auto differenceIn(double si, const std::string& from, const std::string& slot) -> double
{
    if (from == slot)
        return si;
    const double start = units::convertDifference(si, slot, from);
    return shortestThatReadsBack(si, start, [&](double x) { return units::convertDifference(x, from, slot); }).value_or(start);
}

struct Written
{
    std::string unit;        ///< the unit of the numbers; empty: the unit of the slot
    bool from_scope = false; ///< the reader gets the unit from a scope or a default, so it is not written with the value
};

// A quantity in a unit: a bare number if it has only a value and the reader knows the unit
auto quantityJson(const Quantity& q, const std::string& slot, const Written& w, const V2WriterOptions& options) -> json
{
    std::string unit = w.unit;
    bool fromScope = w.from_scope;
    double number = q.value;
    if (!unit.empty())
    {
        // in a readable file the float decides, so the double near it is enough
        const auto found = options.readable && q.single_precision_source ? std::optional<double>(units::convertAny(q.value, slot, molar(unit, slot)))
                                                                         : numberIn(q.value, unit, slot);
        if (found)
            number = *found;
        else
        {
            // no number in that unit reads back the same: the slot unit, written out
            unit = slot;
            fromScope = false;
        }
    }
    const bool shorten = options.readable && q.single_precision_source;
    if (shorten)
        number = shortestFloat(number);

    // a legacy file gives 0 where it knows nothing: missing is not zero
    const bool noUncertainty = !q.uncertainty || (q.uncertainty->standard == 0.0 && q.uncertainty->convention_unknown);
    const auto& u = q.uncertainty;
    const bool hasMeta = q.meta.quality || q.meta.origin || !q.meta.calculation_path.empty() || !q.meta.source.empty();
    if (noUncertainty && !hasMeta && (unit.empty() || fromScope))
        return number;

    json out = json::object();
    out["value"] = number;
    if (!noUncertainty)
    {
        const std::string from = molar(unit.empty() ? slot : unit, slot);
        double standard = differenceIn(u->standard, from, slot);
        if (shorten)
            standard = shortestFloat(standard);
        const bool simple = u->coverage_factor_as_written == 1.0 && u->kind == Uncertainty::Kind::Primary && u->distribution.empty() && !u->degrees_of_freedom;
        if (simple)
            out["uncertainty"] = standard;
        else
        {
            json one = json::object();
            if (u->coverage_factor_as_written != 1.0)
            {
                one["expanded"] = standard * u->coverage_factor_as_written;
                one["coverage_factor"] = u->coverage_factor_as_written;
            }
            else
                one["standard"] = standard;
            if (!u->distribution.empty())
                one["distribution"] = u->distribution;
            if (u->degrees_of_freedom)
                one["degrees_of_freedom"] = *u->degrees_of_freedom;
            if (u->kind == Uncertainty::Kind::Derived)
                one["kind"] = "derived";
            out["uncertainty"] = one;
        }
    }
    if (!unit.empty() && !fromScope)
        out["unit"] = unit;
    if (q.meta.quality)
        out["quality"] = toString(*q.meta.quality);
    if (q.meta.origin)
        out["origin"] = toString(*q.meta.origin);
    if (!q.meta.calculation_path.empty())
        out["calculation_path"] = q.meta.calculation_path;
    if (!q.meta.source.empty())
        out["source"] = q.meta.source;
    return out;
}

// The unit of a value of a record: as written, without a unit that is the slot unit in another spelling or that cannot hold the value
auto writtenUnit(const Quantity& q, const std::string& slot, const std::string& source_unit) -> std::string
{
    // a label of a file that cannot hold the value, for example a swapped one, says nothing
    const std::string& unit = !q.unit_as_written.empty() && usable(q.unit_as_written, slot) ? q.unit_as_written : source_unit;
    if (unit.empty() || !usable(unit, slot) || sameUnit(molar(unit, slot), slot))
        return "";
    return unit;
}

auto simpleQuantity(const Quantity& q, const std::string& slot, const std::string& source_unit, const V2WriterOptions& options) -> json
{
    return quantityJson(q, slot, {writtenUnit(q, slot, source_unit), false}, options);
}

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

// ---- YAML ----

// A word that YAML reads as the same text without quotes
auto plainWord(const std::string& s) -> bool
{
    static const std::regex safe("^[A-Za-z0-9_(][A-Za-z0-9_+\\-./()@|^*]*( [A-Za-z0-9_+\\-./()@|^*]+)*$");
    if (!std::regex_match(s, safe))
        return false;
    static const std::regex number("^[0-9.]+([eE][+-]?[0-9]+)?$");
    if (std::regex_match(s, number))
        return false;
    static const std::set<std::string> words = {"true", "True", "TRUE", "false", "False", "FALSE", "null", "Null", "NULL", "yes", "no", "on", "off", "Yes", "No", "On", "Off"};
    return !words.count(s);
}

auto text(const std::string& s) -> std::string
{
    return plainWord(s) ? s : json(s).dump();
}

auto isBlock(const json& node) -> bool
{
    return (node.is_object() || node.is_array()) && !node.empty();
}

// The node on one line
auto flow(const json& node) -> std::string
{
    if (node.is_string())
        return text(node.get<std::string>());
    if (node.is_object())
    {
        std::string out = "{";
        bool first = true;
        for (const auto& item : node.items())
        {
            out += (first ? "" : ", ") + text(item.key()) + ": " + flow(item.value());
            first = false;
        }
        return out + "}";
    }
    if (node.is_array())
    {
        std::string out = "[";
        bool first = true;
        for (const auto& item : node)
        {
            out += (first ? "" : ", ") + flow(item);
            first = false;
        }
        return out + "]";
    }
    return node.dump();
}

auto onlyNumbers(const json& node) -> bool
{
    return node.is_array() && std::all_of(node.begin(), node.end(), [](const json& x) { return x.is_number(); });
}

// a mapping or list goes on one line if it is short, and a list of numbers always does
auto staysOnOneLine(const json& node, size_t room) -> bool
{
    return !isBlock(node) || onlyNumbers(node) || flow(node).size() <= room;
}

void block(const json& node, size_t indent, std::string& out)
{
    constexpr size_t width = 110;
    const std::string pad(indent, ' ');
    if (node.is_object())
        for (const auto& item : node.items())
        {
            const std::string head = pad + text(item.key()) + ":";
            const json& value = item.value();
            if (staysOnOneLine(value, width > head.size() + 1 ? width - head.size() - 1 : 0))
                out += head + " " + flow(value) + "\n";
            else
            {
                out += head + "\n";
                block(value, indent + 2, out);
            }
        }
    else
        for (const auto& value : node)
        {
            if (staysOnOneLine(value, width > indent + 2 ? width - indent - 2 : 0))
            {
                out += pad + "- " + flow(value) + "\n";
                continue;
            }
            // the first line of the block follows the dash
            std::string inner;
            block(value, indent + 2, inner);
            out += pad + "- " + inner.substr(indent + 2);
        }
}

} // namespace

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
