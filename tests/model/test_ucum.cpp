// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of Common/Ucum.h: translation, dimension and conversion of unit strings.

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Common/Ucum.h"
#include "check.h"

using namespace ThermoFun::units;

namespace {

bool near(double a, double b, double rel = 1e-12)
{
    return std::fabs(a - b) <= rel * std::fmax(std::fabs(a), std::fabs(b));
}

bool throws(const std::string& unit)
{
    try
    {
        fromUcum(unit);
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }
    return false;
}

void testTranslation()
{
    const std::vector<std::pair<std::string, std::string>> pairs = {
        {"J/(mol.K)", "J/(mol*K)"},
        {"J/(mol.K2)", "J/(mol*K^2)"},
        {"cm3/mol", "cm^3/mol"},
        {"m3/mol", "m^3/mol"},
        {"s-1", "1/s"},
        {"J.K-1.mol-1", "J/(K*mol)"},
        {"kg.m2/s2", "kg*m^2/s^2"},
        {"Cel", "degC"},
        {"[degF]", "degF"},
        {"[atm]", "atm"},
        {"cal_IT", "cal_IT"},
        {"cal_th", "cal"},
        {"kcal_th/mol", "kcal/mol"},
        {"%", "percent"},
        {"1", "1"},
        {"10*-5/K", "1e-05/K"},
        {"10*3.mol", "1000*mol"},
        {"kJ/mol", "kJ/mol"},
        {"J/mol", "J/mol"},
        {"mol/kg", "mol/kg"},
        {"K/bar", "K/bar"},
        {"kbar", "kbar"},
        {"g/mol", "g/mol"},
        {"hPa", "hPa"},
        {"mmol/L", "mmol/L"},
        {"min", "min"},
        {"h", "hour"},
        {"J/(mol.K)", "J/(mol*K)"},
        {"(J.K2)/mol", "J*K^2/mol"},
    };
    for (const auto& p : pairs)
    {
        const std::string got = fromUcum(p.first);
        if (got != p.second)
            std::printf("  fromUcum(%s) = %s, expected %s\n", p.first.c_str(), got.c_str(), p.second.c_str());
        CHECK(got == p.second);
    }

    // a string in converter syntax is not changed
    for (const char* s : {"J/(mol*K)", "(J*K^2)/mol", "J/(mol*K^0.5)", "(cal*K)/(mol*bar)", "J/(K*mol)"})
        CHECK(fromUcum(s) == s);

    // units that are not supported
    CHECK(throws("foo"));
    CHECK(throws("J/(mol"));
    CHECK(throws(""));
    CHECK(throws("J/mol)"));
    CHECK(throws("[in_i]"));
}

void testBundledUnits()
{
    // every unit string of the bundled data and of the fixtures (the empty string is not a unit)
    const char* const units[] = {
        "(J*K)/mol", "(J*K^2)/mol", "(cal*K)/(mol*bar)", "(cal*K)/mol", "1", "1/kbar", "1e-05/K", "J/(K*mol)",
        "J/(mol*K)", "J/(mol*K^0.5)", "J/(mol*K^1.5)", "J/(mol*K^2)", "J/(mol*K^3)", "J/(mol*K^4)", "J/(mol*K^5)",
        "J/bar", "J/mol", "K", "K/bar", "bar", "cal/(mol*K)", "cal/(mol*bar)", "cal/mol", "g/mol", "kJ/(mol*K)",
        "kJ/mol", "kbar"};
    for (const char* u : units)
    {
        bool ok = true;
        try
        {
            dimension(u);
            fromUcum(u);
        }
        catch (const std::exception& e)
        {
            std::printf("  unit %s: %s\n", u, e.what());
            ok = false;
        }
        CHECK(ok);
    }
}

void testDimension()
{
    using D = std::array<double, 7>;
    CHECK(dimension("J/(mol.K)") == (D{1, 2, -2, -1, -1, 0, 0}));
    CHECK(dimension("J/(mol*K)") == (D{1, 2, -2, -1, -1, 0, 0}));
    CHECK(dimension("J/mol") == (D{1, 2, -2, 0, -1, 0, 0}));
    CHECK(dimension("kJ/mol") == dimension("J/mol"));
    CHECK(dimension("cal/mol") == dimension("J/mol"));
    CHECK(dimension("Pa") == (D{1, -1, -2, 0, 0, 0, 0}));
    CHECK(dimension("bar") == dimension("Pa"));
    CHECK(dimension("kbar") == dimension("Pa"));
    CHECK(dimension("1/kbar") == (D{-1, 1, 2, 0, 0, 0, 0}));
    CHECK(dimension("J/bar") == (D{0, 3, 0, 0, 0, 0, 0}));  // a volume
    CHECK(dimension("m3/mol") == (D{0, 3, 0, 0, -1, 0, 0}));
    CHECK(dimension("K/bar") == (D{-1, 1, 2, 1, 0, 0, 0}));
    CHECK(dimension("g/mol") == (D{1, 0, 0, 0, -1, 0, 0}));
    CHECK(dimension("1") == (D{0, 0, 0, 0, 0, 0, 0}));
    CHECK(dimension("%") == dimension("1"));
    CHECK(dimension("J/(mol*K^0.5)") == (D{1, 2, -2, -0.5, -1, 0, 0}));
    CHECK(dimension("(J*K^2)/mol") == (D{1, 2, -2, 2, -1, 0, 0}));
    CHECK(dimension("Cel") == dimension("K"));
    CHECK(dimension("1e-05/K") == (D{0, 0, 0, -1, 0, 0, 0}));
}

void testConversion()
{
    CHECK(near(convertAny(1.0, "kJ/mol", "J/mol"), 1000.0));
    CHECK(near(convertAny(1.0, "cal/mol", "J/mol"), 4.184));
    CHECK(near(convertAny(1.0, "cal_th/mol", "J/mol"), 4.184));
    CHECK(near(convertAny(1.0, "cal_IT/mol", "J/mol"), 4.1868));
    CHECK(near(convertAny(1.0, "bar", "Pa"), 1e5));
    CHECK(near(convertAny(1.0, "kbar", "Pa"), 1e8));
    CHECK(near(convertAny(1.0, "J/bar", "m3"), 1e-5));
    CHECK(near(convertAny(1.0, "J/(mol.K)", "kJ/(mol*K)"), 1e-3));
    CHECK(near(convertAny(1.0, "g/mol", "kg/mol"), 1e-3));
    CHECK(near(convertAny(25.0, "Cel", "K"), 298.15));
    CHECK(near(convertAny(298.15, "K", "degC"), 25.0, 1e-9));
    CHECK(near(convertAny(1.0, "10*-5/K", "1/K"), 1e-5));
    CHECK(near(convertAny(1.0, "1e-05/K", "1/K"), 1e-5));
    CHECK(near(convertAny(100.0, "%", "1"), 1.0));

    // the units must have the same dimension
    bool thrown = false;
    try
    {
        convertAny(1.0, "J", "K");
    }
    catch (const std::invalid_argument&)
    {
        thrown = true;
    }
    CHECK(thrown);

    // differences: no offset
    CHECK(near(convertDifference(1.0, "degC", "K"), 1.0));
    CHECK(near(convertDifference(1.0, "K", "degC"), 1.0));
    CHECK(near(convertDifference(9.0, "degF", "K"), 5.0));
    CHECK(near(convertDifference(2.0, "kJ/mol", "J/mol"), 2000.0));
}

}

void runUcum()
{
    testTranslation();
    testBundledUnits();
    testDimension();
    testConversion();
}
