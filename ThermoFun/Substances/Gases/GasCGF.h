// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASCGF
#define GASCGF
#include "ThermoProperties.h"


namespace ThermoFun {
class Substance;

auto thermoPropertiesGasCGF(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASCGF

