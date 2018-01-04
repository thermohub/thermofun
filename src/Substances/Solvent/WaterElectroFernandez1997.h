#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ElectroPropertiesSolvent;

auto electroPropertiesWaterFernandez1997(Reaktoro_::Temperature TC, Reaktoro_::Pressure Pbar, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROFERNANDEZ1997

