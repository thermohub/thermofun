// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef SOLIDLANDAUHP11
#define SOLIDLANDAUHP11
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Minerals/MineralInputs.h>

namespace ThermoFun {


/// Adds the Landau term of Holland and Powell (2011) to the properties of a mineral. The term is zero at the reference T and P.
/// The parameters are Tc0 (degC in the legacy array), Smax (J/(mol K)) and Vmax (J/bar) in the Landau transition properties of the substance.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param tps properties of the substance, previously corrected with other models
auto thermoPropertiesHP11Landau(real t, real p, const MineralInputs& subst, ThermoPropertiesSubstanceAD tps, bool* subcritical = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif
