// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef FRANTZMARSHALL_H
#define FRANTZMARSHALL_H

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

auto thermoPropertiesFrantzMarshall(real TK, real Pbar, Reaction reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // FRANTZMARSHALL_H

