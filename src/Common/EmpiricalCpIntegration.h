#ifndef EMPIRICALCPINTEGRATION
#define EMPIRICALCPINTEGRATION

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

auto thermoPropertiesEmpCpIntegration(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance substance) -> ThermoPropertiesSubstance;

}

#endif // EMPIRICALCPINTEGRATION

