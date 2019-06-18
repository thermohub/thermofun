#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ElectroPropertiesSolvent;

auto electroPropertiesWaterSverjensky2014(Reaktoro_::Temperature TC, Reaktoro_::Pressure Pbar, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROSVERJENSKY2014_H
