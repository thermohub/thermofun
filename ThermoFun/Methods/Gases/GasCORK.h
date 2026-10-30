// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASCORK
#define GASCORK
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Gases/GasInputs.h>


namespace ThermoFun {

auto thermoPropertiesGasCORK(real t, real p, const GasInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity = true) -> ThermoPropertiesSubstanceAD;

}

#endif // GASCORK

