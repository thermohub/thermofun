// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLUTEHOLLANDPOWELL98_H
#define SOLUTEHOLLANDPOWELL98_H
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {
class Substance;

auto thermoPropertiesAqSoluteHP98(real TK, real Pbar, Substance subst, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEHOLLANDPOWELL98_H
