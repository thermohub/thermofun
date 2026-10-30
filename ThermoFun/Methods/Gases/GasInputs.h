// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_GASES_GASINPUTS_H
#define THERMOFUN_METHODS_GASES_GASINPUTS_H

#include <string>
#include <vector>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// What the formulas of a fluid need of a record.
struct GasInputs
{
    std::vector<real> parameters;  ///< in the order and the units of the formulas: Tc K, Pc bar, ...
    double lower_T = 298.15;       ///< K, the lower T of the record
    std::string formula;           ///< CORK and Sterner-Pitzer have special forms for CO2 and H2O
};

} // namespace ThermoFun

#endif
