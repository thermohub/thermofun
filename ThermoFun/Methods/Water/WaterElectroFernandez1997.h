// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>


namespace ThermoFun {



auto electroPropertiesWaterFernandez1997(const Reaktoro_::Pass& pass, const PropertiesSolvent& solvent, const std::vector<double>& constants) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROFERNANDEZ1997

