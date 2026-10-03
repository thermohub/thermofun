// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef VOLUME_FUNCTION_OF_T
#define VOLUME_FUNCTION_OF_T

#include "Common/Real.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesReaction_Vol_fT(real t, real p, Reaction reaction, ThermoPropertiesReactionAD tpr) -> ThermoPropertiesReactionAD;

}

#endif // VOLUME_FUNCTION_OF_T

