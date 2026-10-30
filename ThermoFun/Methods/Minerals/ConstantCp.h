// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_CONSTANTCP_H
#define THERMOFUN_METHODS_MINERALS_CONSTANTCP_H

#include <cmath>

#include <ThermoFun/Methods/MethodBase.h>

namespace ThermoFun::v2 {

/// Integrates the constant reference Cp from the reference S, H and G. Without S or Cp in the record, the terms that
/// need it are left out.
class ConstantCp final : public SubstanceMethodBase<ConstantCp>
{
public:
    static constexpr const char* id = "ConstantCp";

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
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
};

} // namespace ThermoFun::v2

#endif
