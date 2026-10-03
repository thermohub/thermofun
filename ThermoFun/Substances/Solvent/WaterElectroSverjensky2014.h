#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto electroPropertiesWaterSverjensky2014(const Reaktoro_::Pass& pass, Substance substance, int state = -1) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROSVERJENSKY2014_H
