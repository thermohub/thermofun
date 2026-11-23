// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef FRANTZMARSHALL_H
#define FRANTZMARSHALL_H

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Methods/Reactions/ReactionInputs.h>

namespace ThermoFun {

auto thermoPropertiesFrantzMarshall(real TK, real Pbar, const ReactionInputs& reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // FRANTZMARSHALL_H

