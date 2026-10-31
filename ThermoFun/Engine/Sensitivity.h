// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_SENSITIVITY_H
#define THERMOFUN_ENGINE_SENSITIVITY_H

#include <map>
#include <string>

namespace ThermoFun {

/// One parameter of a record. method: its id, for example LandauHP98; empty for a reference value: G, H, S, Cp, V of a substance, logK, dG, dH, dS, dCp, dV of a reaction.
struct ParameterRef
{
    std::string symbol;
    std::string method;
    std::string parameter;

    /// The name in a result: Calcite/LandauHP98/Smax, or Calcite/G
    auto name() const -> std::string { return symbol + "/" + (method.empty() ? "" : method + "/") + parameter; }
};

/// The derivatives: property, then the name of the parameter. SI: the unit of the property per the SI unit of the parameter.
using SensitivityResult = std::map<std::string, std::map<std::string, double>>;

} // namespace ThermoFun

#endif
