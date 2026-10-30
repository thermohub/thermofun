// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_EVALUATE_H
#define THERMOFUN_METHODS_EVALUATE_H

#include <ThermoFun/Methods/Method.h>

namespace ThermoFun::v2 {

/// A property with its derivatives with respect to T and P.
struct PropertyValue
{
    double val = 0.0, ddt = 0.0, ddp = 0.0;
};

/// The properties of one method applied alone, with their derivatives.
struct MethodResult
{
    PropertyValue G, H, S, Cp, V;
    bool outside_range = false;
};

/// The reference values of a substance in plain numbers (SI).
using ReferenceValues = ReferenceState<double>;

/// The state of the solvent in the pass seeded by wrt.
auto solventOfPass(const SolventInputs& in, double T, double P, Reaktoro_::Wrt wrt) -> SolventState<TPReal>;

/// The state of the solvent at its reference, as constants of a pass.
auto solventAtReference(const SolventInputs& in, double T, double P) -> SolventState<TPReal>;

/// Apply one method alone at T (K) and P (Pa), from an empty state, with the passes of the derivatives.
/// solvent: the state of the solvent, for a method that needs it.
auto evaluate(const SubstanceMethod& method, double T, double P, const ReferenceValues& reference, const SolventInputs* solvent = nullptr) -> MethodResult;

/// The derivatives of the properties with respect to parameter `index` of the method, one pass. SI.
auto evaluateParameterDerivative(const SubstanceMethod& method, std::size_t index, double T, double P, const ReferenceValues& reference, const SolventInputs* solvent = nullptr) -> MethodResult;

} // namespace ThermoFun::v2

#endif
