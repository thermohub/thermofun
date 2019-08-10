#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

// Forward declarations
class Substance;
struct ThermoPropertiesSubstance;

/**
 * @brief waterIdealGas
 * @param t temperature (units in K)
 * @param p pressure (units in Pa)
 * @return thermodynamic properties of water in the ideal gas state
 */
auto waterIdealGas (Reaktoro_::Temperature t, Reaktoro_::Pressure p) -> ThermoPropertiesSubstance;

}

#endif // WATERIDEALGASWOLLEY

