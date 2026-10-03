#ifndef STANDARDENTROPYCPINTEGRATION
#define STANDARDENTROPYCPINTEGRATION
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

/// Returns the temperature corrected themrodynamic properties of a substance uisng standard entropy and constant heat capacity integration
/// @ref --
/// @param T temparature (K)
/// @param P pressure, not used
/// @param substance substance instance
auto thermoPropertiesEntropyCpIntegration(real T, real P, Substance substance) -> ThermoPropertiesSubstanceAD;

}

#endif // STANDARDENTROPYCPINTEGRATION

