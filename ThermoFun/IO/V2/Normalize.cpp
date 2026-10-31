// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "detail/Normalize.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/V2/V2Keys.h>

namespace ThermoFun::v2 {

namespace {


using json = nlohmann::json;

void report(Diagnostics& diag, Severity severity, const char* code, const std::string& path, const std::string& message)
{
    Diagnostic d;
    d.severity = severity;
    d.code = code;
    d.path = path;
    d.message = message;
    diag.add(d);
}

// A word of a vocabulary: an unknown word gives a warning with the words allowed, and no value
template <class E>
auto readWord(const std::string& word, Diagnostics& diag, const std::string& path) -> std::optional<E>
{
    if (word.empty())
        return std::nullopt;
    const auto value = fromString<E>(word);
    if (!value)
        report(diag, Severity::Warning, "V2-VOC-001", path, unknownWordMessage<E>(word));
    return value;
}

// The base atom of each dimension that a scope can change
const std::pair<const char*, const char*> dimensions[] = {
    {v2key::energy, "J"}, {v2key::pressure, "Pa"}, {v2key::temperature, "K"},
    {v2key::length, "m"}, {v2key::mass, "kg"}, {v2key::quantity, "mol"}};

auto scopeDefault(const UnitScope& scope, const std::string& dimension) -> const std::string*
{
    for (const UnitScope* s = &scope; s; s = s->parent)
    {
        auto it = s->defaults.find(dimension);
        if (it != s->defaults.end())
            return &it->second;
    }
    return nullptr;
}

auto isNumber(const std::string& text, double& value) -> bool
{
    if (text.empty())
        return false;
    char* end = nullptr;
    value = std::strtod(text.c_str(), &end);
    return end == text.c_str() + text.size() && std::isfinite(value);
}

// Checks that a unit can be parsed and has the dimension of the slot. Returns false with an error in the diagnostics.
auto checkUnit(const std::string& unit, const std::string& slot, Diagnostics& diag, const std::string& path) -> bool
{
    try
    {
        if (units::dimension(unit) != units::dimension(slot))
        {
            report(diag, Severity::Error, "V2-UNIT-002", path, "The unit " + unit + " has not the dimension of " + slot);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        report(diag, Severity::Error, "V2-UNIT-001", path, std::string("The unit ") + unit + " cannot be read: " + e.what());
        return false;
    }
    return true;
}

// A number in an explicit or default unit, as the value in the SI unit of the slot
auto toSi(double x, const std::string& unit, const std::string& slot) -> double
{
    return unit == slot ? x : units::convertAny(x, unit, slot);
}

auto toSiDifference(double x, const std::string& unit, const std::string& slot) -> double
{
    return unit == slot ? x : units::convertDifference(x, unit, slot);
}

// reads the uncertainty (a number or an object) given in `unit`
auto readUncertainty(const json& node, const std::string& unit, const std::string& slot, Diagnostics& diag,
                     const std::string& path) -> std::optional<Uncertainty>
{
    Uncertainty u;
    if (node.is_number())
    {
        const double x = node.get<double>();
        if (x < 0.0)
        {
            report(diag, Severity::Error, "V2-UNC-003", path, "The uncertainty is negative.");
            return std::nullopt;
        }
        u.standard = toSiDifference(x, unit, slot);
        return u;
    }
    if (!node.is_object())
    {
        report(diag, Severity::Error, "V2-QTY-002", path, "The uncertainty must be a number or an object.");
        return std::nullopt;
    }
    const bool hasStandard = node.contains(v2key::standard), hasExpanded = node.contains(v2key::expanded);
    if (hasStandard && hasExpanded)
    {
        report(diag, Severity::Error, "V2-UNC-002", path, "Both standard and expanded uncertainty are given.");
        return std::nullopt;
    }
    if (hasExpanded && !node.contains(v2key::coverage_factor))
    {
        report(diag, Severity::Error, "V2-UNC-001", path, "The expanded uncertainty has no coverage factor.");
        return std::nullopt;
    }
    double written = 0.0;
    if (hasStandard || hasExpanded)
    {
        const json& n = node[hasStandard ? v2key::standard : v2key::expanded];
        if (!n.is_number())
        {
            report(diag, Severity::Error, "V2-QTY-002", path, "The uncertainty must be a number.");
            return std::nullopt;
        }
        written = n.get<double>();
    }
    if (written < 0.0)
    {
        report(diag, Severity::Error, "V2-UNC-003", path, "The uncertainty is negative.");
        return std::nullopt;
    }
    double k = 1.0;
    if (node.contains(v2key::coverage_factor))
    {
        const json& n = node[v2key::coverage_factor];
        k = n.is_number() ? n.get<double>() : std::nan("");
        if (!std::isfinite(k) || k <= 0.0)
        {
            report(diag, Severity::Error, "V2-UNC-004", path, "The coverage factor must be a finite number above zero.");
            return std::nullopt;
        }
    }
    u.coverage_factor_as_written = k;
    u.standard = toSiDifference(written / (hasExpanded ? k : 1.0), unit, slot);
    if (node.contains(v2key::distribution) && node[v2key::distribution].is_string())
        u.distribution = node[v2key::distribution].get<std::string>();
    if (node.contains(v2key::degrees_of_freedom) && node[v2key::degrees_of_freedom].is_number())
        u.degrees_of_freedom = node[v2key::degrees_of_freedom].get<double>();
    if (node.contains(v2key::kind) && node[v2key::kind].is_string())
        u.kind = node[v2key::kind].get<std::string>() == "derived" ? Uncertainty::Kind::Derived : Uncertainty::Kind::Primary;
    return u;
}

auto text(const json& node, const char* key) -> std::string
{
    return node.contains(key) && node[key].is_string() ? node[key].get<std::string>() : std::string();
}

}

auto readUnitScope(const json& node, const UnitScope* parent, Diagnostics& diag, const std::string& path) -> UnitScope
{
    UnitScope scope;
    scope.parent = parent;
    if (!node.is_object())
    {
        report(diag, Severity::Error, "V2-QTY-002", path, "The units must be a mapping.");
        return scope;
    }
    for (const auto& item : node.items())
    {
        bool known = false;
        for (const auto& d : dimensions)
            known = known || item.key() == d.first;
        if (!known || !item.value().is_string())
        {
            report(diag, Severity::Warning, "V2-QTY-003", path + "/" + item.key(), "Unknown dimension in units: " + item.key());
            continue;
        }
        const std::string unit = item.value().get<std::string>();
        for (const auto& d : dimensions)
            if (item.key() == d.first && checkUnit(unit, d.second, diag, path + "/" + item.key()))
                scope.defaults[item.key()] = unit;
    }
    return scope;
}

auto scopedUnit(const std::string& si_unit, const UnitScope& scope) -> std::string
{
    std::string out;
    size_t i = 0;
    while (i < si_unit.size())
    {
        if (std::isalpha(static_cast<unsigned char>(si_unit[i])))
        {
            size_t j = i;
            while (j < si_unit.size() && std::isalpha(static_cast<unsigned char>(si_unit[j])))
                ++j;
            const std::string atom = si_unit.substr(i, j - i);
            std::string replaced = atom;
            for (const auto& d : dimensions)
                if (atom == d.second)
                    if (const std::string* unit = scopeDefault(scope, d.first))
                        replaced = *unit;
            out += replaced;
            i = j;
        }
        else
            out += si_unit[i++];
    }
    return out;
}

auto readQuantity(const json& node, const std::string& slot_si_unit, const UnitScope& scope, Diagnostics& diag,
                  const std::string& path) -> std::optional<Quantity>
{
    Quantity q;
    double number = 0.0;
    std::string unit;      // the unit that the user wrote
    const json* uncertainty = nullptr;

    if (node.is_number())
        number = node.get<double>();
    else if (node.is_string())
    {
        const std::string s = node.get<std::string>();
        const size_t space = s.find(' ');
        if (space == std::string::npos || !isNumber(s.substr(0, space), number))
        {
            report(diag, Severity::Error, "V2-QTY-001", path, "The quantity \"" + s + "\" is not \"number unit\".");
            return std::nullopt;
        }
        unit = s.substr(space + 1);
        while (!unit.empty() && unit.front() == ' ')
            unit.erase(unit.begin());
    }
    else if (node.is_object())
    {
        if (!node.contains(v2key::value) || !node[v2key::value].is_number())
        {
            report(diag, Severity::Error, "V2-QTY-002", path, "The quantity has no value.");
            return std::nullopt;
        }
        for (const auto& item : node.items())
        {
            const std::string& key = item.key();
            if (key != v2key::value && key != v2key::unit && key != v2key::uncertainty && key != v2key::quality &&
                key != v2key::origin && key != v2key::calculation_path && key != v2key::source)
                report(diag, Severity::Warning, "V2-QTY-003", path + "/" + key, "Unknown key in a quantity: " + key);
        }
        number = node[v2key::value].get<double>();
        unit = text(node, v2key::unit);
        if (node.contains(v2key::uncertainty))
            uncertainty = &node[v2key::uncertainty];
        q.meta.quality = readWord<Quality>(text(node, v2key::quality), diag, path + "/" + v2key::quality);
        q.meta.origin = readWord<Origin>(text(node, v2key::origin), diag, path + "/" + v2key::origin);
        q.meta.calculation_path = text(node, v2key::calculation_path);
        q.meta.source = text(node, v2key::source);
    }
    else
    {
        report(diag, Severity::Error, "V2-QTY-002", path, "The quantity must be a number, a string or an object.");
        return std::nullopt;
    }

    // the unit: the written one, else the default of the scope, else the unit of the slot
    const bool explicitUnit = !unit.empty();
    if (!explicitUnit)
        unit = scopedUnit(slot_si_unit, scope);
    if (!checkUnit(unit, slot_si_unit, diag, path))
        return std::nullopt;

    q.value = toSi(number, unit, slot_si_unit);
    if (unit != slot_si_unit)
    {
        q.unit_as_written = unit;
        q.value_as_written = nlohmann::json(number).dump();
    }
    if (uncertainty)
    {
        auto u = readUncertainty(*uncertainty, unit, slot_si_unit, diag, path + "/" + v2key::uncertainty);
        if (!u)
            return std::nullopt;
        q.uncertainty = u;
    }
    return q;
}

auto writeQuantity(const Quantity& q, const std::string& slot_si_unit) -> json
{
    const std::string& unit = q.unit_as_written;
    const bool inSlotUnit = unit.empty();
    const double written = inSlotUnit ? q.value
                           : !q.value_as_written.empty() ? std::stod(q.value_as_written)
                                                : units::convertAny(q.value, slot_si_unit, unit);

    json uncertainty;
    bool hasUncertainty = false;
    if (q.uncertainty)
    {
        hasUncertainty = true;
        const Uncertainty& u = *q.uncertainty;
        const double standard = inSlotUnit ? u.standard : units::convertDifference(u.standard, slot_si_unit, unit);
        const bool simple = u.coverage_factor_as_written == 1.0 && u.kind == Uncertainty::Kind::Primary &&
                            u.distribution.empty() && !u.degrees_of_freedom;
        if (simple)
            uncertainty = standard;
        else
        {
            uncertainty = json::object();
            if (u.coverage_factor_as_written != 1.0)
            {
                uncertainty[v2key::expanded] = standard * u.coverage_factor_as_written;
                uncertainty[v2key::coverage_factor] = u.coverage_factor_as_written;
            }
            else
                uncertainty[v2key::standard] = standard;
            if (!u.distribution.empty())
                uncertainty[v2key::distribution] = u.distribution;
            if (u.degrees_of_freedom)
                uncertainty[v2key::degrees_of_freedom] = *u.degrees_of_freedom;
            if (u.kind == Uncertainty::Kind::Derived)
                uncertainty[v2key::kind] = "derived";
        }
    }

    const bool noMeta = !q.meta.quality && !q.meta.origin && q.meta.calculation_path.empty() && q.meta.source.empty();
    if (!hasUncertainty && noMeta && inSlotUnit)
        return written;

    json out = json::object();
    out[v2key::value] = written;
    if (!inSlotUnit)
        out[v2key::unit] = unit;
    if (hasUncertainty)
        out[v2key::uncertainty] = uncertainty;
    if (q.meta.quality)
        out[v2key::quality] = toString(*q.meta.quality);
    if (q.meta.origin)
        out[v2key::origin] = toString(*q.meta.origin);
    if (!q.meta.calculation_path.empty())
        out[v2key::calculation_path] = q.meta.calculation_path;
    if (!q.meta.source.empty())
        out[v2key::source] = q.meta.source;
    return out;
}

}
