#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY
#include "ThermoProperties.h"


namespace ThermoFun {

// Forward declarations
class Substance;

/**
 * @brief waterIdealGas
 * @param t temperature (units in K)
 * @param p pressure (units in Pa)
 * @return thermodynamic properties of water in the ideal gas state
 */
auto waterIdealGas (real t, real p) -> ThermoPropertiesSubstanceAD;

}

#endif // WATERIDEALGASWOLLEY

