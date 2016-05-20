#ifndef WATERJN91REAKTORO
#define WATERJN91REAKTORO

#include "Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp"
#include "Solvent/Reaktoro/WaterElectroState.hpp"

namespace TCorrPT {

/// Return the electro-chemical properties of water
/// @param wts instance of the strcuture holding the calculated electro-chemical properties of water
auto electroPropertiesWaterJNreaktoro(const Reaktoro::WaterElectroState& wts) -> ElectroPropertiesSolvent;

}

#endif // WATERJN91REAKTORO

