// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_CPPOLYNOMIAL_H
#define THERMOFUN_METHODS_MINERALS_CPPOLYNOMIAL_H

#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/MethodApplication.h>

namespace ThermoFun::v2 {

/// A phase transition at the start of a T segment. K, J/(mol K), J/mol, m3/mol.
struct PhaseTransition
{
    double T = 0.0, dS = 0.0, dH = 0.0, dV = 0.0;
};

/// Integrates Cp = a0 + a1 T + a2/T2 + a3/sqrt(T) + a4 T2 + a5 T3 + a6 T4 + a7/T3 + a8/T + a9 sqrt(T) over T segments,
/// with phase transitions between them. Without coefficients, Cp is the constant reference value.
/// intervals: {lower, upper} of each segment, K. coefficients: a0 to a9 of the segments that have them, one segment
/// after the other. name: the symbol of the record, for the messages.
auto cpPolynomialMethod(std::vector<std::pair<double, double>> intervals, std::vector<double> coefficients,
                        std::vector<PhaseTransition> transitions = {}, std::string name = "") -> std::shared_ptr<const SubstanceMethod>;

/// The method of what a record says.
auto cpPolynomialMethod(const MethodApplication& application, const std::string& name = "") -> std::shared_ptr<const SubstanceMethod>;

} // namespace ThermoFun::v2

#endif
