#ifndef GASPR78
#define GASPR78

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

auto thermoPropertiesGasPR78(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASPR78

