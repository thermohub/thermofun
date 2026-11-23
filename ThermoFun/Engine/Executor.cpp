// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <algorithm>
#include "Executor.h"

namespace ThermoFun::model {

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

/// The result of a method in the canonical convention
auto convert(SubstanceState<TPReal>& state, const ConventionShift& shift, const SubstanceInputs<TPReal>& in) -> void
{
    state.G += shift.G0 + shift.G_per_K * in.T;
    state.S -= shift.G_per_K;
    if (shift.needsSolvent())
    {
        const auto& s = in.solvent->properties;
        const auto m = molarityTerms(in.T, s.density, s.densityT, s.densityP, s.densityTT, in.options.R);
        state.G += shift.molarity * m.G;
        state.H += shift.molarity * m.H;
        state.S += shift.molarity * m.S;
        state.Cp += shift.molarity * m.Cp;
        state.V += shift.molarity * m.V;
    }
}

auto combine(const SubstanceState<TPReal>& wrtT, const SubstanceState<TPReal>& wrtP) -> MethodResult
{
    auto one = [](const TPReal& a, const TPReal& b) { return PropertyValue{a[0], a[1], b[1]}; };
    MethodResult out;
    out.G = one(wrtT.G, wrtP.G);
    out.H = one(wrtT.H, wrtP.H);
    out.S = one(wrtT.S, wrtP.S);
    out.Cp = one(wrtT.Cp, wrtP.Cp);
    out.V = one(wrtT.V, wrtP.V);
    out.extrapolated = wrtT.outside_range;
    return out;
}

} // namespace

/// The values of a state (the pass with respect to T: values only)
static auto valuesOf(const SubstanceState<TPReal>& s) -> RouteValues
{
    return {static_cast<double>(s.G[0]), static_cast<double>(s.H[0]), static_cast<double>(s.S[0]), static_cast<double>(s.Cp[0]), static_cast<double>(s.V[0])};
}

auto executeSubstance(const BoundSubstance& bound, double T, double P, const SubstanceOptions& options, std::vector<MethodStep>* steps, const SolventInputs* solvent) -> SubstanceResult
{
    const auto reference = toReal(bound.reference);
    TPReal Tt(T), Pp(P);
    Tt[1] = 1.0;
    Pp[1] = 1.0;
    SubstanceInputs<TPReal> wrtT{Tt, TPReal(P), reference};
    SubstanceInputs<TPReal> wrtP{TPReal(T), Pp, reference};
    wrtT.options = options;
    wrtP.options = options;

    SolventState<TPReal> solventT, solventP, solventRef;
    if (solvent)
    {
        solventT = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::T);
        solventP = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::P);
        solventRef = solventAtReference(*solvent, T, P);
        wrtT.solvent = &solventT;
        wrtP.solvent = &solventP;
        wrtT.solvent_at_reference = wrtP.solvent_at_reference = &solventRef;
    }

    SubstanceState<TPReal> a, b;
    SubstanceResult result;
    if (steps)
        steps->reserve(steps->size() + bound.order.size());
    for (std::size_t i = 0; i < bound.order.size(); ++i)
    {
        const auto& method = bound.order[i];
        const RouteValues before = steps ? valuesOf(a) : RouteValues{};
        MethodStep step;
        // the formula writes its terms and variables as it calculates them (no other evaluation of the model)
        wrtT.outputs = steps ? &step.outputs : nullptr;
        method->apply(a, wrtT);
        wrtT.outputs = nullptr;
        method->apply(b, wrtP);
        if (steps)
            step.contribution = valuesOf(a) - before;
        if (i < bound.to_canonical.size() && !bound.to_canonical[i].identity())
        {
            const RouteValues after = steps ? valuesOf(a) : RouteValues{};
            convert(a, bound.to_canonical[i], wrtT);
            convert(b, bound.to_canonical[i], wrtP);
            step.converted = true;
            if (steps)
                step.conversion = valuesOf(a) - after;
        }
        method->info(result.info, bound.reference_info, InfoInputs{T, P, bound.reference.T, bound.reference.P, bound.reference.V.value_or(0.0) * 1e5, a.outside_range, options, solvent});
        if (steps)
            steps->push_back(std::move(step));
    }
    result.values = combine(a, b);
    return result;
}

auto executeOn(const MethodResult& initial, const BoundSubstance& bound, double T, double P, const SubstanceOptions& options, const SolventInputs* solvent, std::vector<MethodStep>* steps) -> MethodResult
{
    auto start = [&](bool wrtT) {
        auto one = [&](const PropertyValue& v) {
            TPReal x(v.val);
            x[1] = wrtT ? v.ddt : v.ddp;
            return x;
        };
        SubstanceState<TPReal> s;
        s.G = one(initial.G);
        s.H = one(initial.H);
        s.S = one(initial.S);
        s.Cp = one(initial.Cp);
        s.V = one(initial.V);
        return s;
    };
    const auto reference = toReal(bound.reference);
    TPReal Tt(T), Pp(P);
    Tt[1] = 1.0;
    Pp[1] = 1.0;
    SubstanceInputs<TPReal> wrtT{Tt, TPReal(P), reference};
    SubstanceInputs<TPReal> wrtP{TPReal(T), Pp, reference};
    wrtT.options = options;
    wrtP.options = options;
    SolventState<TPReal> solventT, solventP, solventRef;
    if (solvent)
    {
        solventT = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::T);
        solventP = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::P);
        solventRef = solventAtReference(*solvent, T, P);
        wrtT.solvent = &solventT;
        wrtP.solvent = &solventP;
        wrtT.solvent_at_reference = wrtP.solvent_at_reference = &solventRef;
    }
    auto a = start(true), b = start(false);
    for (const auto& method : bound.order)
    {
        const RouteValues before = steps ? valuesOf(a) : RouteValues{};
        method->apply(a, wrtT);
        method->apply(b, wrtP);
        if (steps)
        {
            MethodStep step;
            step.contribution = valuesOf(a) - before;
            steps->push_back(std::move(step));
        }
    }
    return combine(a, b);
}

auto substanceParameterDerivative(const BoundSubstance& bound, const ParameterSeed& seed, double T, double P, const SubstanceOptions& options, const SolventInputs* solvent,
                                  const std::pair<MethodResult, PropertyDerivatives>* initial) -> PropertyDerivatives
{
    auto reference = toReal(bound.reference);
    auto carry = [](std::optional<TPReal>& value, double s) {
        if (value && s != 0.0)
            (*value)[1] = s;
    };
    carry(reference.G, seed.G);
    carry(reference.H, seed.H);
    carry(reference.S, seed.S);
    carry(reference.Cp, seed.Cp);
    carry(reference.V, seed.V);

    SubstanceInputs<TPReal> in{TPReal(T), TPReal(P), reference};
    in.options = options;
    SolventState<TPReal> atPoint, atReference;
    if (solvent)
    {
        atPoint = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::None);
        atReference = solventAtReference(*solvent, T, P);
        in.solvent = &atPoint;
        in.solvent_at_reference = &atReference;
    }

    SubstanceState<TPReal> state;
    if (initial)
    {
        auto one = [](double value, double derivative) {
            TPReal x(value);
            x[1] = derivative;
            return x;
        };
        const auto& [values, d] = *initial;
        state.G = one(values.G.val, d.G);
        state.H = one(values.H.val, d.H);
        state.S = one(values.S.val, d.S);
        state.Cp = one(values.Cp.val, d.Cp);
        state.V = one(values.V.val, d.V);
    }
    for (std::size_t i = 0; i < bound.order.size(); ++i)
    {
        in.seeded_parameter = static_cast<int>(i) == seed.method ? seed.parameter : -1;
        bound.order[i]->apply(state, in);
        if (i < bound.to_canonical.size() && !bound.to_canonical[i].identity())
            convert(state, bound.to_canonical[i], in);
    }
    return {state.G[1], state.H[1], state.S[1], state.Cp[1], state.V[1]};
}

} // namespace ThermoFun::model
