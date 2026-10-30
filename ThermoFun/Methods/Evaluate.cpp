// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Evaluate.h"

namespace ThermoFun::v2 {

namespace {

auto toReal(const ReferenceValues& r) -> ReferenceState<TPReal>
{
    ReferenceState<TPReal> out;
    out.T = r.T;
    out.P = r.P;
    auto copy = [](const std::optional<double>& x) -> std::optional<TPReal> {
        if (x)
            return TPReal(*x);
        return std::nullopt;
    };
    out.G = copy(r.G);
    out.H = copy(r.H);
    out.S = copy(r.S);
    out.Cp = copy(r.Cp);
    out.V = copy(r.V);
    return out;
}

enum class Seed { T, P, Parameter };

auto pass(const SubstanceMethod& method, double T, double P, const ReferenceState<TPReal>& reference, Seed seed, int parameter) -> SubstanceState<TPReal>
{
    TPReal t(T), p(P);
#ifndef THERMOFUN_NO_AUTODIFF
    if (seed == Seed::T)
        t[1] = 1.0;
    if (seed == Seed::P)
        p[1] = 1.0;
#endif
    SubstanceInputs<TPReal> in{t, p, reference};
    if (seed == Seed::Parameter)
        in.seeded_parameter = parameter;
    SubstanceState<TPReal> state;
    method.apply(state, in);
    return state;
}

} // namespace

auto evaluate(const SubstanceMethod& method, double T, double P, const ReferenceValues& reference) -> MethodResult
{
    const auto r = toReal(reference);
    const auto wrtT = pass(method, T, P, r, Seed::T, -1);
    const auto wrtP = pass(method, T, P, r, Seed::P, -1);
    auto combine = [](const TPReal& a, const TPReal& b) { return PropertyValue{a[0], a[1], b[1]}; };
    MethodResult out;
    out.G = combine(wrtT.G, wrtP.G);
    out.H = combine(wrtT.H, wrtP.H);
    out.S = combine(wrtT.S, wrtP.S);
    out.Cp = combine(wrtT.Cp, wrtP.Cp);
    out.V = combine(wrtT.V, wrtP.V);
    out.outside_range = wrtT.outside_range;
    return out;
}

auto evaluateParameterDerivative(const SubstanceMethod& method, std::size_t index, double T, double P, const ReferenceValues& reference) -> MethodResult
{
    const auto s = pass(method, T, P, toReal(reference), Seed::Parameter, static_cast<int>(index));
    auto one = [](const TPReal& a) { return PropertyValue{a[0], a[1], 0.0}; };  // ddt holds the derivative of the pass
    MethodResult out;
    out.G = one(s.G);
    out.H = one(s.H);
    out.S = one(s.S);
    out.Cp = one(s.Cp);
    out.V = one(s.V);
    return out;
}

} // namespace ThermoFun::v2
