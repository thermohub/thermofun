#ifndef STANDARDENTROPYCPINTEGRATION
#define STANDARDENTROPYCPINTEGRATION

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

class Substance;
struct ThermoPropertiesSubstance;

/// Returns the temperature corrected themrodynamic properties of a substance uisng standard entropy and constant heat capacity integration
/// @ref --
/// @param T temparature (K)
/// @param P pressure, not used
/// @param substance substance instance
auto thermoPropertiesEntropyCpIntegration(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ThermoPropertiesSubstance;

}

#endif // STANDARDENTROPYCPINTEGRATION

