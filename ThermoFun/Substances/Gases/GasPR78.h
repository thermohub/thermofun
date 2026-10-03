// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASPR78
#define GASPR78
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto thermoPropertiesGasPR78(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASPR78

