// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_IDEALGAS_H
#define THERMOFUN_METHODS_MINERALS_IDEALGAS_H

#include <ThermoFun/Methods/MethodBase.h>

namespace ThermoFun::v2 {

/// Sets V = R T / P for a gas. With the pressure correction switched on, it also adds V (P - Pr) to G and H.
class IdealGas final : public SubstanceMethodBase<IdealGas>
{
public:
    static constexpr const char* id = "IdealGas";
    static constexpr double R = 8.31451;  // J/(mol K), as the models of the library

    /// isGas: the volume comes from the gas law only for a record of the class gas_fluid.
    explicit IdealGas(bool isGas) : isGas_(isGas) {}

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
    {
        if (idealVolume(valueOf(in.P)))
            state.V = in.T / in.P * R;
        if (in.options.apply_pressure_correction)
        {
            const Scalar term = state.V * (in.P - in.reference.P);
            state.G += term;
            state.H += term;
        }
    }

    auto info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void override
    {
        if (idealVolume(in.P))
            out.V = ValueInfo{ValueInfo::Status::Calculated, 0.0, ""};
        if (in.options.apply_pressure_correction)
        {
            const double dP = in.P - in.Pr;
            const ValueInfo g = out.G, h = out.H;
            out.G.propagateFrom(g, out.V);
            out.H.propagateFrom(h, out.V);
            out.G.setError({{1.0, g.err}, {dP, out.V.err}});
            out.H.setError({{1.0, h.err}, {dP, out.V.err}});
        }
    }

private:
    bool isGas_;
    auto idealVolume(double P) const -> bool { return isGas_ && P > 0.0; }

    template <typename Scalar>
    static auto valueOf(const Scalar& x) -> double
    {
        if constexpr (std::is_same_v<Scalar, double>)
            return x;
        else
            return x.val();
    }
};

} // namespace ThermoFun::v2

#endif
