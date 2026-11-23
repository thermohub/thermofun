// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Route.h"

#include <cmath>
#include <cstdio>
#include <string_view>

namespace ThermoFun {

auto Route::total() const -> RouteValues
{
    RouteValues sum = contribution;
    for (const auto& m : methods)
        sum += m.contribution;
    for (const auto& c : conversions)
        sum += c.change;
    return sum;
}

namespace {

auto number(double x) -> std::string
{
    if (std::isnan(x))
        return ".nan";
    if (std::isinf(x))
        return x > 0 ? ".inf" : "-.inf";
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.12g", x);
    return buf;
}

/// A plain YAML scalar if the text allows it, else in double quotes
auto text(const std::string& s) -> std::string
{
    constexpr std::string_view special = ":#{}[],&*!|>'\"%@`";
    bool quote = s.empty() || s.front() == '-' || s.front() == '?' || s.front() == ' ' || s.back() == ' ' || s == "null" || s == "true" || s == "false" ||
                 s == "yes" || s == "no";
    for (const char c : s)
        quote = quote || special.find(c) != std::string_view::npos;
    if (!quote)
        return s;
    std::string out = "\"";
    for (const char c : s)
    {
        if (c == '"' || c == '\\')
            out += '\\';
        out += c;
    }
    return out + "\"";
}

/// G, H, S, Cp, V (dG, ... of a reaction: the same keys)
auto values(const RouteValues& v) -> std::string
{
    return "{G: " + number(v.G) + ", H: " + number(v.H) + ", S: " + number(v.S) + ", Cp: " + number(v.Cp) + ", V: " + number(v.V) + "}";
}

auto numbers(const std::vector<RouteNumber>& list) -> std::string
{
    std::string out = "{";
    for (std::size_t i = 0; i < list.size(); ++i)
        out += (i ? ", " : "") + text(list[i].name) + ": " + number(list[i].value);
    return out + "}";
}

auto write(std::string& out, const Route& r, const std::string& pad) -> void
{
    const bool numbersKnown = !std::isnan(r.T);
    out += pad + "symbol: " + text(r.symbol) + "\n";
    out += pad + "kind: " + r.kind + "\n";
    if (numbersKnown)
    {
        out += pad + "T: " + number(r.T) + "\n";
        out += pad + "P: " + number(r.P) + "\n";
    }
    out += pad + "by: " + (r.by.empty() ? std::string("\"\"") : text(r.by)) + "\n";
    if (numbersKnown && !r.contribution.isZero())
        out += pad + "contribution: " + values(r.contribution) + "\n";
    if (!r.methods.empty())
    {
        out += pad + "methods:\n";
        for (const auto& m : r.methods)
        {
            out += pad + "  - method: " + text(m.method) + "\n";
            if (!m.implementation.empty())
                out += pad + "    implementation: " + text(m.implementation) + "\n";
            if (!m.region.empty())
                out += pad + "    region: " + text(m.region) + "\n";
            if (numbersKnown)
                out += pad + "    contribution: " + values(m.contribution) + "\n";
            if (!m.terms.empty())
                out += pad + "    terms: " + numbers(m.terms) + "\n";
            if (!m.variables.empty())
                out += pad + "    variables: " + numbers(m.variables) + "\n";
        }
    }
    if (!r.needs.empty())
    {
        out += pad + "needs:\n";
        for (const auto& n : r.needs)
        {
            out += pad + "  - role: " + n.role + "\n";
            if (n.coefficient != 1.0 || n.role == "reactant")
                out += pad + "    coefficient: " + number(n.coefficient) + "\n";
            if (n.route)
            {
                out += pad + "    route:\n";
                write(out, *n.route, pad + "      ");
            }
        }
    }
    if (!r.conversions.empty())
    {
        out += pad + "conversions:\n";
        for (const auto& c : r.conversions)
        {
            out += pad + "  - what: " + text(c.what) + "\n";
            out += pad + "    detail: " + text(c.detail) + "\n";
            if (numbersKnown)
                out += pad + "    change: " + values(c.change) + "\n";
        }
    }
    if (!r.notes.empty())
    {
        out += pad + "notes:\n";
        for (const auto& n : r.notes)
            out += pad + "  - " + text(n.text) + "\n";
    }
}

} // namespace

auto toYaml(const Route& route) -> std::string
{
    std::string out;
    write(out, route, "");
    return out;
}

} // namespace ThermoFun
