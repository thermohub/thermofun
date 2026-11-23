// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef WATERELECTROBRADLEYPITZER79_H
#define WATERELECTROBRADLEYPITZER79_H
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

auto electroPropertiesWaterBradleyPitzer79(const Reaktoro_::Pass& pass, const std::vector<double>& constants) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROBRADLEYPITZER79_H
