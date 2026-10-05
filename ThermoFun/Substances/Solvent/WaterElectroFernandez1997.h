// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERELECTROFERNANDEZ1997
#define WATERELECTROFERNANDEZ1997
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {

class Substance;


auto electroPropertiesWaterFernandez1997(const Reaktoro_::Pass& pass, Substance substance, int state =-1) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROFERNANDEZ1997

