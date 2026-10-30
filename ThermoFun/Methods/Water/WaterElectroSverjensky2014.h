// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERELECTROSVERJENSKY2014_H
#define WATERELECTROSVERJENSKY2014_H
#include <ThermoFun/Common/ThermoProperties.h>


namespace ThermoFun {


auto electroPropertiesWaterSverjensky2014(const Reaktoro_::Pass& pass, const PropertiesSolvent& solvent) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROSVERJENSKY2014_H
