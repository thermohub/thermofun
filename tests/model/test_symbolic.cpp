// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Symbolic expressions: parsing, the limits, derivatives and evaluation. Without SymEngine only the error is checked.

#include <cmath>

#include "Methods/Symbolic/SymbolicExpression.h"
#include "check.h"

void runSymbolic()
{
    using namespace ThermoFun;

    const std::vector<std::string> names = {"T", "P", "a", "b"};

    if (!SymbolicExpression::available())
    {
        try
        {
            SymbolicExpression::parse("T", names);
            CHECK(false);
        }
        catch (const SymbolicError& e)
        {
            CHECK(e.code == "V2-MTH-008");
        }
        return;
    }

    // the value, and the derivatives by hand
    const auto g = SymbolicExpression::parse("a + b*T**2 - exp(-P/1.0e8)*log(T) + sqrt(T)", names);
    const std::vector<double> v = {400.0, 1.0e7, 3.0, 1.0e-3};
    const double T = v[0], P = v[1], a = v[2], b = v[3];
    const double want = a + b * T * T - std::exp(-P / 1.0e8) * std::log(T) + std::sqrt(T);
    CHECK(std::fabs(g.evaluate<double>(v) - want) < 1e-12 * std::fabs(want));
    const double dT = 2 * b * T - std::exp(-P / 1.0e8) / T + 0.5 / std::sqrt(T);
    CHECK(std::fabs(g.diff("T").evaluate<double>(v) - dT) < 1e-12);
    const double dP = std::exp(-P / 1.0e8) / 1.0e8 * std::log(T);
    CHECK(std::fabs(g.diff("P").evaluate<double>(v) - dP) < 1e-18);
    CHECK(std::fabs(g.diff("T").diff("T").evaluate<double>(v) - (2 * b + std::exp(-P / 1.0e8) / (T * T) - 0.25 / std::pow(T, 1.5))) < 1e-12);

    // the same expression on autodiff numbers: the derivative of the value is the derivative of the expression
    {
        std::vector<real> rv(v.begin(), v.end());
        rv[0][1] = 1.0;
        const real r = g.evaluate<real>(rv);
        CHECK(std::fabs(r[0] - want) < 1e-12 * std::fabs(want));
        CHECK(std::fabs(r[1] - dT) < 1e-12);
    }

    // a power with a symbolic exponent, a division by a variable
    const auto h = SymbolicExpression::parse("a**b/T", names);
    CHECK(std::fabs(h.evaluate<double>({400.0, 1e5, 2.0, 3.0}) - 8.0 / 400.0) < 1e-15);

    // a variable named like a constant of SymEngine (E) stays a variable
    const auto k = SymbolicExpression::parse("E*T + exp(1)", {"T", "E"});
    CHECK(std::fabs(k.evaluate<double>({2.0, 5.0}) - (10.0 + std::exp(1.0))) < 1e-12);
    CHECK(std::fabs(k.diff("E").evaluate<double>({2.0, 5.0}) - 2.0) < 1e-12);

    // the same for the lower-case name e
    const auto k2 = SymbolicExpression::parse("e*T**3", {"T", "e"});
    CHECK(std::fabs(k2.evaluate<double>({2.0, 5.0}) - 40.0) < 1e-12);

    // the codes
    auto code = [&](const std::string& text) {
        try
        {
            SymbolicExpression::parse(text, names);
        }
        catch (const SymbolicError& e)
        {
            return e.code;
        }
        return std::string();
    };
    CHECK(code("T + unknown") == "V2-SYM-001");
    CHECK(code("sin(T)") == "V2-SYM-001");
    CHECK(code("T +") == "V2-SYM-002");
    CHECK(code(std::string(2001, ' ') + "T") == "V2-SYM-006");
    std::string deep = "T";
    for (int i = 0; i < 60; ++i)
        deep = "exp(" + deep + ")";
    CHECK(code(deep) == "V2-SYM-006");
    CHECK(code("T**2") == "");
}
