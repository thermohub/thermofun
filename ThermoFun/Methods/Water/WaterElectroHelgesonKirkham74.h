// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef WATERELECTROHELGESONKIRKHAM74_H
#define WATERELECTROHELGESONKIRKHAM74_H
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

auto electroPropertiesWaterHelgesonKirkham74(const Reaktoro_::Pass& pass, const PropertiesSolvent& solvent, const std::vector<double>& constants) -> ElectroPropertiesSolventAD;

}

#endif // WATERELECTROHELGESONKIRKHAM74_H
