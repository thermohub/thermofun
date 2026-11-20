// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef GASPRSV
#define GASPRSV
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Fluids/FluidInputs.h>


namespace ThermoFun {


auto thermoPropertiesFluidPRSV(real t, real p, const FluidInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity = true) -> ThermoPropertiesSubstanceAD;

}


#endif // GASPRSV
