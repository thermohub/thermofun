#ifndef WATERHGKREAKTORO
#define WATERHGKREAKTORO

namespace TCorrPT {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesSolvent;

auto propertiesWaterHGKreaktoro(double T, double P) -> PropertiesSolvent;

auto thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent;

auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

}

#endif // WATERHGKREAKTORO

