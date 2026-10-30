// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_SOLUTES_SOLUTEINPUTS_H
#define THERMOFUN_METHODS_SOLUTES_SOLUTEINPUTS_H

#include <string>
#include <vector>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// What the HKF formulas need of a species. The parameters a1, a2, a3, a4, c1, c2, omega are in the units of the
/// model (cal, bar); the reference values in J/mol and J/(mol K). No parameters: the record has none.
struct HKFInputs
{
    std::string symbol;
    std::vector<real> parameters;
    double G = 0.0, H = 0.0, S = 0.0;
    double charge = 0.0;
    bool hydrogen_ion = false;
};

/// The reference values of a species for the density models. K, J/mol, J/(mol K), J/bar.
struct SoluteReference
{
    double T = 298.15, G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
};

} // namespace ThermoFun

#endif
