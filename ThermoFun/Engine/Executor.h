// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_EXECUTOR_H
#define THERMOFUN_ENGINE_EXECUTOR_H

#include <string>
#include <vector>

#include <ThermoFun/Engine/Binding.h>

namespace ThermoFun::v2 {

/// The properties of a record at T and P, with their derivatives, status and errors. SI.
struct SubstanceResult
{
    MethodResult values;
    PropertiesInfo info;
};

/// The state after one method, for `explain`.
struct ExplainStep
{
    std::string method;
    MethodResult values;
};

/// Run the bound methods of a record at T (K) and P (Pa). If steps is given, it gets the state after each method.
/// solvent: the state of the solvent, if a method of the record needs it.
auto executeSubstance(const BoundSubstance& bound, double T, double P, const SubstanceOptions& options = {}, std::vector<ExplainStep>* steps = nullptr, const SolventInputs* solvent = nullptr) -> SubstanceResult;

/// The derivatives of the properties by one parameter. SI per unit of the parameter in SI.
struct PropertyDerivatives
{
    double G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
};

/// The parameter of a pass: a parameter of one method, or the reference values with their seeds (one of them is 1).
struct ParameterSeed
{
    int method = -1;                 ///< its place in order; -1: none
    int parameter = -1;              ///< its place in the values of that method
    double G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
};

/// The derivatives of the properties of a record by one parameter, along the bound methods, each with the state that the method before gives.
auto substanceParameterDerivative(const BoundSubstance& bound, const ParameterSeed& seed, double T, double P, const SubstanceOptions& options = {}, const SolventInputs* solvent = nullptr) -> PropertyDerivatives;

} // namespace ThermoFun::v2

#endif
