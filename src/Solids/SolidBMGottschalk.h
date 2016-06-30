#ifndef SOLIDBMGOTTSCHALK
#define SOLIDBMGOTTSCHALK

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesMinBMGottschalk (Reaktoro::Temperature t, Reaktoro::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDBMGOTTSCHALK

