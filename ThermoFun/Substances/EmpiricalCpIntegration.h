#ifndef EMPIRICALCPINTEGRATION
#define EMPIRICALCPINTEGRATION
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

/// Returns the temperature correcected themrodynamic properties of a substance uisng the Empicrical Cp integration
/// @ref --
/// @param pass the temperature and pressure of the autodiff pass
/// @param substance substance instance
/// @param outsideBounds set to true if the temperature is outside of the Cp temperature intervals (optional)
auto thermoPropertiesEmpCpIntegration(const Reaktoro_::Pass& pass, Substance substance, bool* outsideBounds = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // EMPIRICALCPINTEGRATION

