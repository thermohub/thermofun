// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLIDBERMAN88
#define SOLIDBERMAN88
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {

class Substance;

/// Returns the  correcected themrodynamic properties of a substance (mineral) uisng the Berman crrection model
/// @ref Berman R.G. (1988) Internally consistent thermodynamic data for minerals in the system Na2O-K2O-
/// CaO-MgO-FeO-Fe2O3-Al2O3-SiO2-TiO2-H2O-CO2. Journal of Petrology, 29, 445-522.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
auto thermoPropertiesMinBerman88(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps, bool* applied = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLIDBERMAN88

