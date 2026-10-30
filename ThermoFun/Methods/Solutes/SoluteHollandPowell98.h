// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLUTEHOLLANDPOWELL98_H
#define SOLUTEHOLLANDPOWELL98_H
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Solutes/SoluteInputs.h>


namespace ThermoFun {

auto thermoPropertiesAqSoluteHP98(real TK, real Pbar, const SoluteReference& reference, const std::vector<real>& parameters, const std::string& symbol, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEHOLLANDPOWELL98_H
