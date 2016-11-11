#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

auto electroPropertiesWaterSverjensky2014(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROSVERJENSKY2014_H
