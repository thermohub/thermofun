#ifndef GASPRSV
#define GASPRSV

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;

auto thermoPropertiesGasPRSV(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}


#endif // GASPRSV
