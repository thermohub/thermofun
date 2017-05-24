#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

auto electroPropertiesWaterFernandez1997(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROFERNANDEZ1997

