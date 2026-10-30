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
auto executeSubstance(const BoundSubstance& bound, double T, double P, const SubstanceOptions& options = {}, std::vector<ExplainStep>* steps = nullptr) -> SubstanceResult;

} // namespace ThermoFun::v2

#endif
