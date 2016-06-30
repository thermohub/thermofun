#ifndef SOLIDBERMAN88
#define SOLIDBERMAN88

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesMinBerman88(Reaktoro::Temperature t, Reaktoro::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDBERMAN88

