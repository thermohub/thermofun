#ifndef WATERWP95REAKTORO
#define WATERWP95REAKTORO

// Reacktoro includes
#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"
#include "Solvent/Reaktoro/WaterThermoState.hpp"

namespace TCorrPT {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterWP95reaktoro(Reaktoro::Temperature T, /*Reaktoro::Pressure P,*/ const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterWP95reaktoro(const Reaktoro::WaterThermoState& wt) -> PropertiesSolvent;

}

#endif // WATERWP95REAKTORO

