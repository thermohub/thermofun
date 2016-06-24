#ifndef SOLIDBMGOTTSCHALK
#define SOLIDBMGOTTSCHALK

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesMinBMGottschalk (Reaktoro::Temperature t, Reaktoro::Pressure p, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDBMGOTTSCHALK

