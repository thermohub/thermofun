// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Compares the standard properties of the extra models with the reference values
// (tests/extra_models/extra_reference.csv). A negative tolerance in the file means that the test does not compare the property.
// The checks stay active in every build type.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ThermoFun/ThermoFun.h"

using namespace ThermoFun;

int main(int argc, char** argv)
{
    if (argc < 3)
        return 2;
    Database db(argv[1]);
    ThermoEngine engine(db);
    std::ifstream in(argv[2]);
    if (!in)
    {
        printf("extra-models: cannot open %s\n", argv[2]);
        return 2;
    }
    std::string line;
    std::getline(in, line); // header
    int failures = 0, rows = 0;
    while (std::getline(in, line))
    {
        std::stringstream ss(line);
        std::string sym, cell;
        std::getline(ss, sym, ',');
        std::vector<double> v;
        while (std::getline(ss, cell, ','))
            v.push_back(std::stod(cell));
        if (v.size() != 12)
        {
            ++failures;
            printf("  FAIL malformed row for %s: %zu numeric columns, expected 12\n", sym.c_str(), v.size());
            continue;
        }
        const double T = v[0], Pbar = v[1];
        double P = Pbar * 1e5;
        ThermoPropertiesSubstance r = engine.thermoPropertiesSubstance(T, P, sym);
        const double got[5] = {static_cast<double>(r.gibbs_energy), static_cast<double>(r.enthalpy), static_cast<double>(r.entropy),
                               static_cast<double>(r.heat_capacity_cp), static_cast<double>(r.volume)};
        const char* names[5] = {"G", "H", "S", "Cp", "V"};
        ++rows;
        for (int k = 0; k < 5; ++k)
        {
            const double expected = v[2 + k], tol = v[7 + k];
            if (tol < 0.0)
                continue;
            if (!(std::fabs(got[k] - expected) <= tol))
            {
                ++failures;
                printf("  FAIL %s %s at %g K, %g bar: %.10g, expected %.10g (tolerance %g)\n", sym.c_str(), names[k], T, Pbar, got[k], expected, tol);
            }
        }
    }
    if (rows == 0)
        failures++;

    // the engine takes T and P by reference
    auto props = [&](double T, double P, const char* symbol) { return engine.thermoPropertiesSubstance(T, P, symbol); };

    // a table with only the Gibbs energy: the other properties stay unset, the volume is the reference volume
    {
        ThermoPropertiesSubstance r = props(350.0, 1e7, "IG");
        const double Vref = 2.0; // J/bar
        const double Gtab = -141502.5; // the table at 350 K, 100 bar (no volume term)
        const bool unset = r.enthalpy.sta.first != Reaktoro_::Status::calculated && r.entropy.sta.first != Reaktoro_::Status::calculated &&
                           r.heat_capacity_cp.sta.first != Reaktoro_::Status::calculated;
        const double Gexp = Gtab + Vref * (100.0 - 1.0); // volume term, reference pressure 1 bar
        ++rows;
        if (!unset || std::fabs(static_cast<double>(r.volume) - Vref) > 1e-9 || std::fabs(static_cast<double>(r.gibbs_energy) - Gexp) > 1e-6)
        {
            ++failures;
            printf("  FAIL table with only G: V %.10g (expected %g), G %.10g (expected %.10g), H, S, Cp unset: %d\n", static_cast<double>(r.volume), Vref,
                   static_cast<double>(r.gibbs_energy), Gexp, unset);
        }
    }

    // below the table the entropy is the one at the edge of the table
    {
        const double Sin = static_cast<double>(props(300.0, 1e5, "IT").entropy);
        const double Sout = static_cast<double>(props(250.0, 1e5, "IT").entropy);
        ++rows;
        if (!(std::fabs(Sin - Sout) <= 1e-9))
        {
            ++failures;
            printf("  FAIL table entropy below the table: %.10g, expected %.10g\n", Sout, Sin);
        }
    }

    // Extended UNIQUAC below Theta is refused, not returned as NaN
    {
        bool refused = false;
        try { props(190.0, 1e5, "UQ"); }
        catch (const std::exception&) { refused = true; }
        ++rows;
        if (!refused)
        {
            ++failures;
            printf("  FAIL Extended UNIQUAC at 190 K (Theta 200 K) was not refused\n");
        }
    }
    printf(failures ? "extra-models: %d failure(s) in %d rows\n" : "extra-models: all checks passed (%d rows)\n", failures ? failures : rows, rows);
    return failures ? 1 : 0;
}
