// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Compares the standard properties of the models taken from Reaktoro with the values that Reaktoro calculates
// (tests/reaktoro_models/make_reference.py). A negative tolerance in the file means: the property is not compared.
// The checks are never compiled out (see tests/memoization/src/main.cpp).

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
            continue;
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
    printf(failures ? "reaktoro-models: %d failure(s) in %d rows\n" : "reaktoro-models: all checks passed (%d rows)\n", failures ? failures : rows, rows);
    return failures ? 1 : 0;
}
