#ifndef WATERHGKREAKTORO
#define WATERHGKREAKTORO

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/GlobalVariables.h>

namespace ThermoFun {

//// Forward declarations
struct WaterThermoState;
struct WaterTripleProperties;

//auto propertiesSolvent(double T, double P) -> PropertiesSolventAD;

//auto thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent;

//auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstanceAD;

//auto waterSaturatedPressureWagnerPruss(Reaktoro::Temperature T) -> Reaktoro::ThermoScalar;

/// Calculate the water saturated vapor pressure using the HGK model (from GEMS)
/// @param t temperature (K)
auto saturatedWaterVaporPressureHGK(real TK) -> real;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterHGKreaktoro(real T, /*Reaktoro::Pressure P,*/ const WaterThermoState& wt, const WaterTripleProperties& wtr) -> ThermoPropertiesSubstanceAD;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterHGKreaktoro(const WaterThermoState& wt) -> PropertiesSolventAD;

///// Return the electro-chemical properties of water
///// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
//auto electroPropertiesWaterJNreaktoro(const Reaktoro::WaterElectroState& wts) -> ElectroPropertiesSolvent;
}

#endif // WATERHGKREAKTORO

