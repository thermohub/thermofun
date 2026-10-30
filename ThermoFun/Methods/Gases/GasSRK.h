// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASSRK
#define GASSRK
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Gases/GasInputs.h>


namespace ThermoFun {


auto thermoPropertiesGasSRK(real t, real p, const GasInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity = true) -> ThermoPropertiesSubstanceAD;

}

#endif // GASSRK

