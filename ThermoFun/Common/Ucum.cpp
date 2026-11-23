// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Ucum.h"
#include "Numbers.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Units.hpp"

namespace ThermoFun {
namespace units {

namespace {

using Dim = std::array<double, 7>;  // kg, m, s, K, mol, A, cd

Dim dim(double kg, double m, double s, double K = 0, double mol = 0, double A = 0, double cd = 0)
{
    return {kg, m, s, K, mol, A, cd};
}

// An atom without prefix: the name in UCUM, the name in the converter, the dimension, and whether a prefix is allowed.
struct Atom
{
    const char* ucum;
    const char* converter;
    Dim dimension;
    bool prefix;
    const char* si;  // the coherent SI unit that the converter knows, for the factor of the atom
};

const std::vector<Atom>& atoms()
{
    static const std::vector<Atom> table = {
        {"g", "g", dim(1, 0, 0), true, "kg"},
        {"m", "m", dim(0, 1, 0), true, "m"},
        {"s", "s", dim(0, 0, 1), true, "s"},
        {"K", "K", dim(0, 0, 0, 1), true, "K"},
        {"mol", "mol", dim(0, 0, 0, 0, 1), true, "mol"},
        {"A", "A", dim(0, 0, 0, 0, 0, 1), true, "A"},
        {"cd", "cd", dim(0, 0, 0, 0, 0, 0, 1), true, "cd"},
        {"J", "J", dim(1, 2, -2), true, "J"},
        {"N", "N", dim(1, 1, -2), true, "N"},
        {"Pa", "Pa", dim(1, -1, -2), true, "Pa"},
        {"bar", "bar", dim(1, -1, -2), true, "Pa"},
        {"atm", "atm", dim(1, -1, -2), false, "Pa"},
        {"torr", "torr", dim(1, -1, -2), false, "Pa"},
        {"cal", "cal", dim(1, 2, -2), true, "J"},
        {"cal_IT", "cal_IT", dim(1, 2, -2), true, "J"},
        {"cal_th", "cal", dim(1, 2, -2), true, "J"},
        {"eV", "eV", dim(1, 2, -2), true, "J"},
        {"W", "W", dim(1, 2, -3), true, "W"},
        {"Hz", "Hz", dim(0, 0, -1), true, "Hz"},
        {"V", "V", dim(1, 2, -3, 0, 0, -1), true, "V"},
        {"C", "C", dim(0, 0, 1, 0, 0, 1), true, "C"},
        {"L", "L", dim(0, 3, 0), true, "m^3"},
        {"l", "L", dim(0, 3, 0), true, "m^3"},
        {"min", "min", dim(0, 0, 1), false, "s"},
        {"h", "hour", dim(0, 0, 1), false, "s"},
        {"d", "day", dim(0, 0, 1), false, "s"},
        {"Cel", "degC", dim(0, 0, 0, 1), false, "K"},
        {"degC", "degC", dim(0, 0, 0, 1), false, "K"},
        {"degF", "degF", dim(0, 0, 0, 1), false, "K"},
        {"degR", "degR", dim(0, 0, 0, 1), false, "K"},
        {"percent", "percent", dim(0, 0, 0), false, "1"},
        {"%", "percent", dim(0, 0, 0), false, "1"},
        {"ppm", "ppm", dim(0, 0, 0), false, "1"},
    };
    return table;
}

// The prefixes of the SI. "da" has two letters and is not supported.
const std::map<char, double>& prefixes()
{
    static const std::map<char, double> table = {
        {'y', 1e-24}, {'z', 1e-21}, {'a', 1e-18}, {'f', 1e-15}, {'p', 1e-12}, {'n', 1e-9}, {'u', 1e-6},
        {'m', 1e-3},  {'c', 1e-2},  {'d', 1e-1},  {'h', 1e2},   {'k', 1e3},   {'M', 1e6},  {'G', 1e9},
        {'T', 1e12},  {'P', 1e15},  {'E', 1e18},
    };
    return table;
}

// The atom for a name, with or without a prefix. Returns nullptr if the name is not known.
const Atom* findAtom(const std::string& name)
{
    for (const auto& a : atoms())
        if (name == a.ucum)
            return &a;
    if (name.size() > 1 && prefixes().count(name[0]))
        for (const auto& a : atoms())
            if (a.prefix && name.compare(1, std::string::npos, a.ucum) == 0)
                return &a;
    return nullptr;
}

// One atom with an exponent. The name is the name in the converter syntax.
struct Term
{
    std::string name;       // the name in the converter, with the prefix
    double exponent;
    Dim dimension;
    std::string base;       // the name of the atom in the converter, without the prefix
    double prefix = 1.0;    // the factor of the prefix
    std::string si;         // the coherent SI unit of the atom
    bool offset = false;    // a temperature with an offset: degC, degF, degR
};

struct Parsed
{
    std::vector<Term> terms;
    double factor = 1.0;  // a number in front of the unit, for example 1e-5 in 10*-5/K
};

class Parser
{
public:
    explicit Parser(const std::string& text) : s_(text) {}

    Parsed parse()
    {
        Parsed out;
        expression(out, 1.0);
        if (pos_ != s_.size())
            fail(s_.substr(pos_));
        return out;
    }

private:
    const std::string& s_;
    size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& token) const
    {
        throw std::invalid_argument("unsupported token \"" + token + "\" in the unit \"" + s_ + "\"");
    }

    bool more() const { return pos_ < s_.size(); }
    char peek() const { return s_[pos_]; }

    // A sequence of factors with the operators . * / (from left to right). sign: +1 or -1, the power of the sequence.
    void expression(Parsed& out, double sign)
    {
        double op = sign;
        if (more() && peek() == '/')
        {
            ++pos_;
            op = -sign;
        }
        while (true)
        {
            factor(out, op);
            if (!more())
                return;
            char c = peek();
            if (c == '.' || c == '*')
                op = sign;
            else if (c == '/')
                op = -sign;
            else
                return;
            ++pos_;
        }
    }

    // The exponent after an atom or a group: ^n, or a signed integer written directly after the atom.
    double exponent(bool allowBare)
    {
        if (more() && peek() == '^')
        {
            ++pos_;
            return number(true, true);
        }
        // directly after an atom only an integer: in "K-1.mol" the point is the product
        if (allowBare && more() && (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '-' || peek() == '+'))
            return number(true, false);
        return 1.0;
    }

    // A number with an optional sign, a decimal point (if decimals) and an exponent of ten (if decimals).
    double number(bool signedAllowed, bool decimals)
    {
        size_t start = pos_;
        if (signedAllowed && more() && (peek() == '-' || peek() == '+'))
            ++pos_;
        size_t digits = pos_;
        while (more() && (std::isdigit(static_cast<unsigned char>(peek())) || (decimals && peek() == '.')))
            ++pos_;
        if (pos_ == digits)
            fail(s_.substr(start));
        // exponent of ten (1e-05), only if a digit or a sign follows the "e"
        if (decimals && more() && (peek() == 'e' || peek() == 'E') && pos_ + 1 < s_.size() &&
            (std::isdigit(static_cast<unsigned char>(s_[pos_ + 1])) || s_[pos_ + 1] == '-' || s_[pos_ + 1] == '+'))
        {
            size_t save = pos_;
            ++pos_;
            if (peek() == '-' || peek() == '+')
                ++pos_;
            size_t expDigits = pos_;
            while (more() && std::isdigit(static_cast<unsigned char>(peek())))
                ++pos_;
            if (pos_ == expDigits)
                pos_ = save;
        }
        return numbers::parseDouble(std::string_view(s_).substr(start, pos_ - start));
    }

    void factor(Parsed& out, double power)
    {
        if (!more())
            fail("");
        char c = peek();
        if (c == '(')
        {
            ++pos_;
            Parsed inner;
            expression(inner, 1.0);
            if (!more() || peek() != ')')
                fail("(");
            ++pos_;
            double e = exponent(false) * power;
            out.factor *= std::pow(inner.factor, e);
            for (auto t : inner.terms)
            {
                t.exponent *= e;
                out.terms.push_back(t);
            }
            return;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.')
        {
            double x = number(false, true);
            // 10*n, the power of ten of UCUM
            if (x == 10.0 && more() && peek() == '*' && pos_ + 1 < s_.size() &&
                (std::isdigit(static_cast<unsigned char>(s_[pos_ + 1])) || s_[pos_ + 1] == '-' || s_[pos_ + 1] == '+'))
            {
                ++pos_;
                double n = number(true, false);
                out.factor *= std::pow(10.0, n * power);
                return;
            }
            out.factor *= std::pow(x, power);
            return;
        }
        out.terms.push_back(atom(power));
    }

    Term atom(double power)
    {
        size_t start = pos_;
        std::string name;
        if (peek() == '[')
        {
            size_t close = s_.find(']', pos_);
            if (close == std::string::npos)
                fail(s_.substr(pos_));
            name = s_.substr(pos_ + 1, close - pos_ - 1);
            pos_ = close + 1;
        }
        else if (peek() == '%')
        {
            name = "%";
            ++pos_;
        }
        else
        {
            while (more() && (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_'))
                ++pos_;
            name = s_.substr(start, pos_ - start);
        }
        if (name.empty())
            fail(s_.substr(start));
        const Atom* a = findAtom(name);
        if (!a)
            fail(name);
        // the name in the converter: the prefix stays, the atom name is replaced
        std::string converterName = name;
        if (name != a->ucum)
            converterName = name.substr(0, 1) + a->converter;
        else
            converterName = a->converter;
        double e = exponent(true) * power;
        Term t{converterName, e, a->dimension, a->converter, 1.0, a->si, false};
        if (name != a->ucum)
            t.prefix = prefixes().at(name[0]);
        const std::string u = a->converter;
        t.offset = (u == "degC" || u == "degF" || u == "degR");
        return t;
    }
};

// A prefix changes the factor but not the dimension; the dimension of the term is the dimension of the atom.

Parsed parseUnit(const std::string& text)
{
    return Parser(text).parse();
}

bool isConverterSyntax(const std::string& s)
{
    // a product with "*" is the converter syntax. "10*-5" is a power of ten of UCUM and not a product
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '*')
        {
            bool powerOfTen = i >= 2 && s.compare(i - 2, 2, "10") == 0 && i + 1 < s.size() &&
                              (std::isdigit(static_cast<unsigned char>(s[i + 1])) || s[i + 1] == '-' || s[i + 1] == '+');
            if (!powerOfTen)
                return true;
        }
    }
    return false;
}

std::string formatNumber(double x)
{
    char buf[64];
    std::snprintf(buf, sizeof buf, "%g", x);
    return buf;
}

std::string formatTerm(const std::string& name, double exponent)
{
    if (exponent == 1.0)
        return name;
    return name + "^" + formatNumber(exponent);
}

// Text in the converter syntax of the terms. withFactor: the number in front is part of the text.
std::string emit(const Parsed& p, bool withFactor)
{
    // equal atoms are added, the order of the first appearance is kept
    std::vector<std::pair<std::string, double>> merged;
    for (const auto& t : p.terms)
    {
        bool found = false;
        for (auto& m : merged)
            if (m.first == t.name)
            {
                m.second += t.exponent;
                found = true;
            }
        if (!found)
            merged.emplace_back(t.name, t.exponent);
    }

    std::string numerator, denominator;
    int denominators = 0;
    for (const auto& m : merged)
    {
        if (m.second == 0.0)
            continue;
        if (m.second > 0)
            numerator += (numerator.empty() ? "" : "*") + formatTerm(m.first, m.second);
        else
        {
            denominator += (denominator.empty() ? "" : "*") + formatTerm(m.first, -m.second);
            ++denominators;
        }
    }
    if (withFactor && p.factor != 1.0)
        numerator = formatNumber(p.factor) + (numerator.empty() ? "" : "*" + numerator);
    if (numerator.empty())
        numerator = "1";
    if (denominators == 0)
        return numerator;
    if (denominators == 1)
        return numerator + "/" + denominator;
    return numerator + "/(" + denominator + ")";
}

Dim dimensionOf(const Parsed& p)
{
    Dim d{};
    for (const auto& t : p.terms)
        for (size_t i = 0; i < d.size(); ++i)
            d[i] += t.dimension[i] * t.exponent;
    return d;
}

// The converter unit and the factor of a unit text
struct Prepared
{
    std::string converter;
    double factor;       // the number in front of the unit
    Dim dimension;
    double siFactor;     // the factor to the coherent SI unit, from the atoms; used if no atom has an offset
    bool offset;         // an atom has an offset
};

Prepared prepare(const std::string& text)
{
    Parsed p = parseUnit(text);
    Prepared out;
    out.factor = p.factor;
    out.dimension = dimensionOf(p);
    out.offset = false;
    out.siFactor = 1.0;
    for (const auto& t : p.terms)
    {
        out.offset = out.offset || t.offset;
        if (!t.offset)
        {
            double atomFactor = t.prefix;
            try
            {
                atomFactor *= convert(1.0, t.base, t.si);
            }
            catch (const std::exception&)
            {
                throw std::invalid_argument("the unit converter does not know the unit \"" + t.base + "\"");
            }
            out.siFactor *= std::pow(atomFactor, t.exponent);
        }
    }
    // a text in the converter syntax without a number in front goes to the converter as it is;
    // the converter has no negative powers, so s^-1 is written again as 1/s
    if (isConverterSyntax(text) && p.factor == 1.0 && text.find("^-") == std::string::npos)
        out.converter = text;
    else
        out.converter = emit(p, false);
    return out;
}

}

auto fromUcum(const std::string& ucum) -> std::string
{
    if (isConverterSyntax(ucum))
        return ucum;
    return emit(parseUnit(ucum), true);
}

auto dimension(const std::string& unit) -> std::array<double, 7>
{
    return dimensionOf(parseUnit(unit));
}

auto convertAny(double value, const std::string& from, const std::string& to) -> double
{
    const Prepared a = prepare(from);
    const Prepared b = prepare(to);
    if (a.dimension != b.dimension)
        throw std::invalid_argument("the units \"" + from + "\" and \"" + to + "\" have different dimensions");
    if (!a.offset && !b.offset)
        return value * (a.factor * a.siFactor) / (b.factor * b.siFactor);
    return convert(value * a.factor / b.factor, a.converter, b.converter);
}

auto tryConvert(double value, const std::string& from, const std::string& to) -> std::optional<double>
{
    try
    {
        return convertAny(value, from, to);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

auto convertDifference(double delta, const std::string& from, const std::string& to) -> double
{
    // the conversion of a unit is a x + b: the difference uses a only
    const double at0 = convertAny(0.0, from, to);
    const double at1 = convertAny(1.0, from, to);
    return delta * (at1 - at0);
}

}
}
