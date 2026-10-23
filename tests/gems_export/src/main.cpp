// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads a small database with the records of volume equations, a water solvent, a gas given by a reaction and a CG fluid, and compares the values with reference values.
// The checks are never compiled out (see tests/memoization/src/main.cpp).

#include <cmath>
#include <cstdio>
#include <string>

#include "ThermoFun/ThermoFun.h"

using namespace ThermoFun;

namespace {

int failures = 0;

void check(bool ok, const char* what, double value, double expected)
{
    if (!ok)
    {
        ++failures;
        printf("  FAIL %s: %.10g, expected %.10g\n", what, value, expected);
    }
}

bool near(double a, double b, double tol) { return std::fabs(a - b) <= tol; }

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
        return 2;
    Database db(argv[1]);
    ThermoEngine engine(db);
    engine.setSolventSymbol("H2O(l)");

    auto props = [&](const char* symbol, double TC, double Pbar) {
        double P = Pbar * 1e5;
        return engine.thermoPropertiesSubstance(TC + 273.15, P, symbol);
    };

    // Volume equation m_volume_fpt_coeffs with the pressure term of second order (reference: 2.97098715 J/bar)
    double v = static_cast<double>(props("Magnesite", 25., 1000.).volume);
    check(near(v, 2.970987, 1e-4), "Magnesite V at 25 C, 1000 bar", v, 2.970987);

    // Water solvent given by reference values and a constant Cp (reference: -243043.965 J/mol)
    double g = static_cast<double>(props("H2O(l)", 100., 1.).gibbs_energy);
    check(near(g, -243043.965, 1e-2), "H2O(l) G at 100 C, 1 bar", g, -243043.965);

    // Gas defined by a reaction without volume method: ideal gas volume (reference: 3102.5594 J/bar)
    v = static_cast<double>(props("H2Se(g)", 100., 1.).volume);
    check(near(v, 3102.5594, 1e-2), "H2Se(g) V at 100 C, 1 bar", v, 3102.5594);

    // CG fluid: the coefficients of eos_churakov_gottschalk_coeffs are read (reference: 4.82426 J/bar)
    v = static_cast<double>(props("CO", 25., 1000.).volume);
    check(near(v, 4.82426, 1e-3), "CO V at 25 C, 1000 bar", v, 4.82426);

    // A reaction with a logK(T) function and the method of 3-term extrapolation: the reaction is extrapolated from the reference values
    // (dG = 10000, dH = 10000, dS = 0, dCp = 0 J/mol), the logK function (logK = -0.5) is not used. G of ZA is -1e5 at 298.15 K
    {
        const double T = 373.15, Tr = 298.15, Cp = 75.0, S = 50.0, G0 = -100000.0;
        const double ZA = G0 - S * (T - Tr) - Cp * (T * std::log(T / Tr) - (T - Tr));
        const double expected = ZA + 10000.0;
        double zp = static_cast<double>(props("ZP", 100., 1.).gibbs_energy);
        check(near(zp, expected, 0.05), "ZP G at 100 C from the reference values of the reaction", zp, expected);
    }

    // Fugacity coefficient: removed from G by default, kept when the preference is false (reference: CO at 25 C, 1000 bar, G = -135869.687 J/mol)
    double gStd = static_cast<double>(props("CO", 25., 1000.).gibbs_energy);
    ThermoEngine engineFug(db);
    engineFug.setSolventSymbol("H2O(l)");
    auto prefs = engineFug.preferences();
    prefs.cancel_fugacity_coefficient_in_gas_props = false;
    engineFug.setPreferences(prefs);
    double P1000 = 1000. * 1e5;
    double gFug = static_cast<double>(engineFug.thermoPropertiesSubstance(298.15, P1000, "CO").gibbs_energy);
    check(near(gStd, -137157.0, 1.5), "CO G with the fugacity term cancelled", gStd, -137157.0);
    check(near(gFug, -135869.687, 1.5), "CO G with the fugacity term kept", gFug, -135869.687);

    printf(failures ? "gems-export: %d failure(s)\n" : "gems-export: all checks passed\n", failures);
    return failures ? 1 : 0;
}
