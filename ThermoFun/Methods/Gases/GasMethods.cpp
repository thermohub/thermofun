// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "GasMethods.h"

#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Methods/Gases/GasCGF.h>
#include <ThermoFun/Methods/Gases/GasCORK.h>
#include <ThermoFun/Methods/Gases/GasPR78.h>
#include <ThermoFun/Methods/Gases/GasPRSV.h>
#include <ThermoFun/Methods/Gases/GasSRK.h>
#include <ThermoFun/Methods/Gases/GasSTP.h>
#include <ThermoFun/Methods/MethodHelpers.h>

namespace ThermoFun::v2 {

namespace {

template <typename Method>
auto modelName() -> const char*
{
    if constexpr (std::is_same_v<Method, PRSV86>)
        return "PRSV Peng-Robinson-Stryjek-Vera fluid model";
    else if constexpr (std::is_same_v<Method, ChurakovGottschalk03>)
        return "Churakov and Gottschalk fluid model";
    else if constexpr (std::is_same_v<Method, SoaveRedlichKwong72>)
        return "SRK Soave-Redlich-Kwong fluid model";
    else if constexpr (std::is_same_v<Method, SternerPitzer94>)
        return "STP Sterner-Pitzer fluid model";
    else if constexpr (std::is_same_v<Method, PengRobinson78>)
        return "PR78 Peng-Robinson fluid model";
    else
        return "CORK compensated-Redlich-Kwong fluid model";
}

template <typename Method>
auto formula(const real& T, const real& Pbar, const GasInputs& inputs, const ThermoPropertiesSubstanceAD& state, bool cancel) -> ThermoPropertiesSubstanceAD
{
    if constexpr (std::is_same_v<Method, PRSV86>)
        return thermoPropertiesGasPRSV(T, Pbar, inputs, state, cancel);
    else if constexpr (std::is_same_v<Method, ChurakovGottschalk03>)
        return thermoPropertiesGasCGF(T, Pbar, inputs, state, cancel);
    else if constexpr (std::is_same_v<Method, SoaveRedlichKwong72>)
        return thermoPropertiesGasSRK(T, Pbar, inputs, state, cancel);
    else if constexpr (std::is_same_v<Method, SternerPitzer94>)
        return thermoPropertiesGasSTP(T, Pbar, inputs, state, cancel);
    else if constexpr (std::is_same_v<Method, PengRobinson78>)
        return thermoPropertiesGasPR78(T, Pbar, inputs, state, cancel);
    else
        return thermoPropertiesGasCORK(T, Pbar, inputs, state, cancel);
}

} // namespace

template <typename Derived>
template <typename Scalar>
auto GasMethod<Derived>::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(Derived::id);
    else
    {
        GasInputs inputs;
        inputs.lower_T = record_.lower_T;
        inputs.formula = record_.formula;
        for (std::size_t i = 0; i < this->values_.size(); ++i)
        {
            // the formulas have Pc in bar, at place 1, for all methods but the coefficients of Churakov and Gottschalk
            const bool pressure = i == 1 && !std::is_same_v<Derived, ChurakovGottschalk03>;
            inputs.parameters.push_back(pressure ? this->parameter(i, in) / bar_to_Pa : this->parameter(i, in));
        }
        const Scalar p = in.P / bar_to_Pa;
        auto out = formula<Derived>(in.T, p, inputs, toModel(state), in.options.cancel_fugacity_coefficient);
        if (in.options.apply_pressure_correction)
        {
            const Scalar term = out.volume * (p - in.reference.P / bar_to_Pa);
            out.gibbs_energy += term;
            out.enthalpy += term;
        }
        setFromModel(state, out);
    }
}

template <typename Derived>
auto GasMethod<Derived>::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    // the Gibbs energy, enthalpy and entropy are incremented, the volume is set to a value
    const ValueInfo G = out.G, H = out.H, S = out.S;
    out.G.propagateFrom(G);
    out.H.propagateFrom(H);
    out.S.propagateFrom(S);
    out.V = ValueInfo{ValueInfo::Status::Assigned, 0.0, ""};
    if (in.options.apply_pressure_correction)
    {
        const double dP = (in.P - in.Pr) / bar_to_Pa;
        const ValueInfo g = out.G, h = out.H;
        out.G.propagateFrom(g, out.V);
        out.H.propagateFrom(h, out.V);
        out.G.setError({{1.0, g.err}, {dP, out.V.err}});
        out.H.setError({{1.0, h.err}, {dP, out.V.err}});
    }
    const auto message = boundsMessage(modelName<Derived>(), record_.applicability, in.T, in.P);
    if (!message.empty())
        setMessage(out, message);
}

#define THERMOFUN_INSTANTIATE_GAS(M)                                                                                   \
    template class GasMethod<M>;                                                                                       \
    template auto GasMethod<M>::compute(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;        \
    template auto GasMethod<M>::compute(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;

THERMOFUN_INSTANTIATE_GAS(PRSV86)
THERMOFUN_INSTANTIATE_GAS(ChurakovGottschalk03)
THERMOFUN_INSTANTIATE_GAS(SoaveRedlichKwong72)
THERMOFUN_INSTANTIATE_GAS(SternerPitzer94)
THERMOFUN_INSTANTIATE_GAS(PengRobinson78)
THERMOFUN_INSTANTIATE_GAS(CorkHP91)

#undef THERMOFUN_INSTANTIATE_GAS

} // namespace ThermoFun::v2
