// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASCORK
#define GASCORK
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {
class Substance;

auto thermoPropertiesGasCORK(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASCORK

