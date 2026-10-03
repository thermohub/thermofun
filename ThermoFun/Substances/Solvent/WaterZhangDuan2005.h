#ifndef WATERZHANGDUAN2005
#define WATERZHANGDUAN2005

#include "Common/Real.hpp"
#include "ThermoProperties.h"

namespace ThermoFun {

//// Forward declarations

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterZhangDuan2005(real T, real P) -> ThermoPropertiesSubstanceAD;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto propertiesWaterZhangDuan2005(real T, real P) -> PropertiesSolventAD;

auto waterDensityZhangDuan2005(real T, real P) -> real;

}

#endif // WATERZHANGDUAN2005

