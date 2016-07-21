#ifndef WATERZHANGDUAN2005
#define WATERZHANGDUAN2005

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"


namespace TCorrPT {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
//auto propertiesWaterZhangDuan2005(const Reaktoro_::WaterThermoState& wt) -> PropertiesSolvent;

}

#endif // WATERZHANGDUAN2005

