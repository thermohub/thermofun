#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ElectroPropertiesSolvent;

auto electroPropertiesWaterFernandez1997(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROFERNANDEZ1997

