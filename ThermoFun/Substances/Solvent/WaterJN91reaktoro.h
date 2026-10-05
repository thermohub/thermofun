#ifndef WATERJN91REAKTORO
#define WATERJN91REAKTORO
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {

struct WaterElectroState;

/// Return the electro-chemical properties of water
/// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
auto electroPropertiesWaterJNreaktoro(const WaterElectroState& wts) -> ElectroPropertiesSolventAD;

}

#endif // WATERJN91REAKTORO

