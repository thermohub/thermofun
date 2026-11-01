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
/// The constants of the HKF model and of its g function, as in Methods/ParameterSets/HKF88.yaml. The order is that of the catalog.
struct HKFConstants
{
    double eta, theta, psi, Tr, Pr, Zr, Yr, zeta, gr, ag1, ag2, ag3, bg1, bg2, bg3, af1, af2, af3, Dmax;
};

struct HKFInputs
{
    std::string symbol;
    std::vector<real> parameters;
    real G = 0.0, H = 0.0, S = 0.0;
    double charge = 0.0;
    bool hydrogen_ion = false;
    HKFConstants constants{};
};

/// The reference values of a species for the density models. K, J/mol, J/(mol K), J/bar.
struct SoluteReference
{
    double T = 298.15;
    real G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
};

} // namespace ThermoFun

#endif
