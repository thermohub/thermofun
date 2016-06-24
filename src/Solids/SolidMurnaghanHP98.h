#ifndef SOLIDMURNAGHANHP98_H
#define SOLIDMURNAGHANHP98_H

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesMinMurnaghanEOSHP98(Reaktoro::Temperature t, Reaktoro::Pressure p, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDMURNAGHANHP98_H
