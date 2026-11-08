// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLUTEHOLLANDPOWELL98_H
#define SOLUTEHOLLANDPOWELL98_H
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Solutes/SoluteInputs.h>


namespace ThermoFun {

auto thermoPropertiesAqSoluteHP98(real TK, real Pbar, const SoluteReference& reference, const std::vector<real>& parameters, const std::string& symbol, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

/// The formulas as published by Holland and Powell (1998), as ThermoFun had them before 1.0: T' is a constant, and S, Cp
/// and H are not the derivatives of G (the sign of the ln(rho/rho298)/T' term of S, 2 alpha/T' missing in Cp, H = G + T S298
/// in the Berman-Brown form). G and V are the same as in thermoPropertiesAqSoluteHP98. For comparison with the paper.
auto thermoPropertiesAqSoluteHP98Published(real TK, real Pbar, const SoluteReference& reference, const std::vector<real>& parameters, const std::string& symbol, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEHOLLANDPOWELL98_H
