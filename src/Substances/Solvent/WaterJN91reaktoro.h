#ifndef WATERJN91REAKTORO
#define WATERJN91REAKTORO

#include "Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp"
#include "Substances/Solvent/Reaktoro/WaterElectroState.hpp"

namespace ThermoFun {

/// Return the electro-chemical properties of water
/// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
auto electroPropertiesWaterJNreaktoro(const Reaktoro_::WaterElectroState& wts) -> ElectroPropertiesSolvent;

}

#endif // WATERJN91REAKTORO

