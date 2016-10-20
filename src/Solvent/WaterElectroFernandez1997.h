#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto electroPropertiesWaterFernandez1997(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ElectroPropertiesSolvent;

}

#endif // WATERELECTROFERNANDEZ1997

