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

//auto waterSaturatedPressureWagnerPruss(Reaktoro::Temperature T) -> Reaktoro::ThermoScalar;

auto saturatedWaterVaporPressureHGK(double t) -> double;

auto thermoPropertiesWaterHGKreaktoro(Reaktoro::Temperature T, Reaktoro::Pressure P, const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance;

auto propertiesWaterHGKreaktoro(Reaktoro::Temperature T, Reaktoro::Pressure P, const Reaktoro::WaterThermoState& wt) -> PropertiesSolvent;

}

#endif // WATERHGKREAKTORO

