// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

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
/// The properties of water with the exact derivatives of the density up to the third order (the pressure of the pass in Pa)
auto propertiesWaterZhangDuan2005(const Reaktoro_::Pass& pass) -> PropertiesSolventAD;

auto waterDensityZhangDuan2005(real T, real P) -> real;

}

#endif // WATERZHANGDUAN2005

