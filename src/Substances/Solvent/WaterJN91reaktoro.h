#ifndef WATERJN91REAKTORO
#define WATERJN91REAKTORO


namespace ThermoFun {

struct ElectroPropertiesSolvent;
struct WaterElectroState;

/// Return the electro-chemical properties of water
/// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
auto electroPropertiesWaterJNreaktoro(const WaterElectroState& wts) -> ElectroPropertiesSolvent;

}

#endif // WATERJN91REAKTORO

