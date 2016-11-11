#ifndef GASSTP
#define GASSTP

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

auto thermoPropertiesGasSTP(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}


#endif // GASSTP

