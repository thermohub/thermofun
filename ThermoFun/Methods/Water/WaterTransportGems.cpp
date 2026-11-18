// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterTransportGems.h"

#include <cmath>
#include <type_traits>

namespace ThermoFun {

auto readWaterTransportGemsConstants(const std::map<std::string, model::Quantity>& values) -> WaterTransportGemsConstants
{
    WaterTransportGemsConstants out;
    auto one = [&](double& to, const std::string& name) { to = values.at(name).value; };
    // an array of one or two dimensions, row by row: name_1, name_2, ...
    auto list = [&](auto& to, const std::string& name) {
        using Row = std::remove_reference_t<decltype(to[0])>;
        if constexpr (std::is_array_v<Row>)
        {
            const auto columns = std::extent_v<Row>;
            for (std::size_t i = 0; i < std::extent_v<std::remove_reference_t<decltype(to)>> * columns; ++i)
                if (const auto it = values.find(name + "_" + std::to_string(i + 1)); it != values.end())
                    to[i / columns][i % columns] = it->second.value;
        }
        else
        {
            for (std::size_t i = 0; i < std::extent_v<std::remove_reference_t<decltype(to)>>; ++i)
                if (const auto it = values.find(name + "_" + std::to_string(i + 1)); it != values.end())
                    to[i] = it->second.value;
        }
    };
    list(out.visc_a, "visc_a");
    list(out.visc_b, "visc_b");
    list(out.cond_aL, "cond_aL");
    list(out.cond_au, "cond_au");
    list(out.cond_bL, "cond_bL");
    list(out.cond_bu, "cond_bu");
    one(out.visc_Pmax, "visc_Pmax");
    one(out.visc_P2, "visc_P2");
    one(out.visc_T2, "visc_T2");
    one(out.visc_P3, "visc_P3");
    one(out.visc_T3, "visc_T3");
    one(out.visc_Tmax, "visc_Tmax");
    one(out.cond_Pmax, "cond_Pmax");
    one(out.cond_P2, "cond_P2");
    one(out.cond_T2, "cond_T2");
    one(out.cond_P3, "cond_P3");
    one(out.cond_T3, "cond_T3");
    one(out.cond_Tmax, "cond_Tmax");
    one(out.surf_Ttripl, "surf_Ttripl");
    one(out.surf_Tcrit, "surf_Tcrit");
    one(out.surf_Tstar, "surf_Tstar");
    one(out.surf_Tcstar, "surf_Tcstar");
    one(out.surf_v, "surf_v");
    one(out.surf_B, "surf_B");
    one(out.surf_stref, "surf_stref");
    one(out.trans_Tstar, "trans_Tstar");
    one(out.trans_Dstar, "trans_Dstar");
    one(out.trans_Pstar, "trans_Pstar");
    one(out.trans_ustar, "trans_ustar");
    one(out.cond_C, "cond_C");
    return out;
}

// Watson et al. (1980); statements 3.1-2 and 4.1-5, tables 1, 6, 8 of Sengers and Kamgar-Parsi (1984).
// The double sum runs over all i and j (the C translation ran the inner loop for the first i only).
auto waterViscosityGems(const WaterTransportGemsConstants& k, real Tk, real Pbars, real Dkgm3, real betaPa) -> real
{
    const double TOL = 1.0e-2;
    const real TdegC = Tk - 273.15;
    if ((Pbars > k.visc_Pmax + TOL) || ((Pbars > k.visc_P2 + TOL) && (TdegC > k.visc_T2 + TOL)) ||
        ((Pbars > k.visc_P3 + TOL) && (TdegC > k.visc_T3 + TOL)) || (TdegC > k.visc_Tmax + TOL))
        return 0.0;
    const real T = Tk / k.trans_Tstar;
    const real D = Dkgm3 / k.trans_Dstar;
    real sum = 0.0;
    for (int i = 0; i <= 3; ++i)
        sum += k.visc_a[i] / pow(T, i);
    const real u0 = k.trans_ustar * sqrt(T) / sum;
    sum = 0.0;
    for (int i = 0; i <= 5; ++i)
        for (int j = 0; j <= 6; ++j)
            sum += k.visc_b[j][i] * pow(1.0 / T - 1.0, i) * pow(D - 1.0, j);
    const real u1 = exp(D * sum);
    real u2 = 1.0;
    if ((0.997 <= T) && (T <= 1.0082) && (0.755 <= D) && (D <= 1.2900))
    {
        const real xt = k.trans_Pstar / (k.trans_Dstar * k.trans_Dstar) * betaPa * Dkgm3 * Dkgm3;
        if (xt >= 22.0)
            u2 = 0.922 * pow(xt, 0.0263);
    }
    return u0 * u1 * u2;
}

// Sengers and Kamgar-Parsi (1984), with the critical enhancement L2. Both double sums run over all i and j.
auto waterThermalConductivityGems(const WaterTransportGemsConstants& k, real Tk, real Pbars, real Dkgm3, real alpha, real betaPa) -> real
{
    const double TOL = 1.0e-2;
    const real TdegC = Tk - 273.15;
    if ((Pbars > k.cond_Pmax + TOL) || ((Pbars > k.cond_P2 + TOL) && (TdegC > k.cond_T2 + TOL)) ||
        ((Pbars > k.cond_P3 + TOL) && (TdegC > k.cond_T3 + TOL)) || (TdegC > k.cond_Tmax + TOL))
        return 0.0;
    const real T = Tk / k.trans_Tstar;
    const real D = Dkgm3 / k.trans_Dstar;
    real sum = 0.0;
    for (int i = 0; i <= 3; ++i)
        sum += k.cond_aL[i] / pow(T, i);
    const real L0 = sqrt(T) / sum;
    sum = 0.0;
    for (int i = 0; i <= 4; ++i)
        for (int j = 0; j <= 5; ++j)
            sum += k.cond_bL[i][j] * pow(1.0 / T - 1.0, i) * pow(D - 1.0, j);
    const real L1 = exp(D * sum);
    sum = 0.0;
    for (int i = 0; i <= 3; ++i)
        sum += k.cond_au[i] / pow(T, i);
    const real u0 = k.trans_ustar * sqrt(T) / sum;
    sum = 0.0;
    for (int i = 0; i <= 5; ++i)
        for (int j = 0; j <= 4; ++j)
            sum += k.cond_bu[i][j] * pow(1.0 / T - 1.0, i) * pow(D - 1.0, j);
    const real u1 = exp(D * sum);
    const real xt = k.trans_Pstar / (k.trans_Dstar * k.trans_Dstar) * betaPa * Dkgm3 * Dkgm3;
    const real dPdT = k.trans_Tstar / k.trans_Pstar * alpha / betaPa;
    const real L2 = k.cond_C / (u0 * u1) * pow(T / D, 2.0) * pow(dPdT, 2.0) * pow(xt, 0.4678) * sqrt(D) *
                    exp(-18.66 * pow(T - 1.0, 2.0) - pow(D - 1.0, 4.0));
    return L0 * L1 + L2;
}

// IAPWS (1976), at the saturation temperature
auto waterSurfaceTensionGems(const WaterTransportGemsConstants& k, real Tsat) -> real
{
    if ((Tsat < k.surf_Ttripl) || (Tsat > k.surf_Tcrit))
        return 0.0;
    const real Tnorm = (Tsat >= k.surf_Tcrit - 1.0e-10) ? real(0.0) : (k.surf_Tcstar - Tsat / k.surf_Tstar) / k.surf_Tcstar;
    return k.surf_stref * pow(Tnorm, k.surf_v) * (1.0 + k.surf_B * Tnorm);
}

} // namespace ThermoFun
