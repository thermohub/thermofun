// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef VOLUME_FUNCTION_OF_T
#define VOLUME_FUNCTION_OF_T

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Methods/Reactions/ReactionInputs.h>

namespace ThermoFun {

auto thermoPropertiesReaction_Vol_fT(real t, real p, const ReactionInputs& reaction, ThermoPropertiesReactionAD tpr) -> ThermoPropertiesReactionAD;

}

#endif // VOLUME_FUNCTION_OF_T

