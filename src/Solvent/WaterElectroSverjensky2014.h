#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto electroPropertiesWaterSverjensky2014(PropertiesSolvent ps, Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROSVERJENSKY2014_H
