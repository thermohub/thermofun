#ifndef OUTPUTWATERSTEAMCONVENTIONPROP
#define OUTPUTWATERSTEAMCONVENTIONPROP

#include <fstream>
#include <string>

#include "Solvent/Reaktoro/WaterThermoState.hpp"

namespace TCorrPT {

/// Outputs water proeprties in the steam convention
/// @ref --
/// @param filename - path and name to the output CSV file
/// @param wt structure holding the water proeprties in steam convention
auto OutputSteamConventionH2OProp (std::string filename, const Reaktoro::WaterThermoState wt) -> void;

}

#endif // OUTPUTWATERSTEAMCONVENTIONPROP

