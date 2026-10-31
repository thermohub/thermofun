// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyReader.h"

#include <algorithm>
#include <regex>
#include <cctype>
#include <cmath>
#include <exception>
#include <limits>
#include <set>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/Methods/MethodCatalog.h>

#include <ThermoFun/IO/Equation.h>

#include "LegacyKeys.h"
#include "LegacyMethodMap.h"
#include "LegacyQuirks.h"
#include "LegacyVocabulary.h"

namespace ThermoFun::v2 {

namespace {

using json = nlohmann::json;
constexpr double inf = std::numeric_limits<double>::infinity();

// The number of coefficients of one array that the legacy parser accepts
constexpr size_t maxCoefficients = 1000;

// GEMS writes the smallest float32 for a value that is empty
constexpr double emptyMarker = 1e-30;

struct Context
{
    const LegacyReadOptions& options;
    Diagnostics* diagnostics;
    std::string record;  // symbol of the record, for the diagnostics
};

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

legacy::Where where(const Context& c, const std::string& path = "")
{
    return {c.record, path};
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

// ---------------------------------------------------------------------------------------------------------------
// Coefficient arrays

auto numbersOf(const json& entry, const char* field) -> std::vector<std::optional<double>>
{
    std::vector<std::optional<double>> out;
    if (entry.contains(field) && entry[field].is_array())
        for (const auto& v : entry[field])
            out.push_back(v.is_number() ? std::optional<double>(v.get<double>()) : std::nullopt);
    return out;
}

// The unit of the i-th value of an array: the unit of the file, or the unit of the model if the file gives none
auto unitOfValue(size_t i, const std::vector<std::string>& from, const std::string& model) -> std::string
{
    std::string unit = i < from.size() ? from[i] : "";
    if (unit.empty())
        unit = model;
    return unit;
}

auto ownsKey(const legacy::LegacyMethod& method, const std::string& key) -> bool
{
    for (const auto& p : method.parameters)
        if (key == p.key)
            return true;
    return false;
}

// The SI unit of a parameter of a method
auto siUnitOf(const MethodSpec& spec, const std::string& name) -> std::string
{
    for (const auto& p : spec.parameters)
        if (p.name == name)
            return p.si_unit;
    return "";
}

// A legacy volume in J/bar is per mole
auto perMole(const std::string& unit) -> std::string
{
    return unit == "J/bar" ? "J/(mol*bar)" : unit;
}

// Reads the array of a key into the parameters of a method, in SI units. Values that no parameter takes are reported.
void readArray(const json& entry, const std::string& key, const legacy::LegacyMethod& method, const MethodSpec& spec,
               std::map<std::string, Quantity>& out, const Context& c)
{
    if (!entry.contains(key) || !entry[key].is_object())
        return;
    const auto& array = entry[key];
    const auto values = numbersOf(array, "values");
    const auto errors = numbersOf(array, "errors");
    if (values.size() > maxCoefficients)
    {
        report(c, Severity::Error, "LEG-QRK-009", key, "too many coefficients: " + std::to_string(values.size()));
        return;
    }

    std::vector<std::string> from;
    if (array.contains("units") && array["units"].is_array())
        for (const auto& u : array["units"])
            from.push_back(u.is_string() ? u.get<std::string>() : "");

    bool unitsShort = !from.empty() && from.size() < values.size();
    if (legacy::hkfUnitsAreWrong(key, from, c.diagnostics, where(c, key)))
    {
        from.clear();  // the units of the method are used
        unitsShort = false;
    }
    for (const auto& u : from)
        unitsShort = unitsShort || u.empty();
    if (unitsShort)
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-011", where(c, key), "the list of units is shorter than the list of values, or it has empty units");

    size_t used = 0;
    for (const auto& p : method.parameters)
    {
        if (key != p.key)
            continue;
        const size_t i = static_cast<size_t>(p.index);
        used = std::max(used, i + 1);
        if (i >= values.size() || !values[i])
            continue;
        const std::string si = siUnitOf(spec, p.name);
        const std::string unit = perMole(unitOfValue(i, from, p.unit));
        Quantity q;
        q.single_precision_source = isSinglePrecision(*values[i]);
        try
        {
            q.value = units::convertAny(*values[i], unit, si);
            if (i < from.size() && !from[i].empty())
                q.unit_as_written = from[i];
            if (i < errors.size() && errors[i] && *errors[i] > 0.0)
            {
                Uncertainty u;
                u.standard = std::fabs(units::convertDifference(*errors[i], unit, si)) / c.options.error_coverage_factor;
                u.coverage_factor_as_written = c.options.error_coverage_factor;
                u.convention_unknown = true;
                q.uncertainty = u;
            }
        }
        catch (const std::exception& e)
        {
            report(c, Severity::Error, "V2-UNIT-001", key, e.what());
            continue;
        }
        out[p.name] = q;
    }

    if (values.size() > used && used > 0)
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-010", where(c, key),
                            "the array has " + std::to_string(values.size()) + " values, the method uses " + std::to_string(used));
}

// {lowerT, upperT, lowerP, upperP} of a limitsTP object; a missing bound is no limit
auto readLimits(const json& entry) -> Applicability
{
    Applicability a;
    if (!isPresent(entry, legacy::LimitsTP) || !entry[legacy::LimitsTP].is_object())
        return a;
    const auto& l = entry[legacy::LimitsTP];
    auto number = [&](const char* key) -> std::optional<double> {
        return isPresent(l, key) && l[key].is_number() ? std::optional<double>(l[key].get<double>()) : std::nullopt;
    };
    a.t_min = number("lowerT");
    a.t_max = number("upperT");
    a.p_min = number("lowerP");
    a.p_max = number("upperP");
    return a;
}

// The T limits of a segment: a limit that the file does not give is infinite
auto segmentOf(const json& entry) -> Segment
{
    const Applicability a = readLimits(entry);
    Segment s;
    s.t_min = a.t_min.value_or(-inf);
    s.t_max = a.t_max.value_or(inf);
    return s;
}

// A row of m_phase_trans_props: T, dS, dH, dV, dTdP
auto readTransition(const json& entry, const Context& c) -> std::map<std::string, Quantity>
{
    std::map<std::string, Quantity> row;
    static const char* names[] = {"T", "dS", "dH", "dV", "dTdP"};
    static const char* modelUnits[] = {"K", "J/(mol*K)", "J/mol", "J/bar", "K/bar"};
    static const char* si[] = {"K", "J/(mol.K)", "J/mol", "m3", "K/Pa"};
    const auto& array = entry[legacy::MPhaseTransProps];
    const auto values = numbersOf(array, "values");
    std::vector<std::string> from;
    if (array.contains("units") && array["units"].is_array())
        for (const auto& u : array["units"])
            from.push_back(u.is_string() ? u.get<std::string>() : "");
    for (size_t i = 0; i < 5 && i < values.size(); ++i)
    {
        if (!values[i])
            continue;
        try
        {
            Quantity q;
            q.value = units::convertAny(*values[i], unitOfValue(i, from, modelUnits[i]), si[i]);
            row[names[i]] = q;
        }
        catch (const std::exception& e)
        {
            report(c, Severity::Error, "V2-UNIT-001", legacy::MPhaseTransProps, e.what());
        }
    }
    return row;
}

auto readTable(const json& table) -> TableData
{
    TableData t;
    auto vec = [&](const char* key) {
        std::vector<double> v;
        if (table.contains(key) && table[key].is_array())
            v = table[key].get<std::vector<double>>();
        return v;
    };
    t.temperatures = vec("temperatures");
    t.pressures = vec("pressures");
    static const std::pair<const char*, const char*> names[] = {
        {"gibbs_energy", "G"}, {"enthalpy", "H"}, {"volume", "V"}, {"heat_capacity_cp", "Cp"}};
    for (const auto& n : names)
    {
        auto v = vec(n.first);
        if (!v.empty())
            t.values[n.second] = v;
    }
    if (table.contains("reference_pressure") && table["reference_pressure"].is_number())
        t.reference_pressure = table["reference_pressure"].get<double>();
    return t;
}

// ---------------------------------------------------------------------------------------------------------------
// TPMethods

struct EntryInfo
{
    const json* entry;
    const legacy::LegacyMethod* method;
    const MethodSpec* spec;
    size_t application;         // index in the list of applications
};

// Reads the keys of an entry that belong to its method into the application
void readEntryKeys(const json& entry, const legacy::LegacyMethod& method, const MethodSpec& spec, MethodApplication& app, const Context& c)
{
    std::map<std::string, Quantity> parameters;
    std::set<std::string> read;  // each key once
    for (const auto& p : method.parameters)
        if (entry.contains(p.key) && read.insert(p.key).second)
            readArray(entry, p.key, method, spec, parameters, c);

    if (spec.piecewise)
    {
        Segment s = segmentOf(entry);
        s.parameters = parameters;
        app.segments.push_back(s);
        if (entry.contains(legacy::MPhaseTransProps) && entry[legacy::MPhaseTransProps].is_object())
            app.transitions.push_back(readTransition(entry, c));
    }
    else
    {
        for (auto& p : parameters)
            app.parameters[p.first] = p.second;
        app.applicability = readLimits(entry);
        if (entry.contains(legacy::TpTable) && entry[legacy::TpTable].is_object())
            app.table = readTable(entry[legacy::TpTable]);
    }
}

// What the entries of a record say besides its methods
struct MethodFlags
{
    bool gas_critical = false;   // fug_critical_param: a flag, not a method
};

auto readMethods(const json& j, OperatesOn operatesOn, const Context& c, MethodFlags& flags) -> std::vector<MethodApplication>
{
    std::vector<MethodApplication> apps;
    if (!isPresent(j, legacy::TPMethods) || !j[legacy::TPMethods].is_array())
        return apps;

    const MethodCatalog& catalog = MethodCatalog::instance();
    std::vector<EntryInfo> infos;

    for (const auto& entry : j[legacy::TPMethods])
    {
        if (!entry.is_object() || !entry.contains(legacy::Method) || entry[legacy::Method].is_null())
            continue;
        const auto& m = entry[legacy::Method];
        if (!(m.is_object() || m.is_array()) || m.empty() || m.begin()->is_null())
            continue;

        int id = -1;
        std::string name;
        try
        {
            id = std::stoi(m.begin().key());
            name = m.begin().value().is_string() ? m.begin().value().get<std::string>() : "";
        }
        catch (const std::exception&)
        {
            report(c, Severity::Error, "LEG-MTH-001", legacy::TPMethods, "the method id is not a number");
            continue;
        }
        const std::string trimmed = legacy::trimmedMethodName(name, c.diagnostics, where(c, legacy::TPMethods));
        const auto* method = operatesOn == OperatesOn::Substance ? legacy::legacySubstanceMethod(id) : legacy::legacyReactionMethod(id);
        if (!method)
        {
            report(c, Severity::Error, "LEG-MTH-001", legacy::TPMethods, "the legacy method id " + std::to_string(id) + " is not known");
            continue;
        }
        legacy::methodNameDisagrees(trimmed, method->name, c.diagnostics, where(c, legacy::TPMethods));

        // not a method: a flag of the record
        if (std::string(method->name) == "fug_critical_param")
        {
            flags.gas_critical = true;
            continue;
        }
        const MethodSpec* spec = *method->method ? catalog.find(method->method, operatesOn) : nullptr;
        if (!spec || !spec->implemented)
        {
            report(c, Severity::Warning, "LEG-MTH-002", legacy::TPMethods, "the method " + trimmed + " is not implemented, so it is not used");
            continue;
        }
        if (std::string(method->name) == "water_eos_iapws95_gems")
            report(c, Severity::Warning, "LEG-MTH-003", legacy::TPMethods, "water_eos_iapws95_gems is read as WaterIAPWS95 of Reaktoro");

        // a piecewise method is one application with a segment for each entry
        size_t index = apps.size();
        if (spec->piecewise)
            for (size_t i = 0; i < apps.size(); ++i)
                if (apps[i].method == spec->id)
                    index = i;
        if (index == apps.size())
        {
            MethodApplication app;
            app.method = spec->id;
            app.method_as_written = name;
            app.implementation = method->implementation;
            apps.push_back(app);
        }
        readEntryKeys(entry, *method, *spec, apps[index], c);
        infos.push_back({&entry, method, spec, index});
    }

    // keys of an entry that belong to another method of the record go to that method; others are reported
    for (const auto& info : infos)
        for (const auto& key : legacy::legacyCoefficientKeys())
        {
            if (!info.entry->contains(key) || ownsKey(*info.method, key))
                continue;
            bool placed = false;
            for (const auto& other : infos)
                if (other.application != info.application && ownsKey(*other.method, key))
                {
                    json entryOnly = json::object();
                    entryOnly[key] = (*info.entry)[key];
                    MethodApplication& target = apps[other.application];
                    std::map<std::string, Quantity> parameters;
                    readArray(entryOnly, key, *other.method, *other.spec, parameters, c);
                    if (other.spec->piecewise && !target.segments.empty())
                        for (auto& p : parameters)
                            target.segments.back().parameters.emplace(p.first, p.second);
                    else
                        for (auto& p : parameters)
                            target.parameters.emplace(p.first, p.second);
                    placed = true;
                    break;
                }
            if (!placed)
                report(c, Severity::Warning, "LEG-KEY-002", key, "the key " + key + " does not belong to the method " + info.spec->id + "; it is not used");
        }
    return apps;
}

auto findMethod(std::vector<MethodApplication>& apps, const char* id) -> MethodApplication*
{
    for (auto& a : apps)
        if (a.method == id)
            return &a;
    return nullptr;
}

// A record has one base method. The legacy logk_fpt_function and dr_heat_capacity_ft only hold coefficients for the
// log K method beside them, so their coefficients move to that method.
void mergeLogKMethods(std::vector<MethodApplication>& apps)
{
    MethodApplication* target = nullptr;
    for (const char* id : {"LogKNordstromMunoz88", "LogKConstantCp", "LogKVantHoff", "LogKZeroEntropy", "LogKConstant", "LogKPolynomial"})
        if (!target)
            target = findMethod(apps, id);
    if (!target)
        return;
    const std::string kept = target->method;
    for (const char* id : {"LogKPolynomial", "ReactionCpPolynomial"})
    {
        const auto* from = findMethod(apps, id);
        if (from && from->method != kept)
            target->parameters.insert(from->parameters.begin(), from->parameters.end());
    }
    apps.erase(std::remove_if(apps.begin(), apps.end(), [&](const MethodApplication& a) {
                   return a.method != kept && (a.method == "LogKPolynomial" || a.method == "ReactionCpPolynomial");
               }),
               apps.end());

    // the same beside a method with parameters of its own: only the coefficients of log K, a0 to a6, have other names
    auto* polynomial = findMethod(apps, "LogKPolynomial");
    MethodApplication* other = nullptr;
    for (const char* id : {"LogKTable", "MarshallFranck81", "RyzhenkoBryzgalin", "DolejsManning10"})
        if (!other)
            other = findMethod(apps, id);
    if (polynomial && other)
    {
        for (const auto& p : polynomial->parameters)
            if (p.first.size() == 2 && p.first[0] == 'a' && std::isdigit(static_cast<unsigned char>(p.first[1])))
                other->parameters.insert(p);
        apps.erase(apps.begin() + (polynomial - apps.data()));
    }
}

// Water vapour: HKF with the flag fug_critical_param, or with a water equation of state on a record that is not an
// aqueous solute, is water as a gas; so is a water equation of state on a record in the gas state.
void applyWaterVapourRule(SubstanceRecord& s, const MethodFlags& flags)
{
    auto isWaterEoS = [](const MethodApplication& a) { return a.method == "WaterHGK84" || a.method == "WaterIAPWS95" || a.method == "WaterZhangDuan05"; };
    auto* hkf = findMethod(s.methods, "HKF88");
    MethodApplication* water = nullptr;
    for (auto& a : s.methods)
        if (isWaterEoS(a))
            water = &a;

    const bool vapourByHkf = hkf && (flags.gas_critical || (water && s.class_ != SubstanceClass::AqueousSolute));
    if (vapourByHkf && !water)
    {
        hkf->method = "WaterHGK84";
        hkf->parameters.clear();
        water = hkf;
    }
    else if (vapourByHkf)
    {
        // the name of HKF stays on the water method: it says that this record is vapour
        water->method_as_written = hkf->method_as_written;
        s.methods.erase(s.methods.begin() + (hkf - s.methods.data()));
        water = nullptr;
    }

    for (auto& a : s.methods)
        if (isWaterEoS(a) && (vapourByHkf || s.aggregate_state == AggregateState::Gas))
        {
            Quantity state;
            state.value = 1.0;
            a.parameters["state"] = state;
        }
}

// alpha0 and kappa0 of the record-level keys m_expansivity and m_compressibility
void readHp98Values(const json& j, SubstanceRecord& s, const Context& c)
{
    std::optional<Quantity> alpha, kappa;

    if (isPresent(j, legacy::MExpansivity) && j[legacy::MExpansivity].is_object())
    {
        const auto& e = j[legacy::MExpansivity];
        const auto values = numbersOf(e, "values");
        std::string unit = "1/K";
        if (e.contains("units") && e["units"].is_array() && !e["units"].empty() && e["units"][0].is_string())
            unit = e["units"][0].get<std::string>();
        if (!values.empty() && values[0])
        {
            Quantity q;
            q.single_precision_source = isSinglePrecision(*values[0]);
            q.unit_as_written = unit == "1/K" ? "" : unit;
            try
            {
                if (legacy::expansivityLabelIsSwapped(unit, c.diagnostics, where(c, legacy::MExpansivity)))
                    q.value = *values[0];  // the value is in 1/K
                else
                {
                    // the legacy parser multiplies by 1e5 only for the swapped label
                    report(c, Severity::Warning, "LEG-QRK-012", legacy::MExpansivity, "the unit of m_expansivity is " + unit);
                    q.value = units::convertAny(*values[0], unit, "1/K") * 1e-5;
                }
                if (std::fabs(q.value) > emptyMarker)
                    alpha = q;
            }
            catch (const std::exception& ex)
            {
                report(c, Severity::Error, "V2-UNIT-001", legacy::MExpansivity, ex.what());
            }
        }
    }
    if (isPresent(j, legacy::MCompressibility) && j[legacy::MCompressibility].is_object())
    {
        const auto& e = j[legacy::MCompressibility];
        const auto values = numbersOf(e, "values");
        std::string unit = "kbar";
        if (e.contains("units") && e["units"].is_array() && !e["units"].empty() && e["units"][0].is_string())
            unit = e["units"][0].get<std::string>();
        if (!values.empty() && values[0])
        {
            Quantity q;
            q.single_precision_source = isSinglePrecision(*values[0]);
            q.unit_as_written = unit == "kbar" ? "" : unit;
            try
            {
                if (legacy::compressibilityLabelIsSwapped(unit, c.diagnostics, where(c, legacy::MCompressibility)))
                    unit = "kbar";
                q.value = units::convertAny(*values[0], unit, "Pa");
                if (std::fabs(*values[0]) > emptyMarker)
                    kappa = q;
            }
            catch (const std::exception& ex)
            {
                report(c, Severity::Error, "V2-UNIT-001", legacy::MCompressibility, ex.what());
            }
        }
    }

    // the values go to every method that declares them, and to the record if no method does
    bool alphaPlaced = false, kappaPlaced = false;
    for (auto& app : s.methods)
    {
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, OperatesOn::Substance);
        if (!spec)
            continue;
        for (const auto& p : spec->parameters)
        {
            if (alpha && p.name == "alpha0")
            {
                app.parameters["alpha0"] = *alpha;
                alphaPlaced = true;
            }
            if (kappa && p.name == "kappa0")
            {
                app.parameters["kappa0"] = *kappa;
                kappaPlaced = true;
            }
        }
    }
    if (alpha && !alphaPlaced)
        s.structural["alpha0"] = *alpha;
    if (kappa && !kappaPlaced)
        s.structural["kappa0"] = *kappa;
}

// The methods of a record, and what depends on them
void readSubstanceMethods(const json& j, SubstanceRecord& s, const Context& c)
{
    MethodFlags flags;
    s.methods = readMethods(j, OperatesOn::Substance, c, flags);
    applyWaterVapourRule(s, flags);
    readHp98Values(j, s, c);

    // from a reaction: a defining reaction and no methods, no method at all, or only a constant volume
    const bool onlyConstantVolume = s.methods.size() == 1 && s.methods[0].method == "ConstantVolume";
    if (s.methods.empty() || onlyConstantVolume)
        s.preferred_property_source = PropertySource::Reaction;
}

void readReactionMethods(const json& j, ReactionRecord& r, const Context& c)
{
    MethodFlags flags;
    r.methods = readMethods(j, OperatesOn::Reaction, c, flags);
    mergeLogKMethods(r.methods);
    if (r.methods.empty())
        r.preferred_property_source = PropertySource::Reactants;
}

// the keys that the reader handles, per kind of record
const std::set<std::string>& knownSubstanceKeys()
{
    static const std::set<std::string> k = {
        legacy::Symbol, legacy::Name, legacy::Formula, legacy::FormulaCharge, legacy::AggregateState, legacy::Class, legacy::Tst,
        legacy::Pst, legacy::SmGibbsEnergy, legacy::SmEnthalpy, legacy::SmEntropyAbs, legacy::SmHeatCapacityP, legacy::SmVolume,
        legacy::TPMethods, legacy::MExpansivity, legacy::MCompressibility, legacy::MassPerMole, legacy::Reaction,
        legacy::Datasources, legacy::References, legacy::Label, legacy::LimitsTP};
    return k;
}

const std::set<std::string>& knownReactionKeys()
{
    static const std::set<std::string> k = {
        legacy::Symbol, legacy::Name, legacy::Equation, legacy::Reactants, legacy::Tst, legacy::Pst, legacy::LogKr,
        legacy::DrsmGibbsEnergy, legacy::DrsmEnthalpy, legacy::DrsmEntropy, legacy::DrsmHeatCapacityP, legacy::DrsmVolume,
        legacy::LimitsTP, legacy::TPMethods, legacy::Datasources, legacy::References, legacy::Label};
    return k;
}

const std::set<std::string>& knownElementKeys()
{
    static const std::set<std::string> k = {legacy::Symbol, legacy::Name,     legacy::Class,       legacy::Entropy,    legacy::HeatCapacity,
                                            legacy::AtomicMass, legacy::Volume, legacy::IsotopeMass, legacy::Number,
                                            legacy::Datasources, legacy::References, legacy::Label};
    return k;
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

}

// ---------------------------------------------------------------------------------------------------------------

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

} // namespace ThermoFun::v2
