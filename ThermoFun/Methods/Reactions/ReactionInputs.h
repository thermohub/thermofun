// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_REACTIONS_REACTIONINPUTS_H
#define THERMOFUN_METHODS_REACTIONS_REACTIONINPUTS_H

#include <vector>

#include <ThermoFun/Common/Real.hpp>
#include <string>
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

/// One coefficient of a reaction method: its name, and the factor from SI to the units of the formula.
struct CoefficientUnit
{
    std::string name;
    double factor = 1.0;
};

/// The derivatives of the reference values in a pass by one parameter: 0 for the others. V: of J/bar by m3/mol.
struct ReferenceSeed
{
    double logK = 0.0, dG = 0.0, dH = 0.0, dS = 0.0, dCp = 0.0, dV = 0.0;
};

/// A reference value as a number of a pass by a parameter.
inline auto seeded(const Reaktoro_::ThermoProperty& x, double seed) -> real
{
    real out(x.val());
    if (seed != 0.0)
        out[1] = seed;
    return out;
}

/// What the formulas of a reaction need of a record.
struct ReactionInputs
{
    std::vector<double> constants;  ///< of the method, in the order of the catalog
    ThermoPropertiesReaction reference;  ///< the reference properties, with their status and errors. V in J/bar
    double Tr = 298.15;                  ///< K
    double Pr = 1e5;                     ///< Pa
    std::vector<real> coefficients;      ///< of the method, in the units of its formulas
    std::vector<real> cp_coefficients;   ///< of dCp(T), for the polynomial of log K
    std::vector<CoefficientUnit> coefficient_units, cp_coefficient_units;  ///< of the two lists; set if a record made them
    ReferenceSeed seed;                  ///< set only in a pass by a parameter
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
