// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef OUTPUTWATERSTEAMCONVENTIONPROP
#define OUTPUTWATERSTEAMCONVENTIONPROP

#include <fstream>
#include <string>

#include <ThermoFun/Substances/Solvent/Reaktoro/WaterThermoState.hpp>

namespace ThermoFun {

/// Outputs water proeprties in the steam convention
/// @ref --
/// @param filename - path and name to the output CSV file
/// @param wt structure holding the water proeprties in steam convention
auto OutputSteamConventionH2OProp (std::string filename, const WaterThermoState wt) -> void;

}

#endif // OUTPUTWATERSTEAMCONVENTIONPROP

