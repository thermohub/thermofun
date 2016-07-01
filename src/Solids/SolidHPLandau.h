#ifndef SOLIDHPLANDAU
#define SOLIDHPLANDAU

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesHPLandau(Reaktoro::Temperature t, Reaktoro::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;


}

#endif // SOLIDHPLANDAU

