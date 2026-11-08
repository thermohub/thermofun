// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_FLUIDS_FLUIDINPUTS_H
#define THERMOFUN_METHODS_FLUIDS_FLUIDINPUTS_H

#include <string>
#include <vector>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// What the formulas of a fluid need of a record.
struct FluidInputs
{
    std::vector<real> parameters;  ///< in the order and the units of the formulas: Tc K, Pc bar, ...
    double lower_T = 298.15;       ///< K, the lower T of the record
    std::string formula;           ///< CORK and Sterner-Pitzer have special forms for CO2 and H2O
    std::vector<double> constants; ///< of the model, in the order of the catalog
    bool gems = false;             ///< the formulas of GEMS: no residual Cp, and the H and S that are not -T^2 d(G/T)/dT and -dG/dT (implementation gems)
};

} // namespace ThermoFun

#endif
