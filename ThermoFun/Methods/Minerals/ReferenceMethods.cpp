// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReferenceMethods.h"

#include <cmath>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Methods/FunctionMethod.h>

namespace ThermoFun::model {

namespace {

constexpr double R = R_CONSTANT;  // J/(mol K), as the models of the library

template <typename Scalar>
auto constantProperties(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) -> void
{
    const auto& r = in.reference;
    state.G = r.G.value_or(Scalar(0.0));
    state.H = r.H.value_or(Scalar(0.0));
    state.S = r.S.value_or(Scalar(0.0));
    state.Cp = r.Cp.value_or(Scalar(0.0));
    state.V = r.V.value_or(Scalar(0.0));
}

template <typename Scalar>
auto constantCp(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) -> void
{
    using std::log;
    const Scalar& TK = in.T;
    const double TrK = in.reference.T;

    Scalar S = in.reference.S.value_or(Scalar(0.0));
    Scalar G = in.reference.G.value_or(Scalar(0.0));
    Scalar H = in.reference.H.value_or(Scalar(0.0));
    Scalar Cp = in.reference.Cp.value_or(Scalar(0.0));

    if (in.reference.S)
        G -= S * (TK - TrK);
    if (in.reference.Cp)
    {
        S += (Cp * log(TK / TrK));
        G -= (Cp * (TK * log(TK / TrK) - (TK - TrK)));
        H += (Cp * (TK - TrK));
    }

    state.Cp = Cp;
    state.G = G;
    state.H = H;
    state.S = S;
    state.V = Scalar(0.0);
}

auto constantCpInfo(PropertiesInfo& out, const PropertiesInfo& ref) -> void
{
    const bool hasS = ref.S.status != ValueInfo::Status::NotDefined;
    const bool hasCp = ref.Cp.status != ValueInfo::Status::NotDefined;
    out.Cp = ref.Cp;
    out.G = ref.G;
    if (hasS || hasCp)
        out.G.propagateFrom(ref.G);
    out.S = ref.S;
    out.H = ref.H;
    if (hasCp)
    {
        out.S.propagateFrom(ref.S);
        out.H.propagateFrom(ref.H);
    }
}

template <typename Scalar>
auto constantVolume(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) -> void
{
    if (!in.reference.V)
        return;
    const Scalar V = *in.reference.V;
    const Scalar dV = V * (in.P - in.reference.P);
    state.V = V;
    state.G += dV;
    state.H += dV;
}

auto constantVolumeInfo(PropertiesInfo& out, const PropertiesInfo& ref) -> void
{
    if (ref.V.status == ValueInfo::Status::NotDefined)
        return;
    out.V = ref.V;
    // the rule of the old engine: the error of V, in J/bar, adds to the errors of G and H without the factor dP
    ValueInfo volume = ref.V;
    volume.err *= 1e5;
    const ValueInfo g = out.G, h = out.H;
    out.G.propagateFrom(g, volume);
    out.H.propagateFrom(h, volume);
}

template <typename Scalar>
auto plainValue(const Scalar& x) -> double
{
    if constexpr (std::is_same_v<Scalar, double>)
        return x;
    else
        return x.val();
}

template <typename Scalar>
auto idealGas(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) -> void
{
    if (plainValue(in.P) <= 0.0)
        return;
    state.V = in.T / in.P * R;
    if (in.options.apply_pressure_correction)  // G(T, P) = G(T, Pr) + RT ln(P/Pr), and H does not depend on P
    {
        const Scalar lnP = log(in.P / in.reference.P);
        state.G += R * in.T * lnP;
        state.S -= R * lnP;
    }
}

auto idealGasInfo(PropertiesInfo& out, const InfoInputs& in) -> void
{
    if (in.P > 0.0)
        out.V = ValueInfo{ValueInfo::Status::Calculated, 0.0, ""};
}

} // namespace

auto constantPropertiesMethod() -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("ConstantProperties", MethodParameters<>{},
                      [](auto& state, const auto& in, const auto&) { constantProperties(state, in); },
                      [](PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs&, const auto&) { out = reference; });
}

auto constantCpMethod() -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("ConstantCp", MethodParameters<>{},
                      [](auto& state, const auto& in, const auto&) { constantCp(state, in); },
                      [](PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs&, const auto&) { constantCpInfo(out, reference); });
}

auto constantVolumeMethod() -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("ConstantVolume", MethodParameters<>{},
                      [](auto& state, const auto& in, const auto&) { constantVolume(state, in); },
                      [](PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs&, const auto&) { constantVolumeInfo(out, reference); });
}

auto idealGasMethod() -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("IdealGas", MethodParameters<NoData>{},
                      [](auto& state, const auto& in, const MethodParameters<NoData>&) { idealGas(state, in); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const MethodParameters<NoData>&) { idealGasInfo(out, in); });
}

} // namespace ThermoFun::model
