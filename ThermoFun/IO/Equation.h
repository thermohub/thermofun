// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_EQUATION_H
#define THERMOFUN_IO_EQUATION_H

#include <string>
#include <utility>
#include <vector>

namespace ThermoFun::v2 {

/// The symbols and coefficients of "A + 2 B = C": below 0 on the left, above 0 on the right, in the order of the equation.
auto stoichiometryOfEquation(const std::string& equation) -> std::vector<std::pair<std::string, double>>;

} // namespace ThermoFun::v2

#endif
