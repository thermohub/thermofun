#ifndef OUTPUTWATERSTEAMCONVENTIONPROP
#define OUTPUTWATERSTEAMCONVENTIONPROP

#include <fstream>
#include <string>

#include "Solvent/Reaktoro/WaterThermoState.hpp"

namespace TCorrPT {

auto OutputSteamConventionH2OProp (std::string filename, const Reaktoro::WaterThermoState wt) -> void;

}

#endif // OUTPUTWATERSTEAMCONVENTIONPROP

