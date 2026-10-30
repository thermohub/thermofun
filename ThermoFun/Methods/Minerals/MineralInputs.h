// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_MINERALINPUTS_H
#define THERMOFUN_METHODS_MINERALS_MINERALINPUTS_H

#include <map>
#include <string>
#include <vector>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// What the formulas of a mineral or a standard model need of a record.
struct MineralInputs
{
    double Tr = 298.15;              ///< K
    double Pr = 1e5;                 ///< Pa
    double G = 0.0, H = 0.0, S = 0.0, Cp = 0.0;  ///< J/mol, J/(mol K)
    double V = 0.0;                  ///< J/bar
    real alpha0 = 0.0;               ///< thermal expansion, 1/K
    real kappa0 = 0.0;               ///< bulk modulus, kbar
    std::vector<real> coefficients;  ///< of the method, in the units of its formulas
};

/// NASA polynomials: one row a1..a7, b1, b2 for each T interval.
struct NasaInputs
{
    std::vector<std::vector<double>> rows, intervals;
    double V = 0.0;  ///< J/bar
};

/// A table in T (K) and P (Pa): value[i + j nT] for temperature i and pressure j. Volume in m3/mol.
struct TableInputs
{
    std::vector<double> temperatures, pressures;
    std::map<std::string, std::vector<double>> values;
    double reference_pressure = 0.0;  ///< Pa, the pressure of the values of G (0: 1e5)
    double V = 0.0;                   ///< J/bar, the reference volume
};

} // namespace ThermoFun

#endif
