#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

// Forward declarations
struct Substance;
struct ThermoPropertiesSubstance;

auto waterIdealGas (Reaktoro_::Temperature t, Reaktoro_::Pressure p) -> ThermoPropertiesSubstance;

}

#endif // WATERIDEALGASWOLLEY

