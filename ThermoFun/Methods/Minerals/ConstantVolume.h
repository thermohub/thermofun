// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_CONSTANTVOLUME_H
#define THERMOFUN_METHODS_MINERALS_CONSTANTVOLUME_H

#include <ThermoFun/Methods/MethodBase.h>

namespace ThermoFun::v2 {

/// Adds V dP to G and H, with the reference volume. Without a reference volume it does nothing.
class ConstantVolume final : public SubstanceMethodBase<ConstantVolume>
{
public:
    static constexpr const char* id = "ConstantVolume";

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
    {
        if (!in.reference.V)
            return;
        const Scalar V = *in.reference.V;
        const Scalar dV = V * (in.P - in.reference.P);
        state.V = V;
        state.G += dV;
        state.H += dV;
    }

    auto info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs&) const -> void override
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
};

} // namespace ThermoFun::v2

#endif
