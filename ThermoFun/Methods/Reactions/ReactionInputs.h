// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_REACTIONS_REACTIONINPUTS_H
#define THERMOFUN_METHODS_REACTIONS_REACTIONINPUTS_H

#include <vector>

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

/// What the formulas of a reaction need of a record.
struct ReactionInputs
{
    ThermoPropertiesReaction reference;  ///< the reference properties, with their status and errors. V in J/bar
    double Tr = 298.15;                  ///< K
    double Pr = 1e5;                     ///< Pa
    std::vector<real> coefficients;      ///< of the method, in the units of its formulas
    std::vector<real> cp_coefficients;   ///< of dCp(T), for the polynomial of log K
};

/// The forms of log K(T).
enum class LogKForm
{
    Constant,      ///< log K is the reference value
    ZeroEntropy,   ///< dS = 0, dH = dG
    VantHoff,      ///< dH and dS do not change with T
    ConstantCp,    ///< dCp does not change with T
    Polynomial     ///< the 7-term polynomial, or the integration of dCp(T)
};

} // namespace ThermoFun

#endif
