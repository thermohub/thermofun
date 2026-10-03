// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef RYZHENKOBRYZGALYN
#define RYZHENKOBRYZGALYN

#include "Common/Real.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesRyzhenkoBryzgalin(real TK, real Pbar, Reaction reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // RYZHENKOBRYZGALYN

