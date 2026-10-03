// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERIDEALGASWOLLEY
#define WATERIDEALGASWOLLEY
#include "ThermoProperties.h"


namespace ThermoFun {

// Forward declarations
class Substance;

/**
 * @brief waterIdealGas
 * @param t temperature (units in K)
 * @param p pressure (units in Pa)
 * @return thermodynamic properties of water in the ideal gas state
 */
auto waterIdealGas (real t, real p) -> ThermoPropertiesSubstanceAD;

}

#endif // WATERIDEALGASWOLLEY

