#ifndef WATERHGKREAKTORO
#define WATERHGKREAKTORO

#include "Common/ScalarTypes.hpp"
#include "Common/ThermoScalar.hpp"

namespace ThermoFun {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;
struct WaterThermoState;

//auto propertiesSolvent(double T, double P) -> PropertiesSolvent;

//auto thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent;

//auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

//auto waterSaturatedPressureWagnerPruss(Reaktoro::Temperature T) -> Reaktoro::ThermoScalar;

/// Calculate the water saturated vapor pressure using the HGK model (from GEMS)
/// @param t temperature (K)
auto saturatedWaterVaporPressureHGK(Reaktoro_::Temperature TK) -> Reaktoro_::ThermoScalar;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterHGKreaktoro(Reaktoro_::Temperature T, /*Reaktoro::Pressure P,*/ const WaterThermoState& wt) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterHGKreaktoro(const WaterThermoState& wt) -> PropertiesSolvent;

///// Return the electro-chemical properties of water
///// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
//auto electroPropertiesWaterJNreaktoro(const Reaktoro::WaterElectroState& wts) -> ElectroPropertiesSolvent;
}

#endif // WATERHGKREAKTORO

