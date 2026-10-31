// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// How a value is written: the shortest number that reads back, and its unit.

#include <ThermoFun/IO/V2/detail/V2WriterParts.h>

namespace ThermoFun::v2::v2writer {

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

} // namespace ThermoFun::v2::v2writer
