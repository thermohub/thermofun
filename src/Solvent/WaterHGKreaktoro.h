#ifndef WATERHGKREAKTORO
#define WATERHGKREAKTORO

// Reacktoro includes
#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"
#include "Solvent/Reaktoro/WaterThermoState.hpp"
#include "Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp"

namespace TCorrPT {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

//auto propertiesSolvent(double T, double P) -> PropertiesSolvent;

//auto thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent;

//auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

//auto waterSaturatedPressureWagnerPruss(Reaktoro::Temperature T) -> Reaktoro::ThermoScalar;

/// Calculate the water saturated vapor pressure using the HGK model (from GEMS)
/// @param t temperature (K)
auto saturatedWaterVaporPressureHGK(double t) -> double;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterHGKreaktoro(Reaktoro::Temperature T, /*Reaktoro::Pressure P,*/ const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterHGKreaktoro(const Reaktoro::WaterThermoState& wt) -> PropertiesSolvent;

/// Return the electro-chemical properties of water
/// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
auto electroPropertiesWaterJNreaktoro(const Reaktoro::WaterElectroState& wts) -> ElectroPropertiesSolvent;

}

#endif // WATERHGKREAKTORO

