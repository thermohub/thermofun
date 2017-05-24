#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

// Forward declarations
struct PropertiesSolvent;

auto waterIdealGas (Reaktoro_::Temperature t, Reaktoro_::Pressure p) -> ThermoPropertiesSubstance;

}

#endif // WATERIDEALGASWOLLEY

