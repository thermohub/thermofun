// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLIDMURNAGHANHP98_H
#define SOLIDMURNAGHANHP98_H
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {

class Substance;

/// Returns the  correcected themrodynamic properties of a substance (mineral) uisng the Murnaghan EoS (Holland and Powell, 1998)
/// @ref Holland T.J.B., Powell R. (1998) An internally consistent thermodynamic data set for phases of
/// petrological interest. Journal of Metamorphic Geology, 16, 309-343.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
auto thermoPropertiesMinMurnaghanEOSHP98(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps, bool* variableVolume = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLIDMURNAGHANHP98_H
