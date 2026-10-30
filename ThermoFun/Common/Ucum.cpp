// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Ucum.h"

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
};

const std::vector<Atom>& atoms()
{
    static const std::vector<Atom> table = {
        {"g", "g", dim(1, 0, 0), true},
        {"m", "m", dim(0, 1, 0), true},
        {"s", "s", dim(0, 0, 1), true},
        {"K", "K", dim(0, 0, 0, 1), true},
        {"mol", "mol", dim(0, 0, 0, 0, 1), true},
        {"A", "A", dim(0, 0, 0, 0, 0, 1), true},
        {"cd", "cd", dim(0, 0, 0, 0, 0, 0, 1), true},
        {"J", "J", dim(1, 2, -2), true},
        {"N", "N", dim(1, 1, -2), true},
        {"Pa", "Pa", dim(1, -1, -2), true},
        {"bar", "bar", dim(1, -1, -2), true},
        {"atm", "atm", dim(1, -1, -2), false},
        {"torr", "torr", dim(1, -1, -2), false},
        {"cal", "cal", dim(1, 2, -2), true},
        {"cal_IT", "cal_IT", dim(1, 2, -2), true},
        {"cal_th", "cal", dim(1, 2, -2), true},
        {"eV", "eV", dim(1, 2, -2), true},
        {"W", "W", dim(1, 2, -3), true},
        {"Hz", "Hz", dim(0, 0, -1), true},
        {"V", "V", dim(1, 2, -3, 0, 0, -1), true},
        {"C", "C", dim(0, 0, 1, 0, 0, 1), true},
        {"L", "L", dim(0, 3, 0), true},
        {"l", "L", dim(0, 3, 0), true},
        {"min", "min", dim(0, 0, 1), false},
        {"h", "hour", dim(0, 0, 1), false},
        {"d", "day", dim(0, 0, 1), false},
        {"Cel", "degC", dim(0, 0, 0, 1), false},
        {"degC", "degC", dim(0, 0, 0, 1), false},
        {"degF", "degF", dim(0, 0, 0, 1), false},
        {"degR", "degR", dim(0, 0, 0, 1), false},
        {"percent", "percent", dim(0, 0, 0), false},
        {"%", "percent", dim(0, 0, 0), false},
        {"ppm", "ppm", dim(0, 0, 0), false},
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
    std::string name;
    double exponent;
    Dim dimension;
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
        return std::strtod(s_.substr(start, pos_ - start).c_str(), nullptr);
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
        return Term{converterName, e, a->dimension};
    }
};

// A prefix changes the factor but not the dimension; the dimension of the term is the dimension of the atom.

Parsed parseUnit(const std::string& text)
{
    return Parser(text).parse();
}

bool isConverterSyntax(const std::string& s)
{
    // "10*-5" is a power of ten of UCUM and not a product
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '^')
            return true;
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
    double factor;
    Dim dimension;
};

Prepared prepare(const std::string& text)
{
    Parsed p = parseUnit(text);
    Prepared out;
    out.factor = p.factor;
    out.dimension = dimensionOf(p);
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
