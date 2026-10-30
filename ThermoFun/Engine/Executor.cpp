// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Executor.h"

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

auto combine(const SubstanceState<TPReal>& wrtT, const SubstanceState<TPReal>& wrtP) -> MethodResult
{
    auto one = [](const TPReal& a, const TPReal& b) { return PropertyValue{a[0], a[1], b[1]}; };
    MethodResult out;
    out.G = one(wrtT.G, wrtP.G);
    out.H = one(wrtT.H, wrtP.H);
    out.S = one(wrtT.S, wrtP.S);
    out.Cp = one(wrtT.Cp, wrtP.Cp);
    out.V = one(wrtT.V, wrtP.V);
    out.outside_range = wrtT.outside_range;
    return out;
}

} // namespace

auto executeSubstance(const BoundSubstance& bound, double T, double P, const SubstanceOptions& options, std::vector<ExplainStep>* steps, const SolventInputs* solvent) -> SubstanceResult
{
    const auto reference = toReal(bound.reference);
    TPReal Tt(T), Pp(P);
#ifndef THERMOFUN_NO_AUTODIFF
    Tt[1] = 1.0;
    Pp[1] = 1.0;
#endif
    SubstanceInputs<TPReal> wrtT{Tt, TPReal(P), reference};
    SubstanceInputs<TPReal> wrtP{TPReal(T), Pp, reference};
    wrtT.options = options;
    wrtP.options = options;

    SolventState<TPReal> solventT, solventP, solventRef;
    if (solvent)
    {
#ifndef THERMOFUN_NO_AUTODIFF
        solventT = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::T);
        solventP = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::P);
#else
        solventT = solventP = solventOfPass(*solvent, T, P, Reaktoro_::Wrt::None);
#endif
        solventRef = solventAtReference(*solvent, T, P);
        wrtT.solvent = &solventT;
        wrtP.solvent = &solventP;
        wrtT.solvent_at_reference = wrtP.solvent_at_reference = &solventRef;
    }

    SubstanceState<TPReal> a, b;
    SubstanceResult result;
    for (const auto& method : bound.order)
    {
        method->apply(a, wrtT);
        method->apply(b, wrtP);
        method->info(result.info, bound.reference_info, InfoInputs{T, P, bound.reference.T, bound.reference.P, a.outside_range, options, solvent});
        if (steps)
            steps->push_back({method->spec().id, combine(a, b)});
    }
    result.values = combine(a, b);
    return result;
}

} // namespace ThermoFun::v2
