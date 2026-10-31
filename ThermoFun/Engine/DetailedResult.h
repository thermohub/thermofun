// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_DETAILEDRESULT_H
#define THERMOFUN_ENGINE_DETAILEDRESULT_H

#include <string>
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

/// One method of a substance and the properties after it, in SI: G, H [J/mol], S, Cp [J/(mol K)], V [m3/mol].
struct ExplainStep
{
    std::string method;
    double G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
};

/// The result of a calculation and how it came.
struct DetailedPropertiesSubstance
{
    ThermoPropertiesSubstance properties;
    /// "methods", "water", "reaction" (the defining reaction), "reference" (the reference properties), "hydrogen_ion", or
    /// "none" (no method ran)
    std::string source;
    std::string reaction;              ///< the defining reaction, for the source "reaction"
    std::vector<ExplainStep> steps;    ///< the methods in the order they ran, for the source "methods"
    std::vector<std::string> conversions;   ///< "<method> to canonical", and the words of the output convention: "steam_tables", "berman_brown", ...
};

} // namespace ThermoFun

#endif
