// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLUTEANDERSON91_H
#define SOLUTEANDERSON91_H
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Solutes/SoluteInputs.h>


namespace ThermoFun {

auto thermoPropertiesAqSoluteAN91(real TK, real Pbar, const SoluteReference& reference, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEANDERSON91_H
