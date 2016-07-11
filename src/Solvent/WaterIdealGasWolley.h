#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

// Forward declarations
struct PropertiesSolvent;

auto waterIdealGas (Reaktoro_::Temperature t, Reaktoro_::Pressure p) -> ThermoPropertiesSubstance;

}

#endif // WATERIDEALGASWOLLEY

