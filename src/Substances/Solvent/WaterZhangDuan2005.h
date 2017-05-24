#ifndef WATERZHANGDUAN2005
#define WATERZHANGDUAN2005

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"


namespace ThermoFun {

//// Forward declarations
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ThermoPropertiesSubstance;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> PropertiesSolvent;

auto waterDensityZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> Reaktoro_::ThermoScalar;

}

#endif // WATERZHANGDUAN2005

