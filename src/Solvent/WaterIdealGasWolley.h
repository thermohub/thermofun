#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

// Forward declarations
struct PropertiesSolvent;

auto waterIdealGas (Reaktoro::Temperature t, Reaktoro::Pressure p) -> ThermoPropertiesSubstance;

}

#endif // WATERIDEALGASWOLLEY

