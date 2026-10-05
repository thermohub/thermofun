// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef RYZHENKOBRYZGALYN
#define RYZHENKOBRYZGALYN

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/ThermoParameters.h>

namespace ThermoFun {

auto thermoPropertiesRyzhenkoBryzgalin(real TK, real Pbar, Reaction reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // RYZHENKOBRYZGALYN

