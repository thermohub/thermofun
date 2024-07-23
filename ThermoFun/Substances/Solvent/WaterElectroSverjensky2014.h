#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

class Substance;
struct ElectroPropertiesSolvent;

auto electroPropertiesWaterSverjensky2014(Reaktoro_::Temperature TC, Reaktoro_::Pressure Pbar, Substance substance, int state = -1) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROSVERJENSKY2014_H
