// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_EXECUTOR_H
#define THERMOFUN_ENGINE_EXECUTOR_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Engine/Binding.h>

namespace ThermoFun::model {

/// The properties of a record at T and P, with their derivatives, status and errors. SI.
struct SubstanceResult
{
    MethodResult values;
    PropertiesInfo info;
};

/// One method of a record on the route of a result (decision 0030): what it changed, and the change of the conversion of its
/// convention to the canonical one. SI.
struct MethodStep
{
    RouteValues contribution;     ///< the state after the method minus the state before it, in the convention of the method
    RouteValues conversion;       ///< the change of the conversion to the canonical convention after the method; zero if none
    bool converted = false;       ///< the method has a conversion to the canonical convention
    MethodOutputs outputs;        ///< terms and variables
};

/// Run the bound methods of a record at T (K) and P (Pa). If steps is given, it gets one step for each method, with its terms and
/// variables (written by the formula as it calculates). solvent: the state of the solvent, if a method of the record needs it.
auto executeSubstance(const BoundSubstance& bound, double T, double P, const SubstanceOptions& options = {}, std::vector<MethodStep>* steps = nullptr,
                      const SolventInputs* solvent = nullptr) -> SubstanceResult;

/// The methods of a record run on properties that come from elsewhere, from a defining reaction: initial, with its derivatives. SI.
/// steps: as for executeSubstance (no conversions, no terms).
auto executeOn(const MethodResult& initial, const BoundSubstance& bound, double T, double P, const SubstanceOptions& options = {}, const SolventInputs* solvent = nullptr,
               std::vector<MethodStep>* steps = nullptr) -> MethodResult;

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
/// initial: the values and derivatives of what the methods work on, if they do not start from nothing.
auto substanceParameterDerivative(const BoundSubstance& bound, const ParameterSeed& seed, double T, double P, const SubstanceOptions& options = {}, const SolventInputs* solvent = nullptr,
                                  const std::pair<MethodResult, PropertyDerivatives>* initial = nullptr) -> PropertyDerivatives;

} // namespace ThermoFun::model

#endif
