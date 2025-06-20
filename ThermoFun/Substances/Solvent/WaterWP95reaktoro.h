#ifndef WATERWP95REAKTORO
#define WATERWP95REAKTORO

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;
struct WaterThermoState;
struct WaterTripleProperties;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterWP95reaktoro(Reaktoro_::Temperature T, /*Reaktoro::Pressure P,*/ const WaterThermoState& wt, const WaterTripleProperties& wat) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterWP95reaktoro(const WaterThermoState& wt) -> PropertiesSolvent;

}

#endif // WATERWP95REAKTORO

