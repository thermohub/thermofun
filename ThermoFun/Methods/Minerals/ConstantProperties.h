// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_CONSTANTPROPERTIES_H
#define THERMOFUN_METHODS_MINERALS_CONSTANTPROPERTIES_H

#include <ThermoFun/Methods/MethodBase.h>

namespace ThermoFun::v2 {

/// The reference properties at every T and P. A property that the record does not give is 0.
class ConstantProperties final : public SubstanceMethodBase<ConstantProperties>
{
public:
    static constexpr const char* id = "ConstantProperties";

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
    {
        const auto& r = in.reference;
        state.G = r.G.value_or(Scalar(0.0));
        state.H = r.H.value_or(Scalar(0.0));
        state.S = r.S.value_or(Scalar(0.0));
        state.Cp = r.Cp.value_or(Scalar(0.0));
        state.V = r.V.value_or(Scalar(0.0));
    }
};

} // namespace ThermoFun::v2

#endif
