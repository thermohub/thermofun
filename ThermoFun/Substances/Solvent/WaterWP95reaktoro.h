#ifndef WATERWP95REAKTORO
#define WATERWP95REAKTORO
#include "ThermoProperties.h"


namespace ThermoFun {

//// Forward declarations
struct WaterThermoState;
struct WaterTripleProperties;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterWP95reaktoro(real T, /*Reaktoro::Pressure P,*/ const WaterThermoState& wt, const WaterTripleProperties& wat) -> ThermoPropertiesSubstanceAD;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterWP95reaktoro(const WaterThermoState& wt) -> PropertiesSolventAD;

}

#endif // WATERWP95REAKTORO

