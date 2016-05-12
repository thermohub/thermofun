#ifndef WATERHGKREAKTORO
#define WATERHGKREAKTORO

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"
#include "Solvent/Reaktoro/WaterThermoState.hpp"

namespace TCorrPT {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesSolvent;

//auto propertiesSolvent(double T, double P) -> PropertiesSolvent;

//auto thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent;

//auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

auto thermoPropertiesWaterHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance;

}

#endif // WATERHGKREAKTORO

