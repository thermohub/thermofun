#ifndef EMPIRICALCPINTEGRATION
#define EMPIRICALCPINTEGRATION

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

class Substance;
struct ThermoPropertiesSubstance;

/// Returns the temperature correcected themrodynamic properties of a substance uisng the Empicrical Cp integration
/// @ref --
/// @param T temparature (K)
/// @param P pressure (bar)
/// @param substance substance instance
auto thermoPropertiesEmpCpIntegration(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ThermoPropertiesSubstance;

}

#endif // EMPIRICALCPINTEGRATION

