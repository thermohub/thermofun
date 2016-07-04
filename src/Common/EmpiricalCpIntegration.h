#ifndef EMPIRICALCPINTEGRATION
#define EMPIRICALCPINTEGRATION

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

/// Returns the temperature correcected themrodynamic properties of a substance uisng the Empicrical Cp integration
/// @ref --
/// @param T temparature (K)
/// @param P pressure (bar)
/// @param substance substance instance
auto thermoPropertiesEmpCpIntegration(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance substance) -> ThermoPropertiesSubstance;

}

#endif // EMPIRICALCPINTEGRATION

