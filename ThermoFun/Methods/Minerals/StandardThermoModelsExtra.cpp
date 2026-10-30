// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "StandardThermoModelsExtra.h"
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>
#include <algorithm>

namespace ThermoFun {

//-------------------------------------------------------------------------------------------------------
// Extended UNIQUAC
//-------------------------------------------------------------------------------------------------------
auto thermoPropertiesExtendedUNIQUAC(real T, real Pbar, const MineralInputs& subst) -> ThermoPropertiesSubstanceAD
{
    const auto& cf = subst.coefficients;
    if (cf.size() < 5)
        errorModelParameters("coefficients a, b, c, alpha, beta (Theta optional)", "Extended UNIQUAC", __LINE__, __FILE__);

    const real a = cf[0], b = cf[1], c = cf[2], alpha = cf[3], beta = cf[4];
    const real Theta = (cf.size() > 5 && cf[5] != 0.0) ? cf[5] : real(200.0);

    const double Tr = subst.Tr;
    const double Pr = subst.Pr / bar_to_Pa; // bar
    const double Gr = subst.G;
    const double Hr = subst.H;
    const double Vr = subst.V;
    // the heat capacity has a pole at Theta: T and the reference T must be on the same side, above it
    if (T.val() <= Theta || Tr <= Theta)
        errorModelParameters("temperature and reference temperature above Theta", "Extended UNIQUAC", __LINE__, __FILE__);

    const real TmTr = T - Tr;
    const real dP = Pbar - Pr;

    ThermoPropertiesSubstanceAD out;
    out.heat_capacity_cp = a + b * T + c / (T - Theta);
    out.enthalpy = Hr + a * TmTr + 0.5 * b * (T * T - Tr * Tr) + c * log((T - Theta) / (Tr - Theta));
    out.gibbs_energy = Gr * T / Tr + Hr * (1 - T / Tr) - a * T * (log(T / Tr) + Tr / T - 1) - 0.5 * b * TmTr * TmTr
                     - c * T / Theta * ((T - Theta) / T * log((T - Theta) / (Tr - Theta)) - log(T / Tr));
    out.entropy = (out.enthalpy - out.gibbs_energy) / T;
    out.volume = Vr;

    if (alpha != 0.0 || beta != 0.0)
    {
        // G = G(T) + R T (alpha + beta dP) dP, so that S = -dG/dT, V = dG/dP; H is not changed
        out.gibbs_energy += R_CONSTANT * T * (alpha + beta * dP) * dP;
        out.entropy      -= R_CONSTANT * (alpha + beta * dP) * dP;
        out.volume        = R_CONSTANT * T * (alpha + 2 * beta * dP);
    }

    out.internal_energy  = out.enthalpy - Pbar * out.volume;
    out.helmholtz_energy = out.internal_energy - T * out.entropy;
    return out;
}

//-------------------------------------------------------------------------------------------------------
// NASA polynomials
//-------------------------------------------------------------------------------------------------------
auto thermoPropertiesNasaPolynomials(real T, real Pbar, const NasaInputs& subst, bool* outsideBounds) -> ThermoPropertiesSubstanceAD
{
    const auto& rows = subst.rows;
    const auto& intervals = subst.intervals;
    if (rows.empty() || rows.size() != intervals.size())
        errorModelParameters("at least one interval with nine coefficients", "NASA polynomials", __LINE__, __FILE__);

    // the interval of T, or the nearest one
    const double Tv = T.val();
    size_t k = 0;
    bool inside = false;
    double best = 1e300;
    for (size_t i = 0; i < intervals.size(); i++)
    {
        if (intervals[i].size() < 2 || rows[i].size() < 9)
            errorModelParameters("lower and upper T and nine coefficients per interval", "NASA polynomials", __LINE__, __FILE__);
        if (intervals[i][0] <= Tv && Tv <= intervals[i][1])
        {
            k = i;
            inside = true;
            break;
        }
        const double dist = std::min(std::fabs(Tv - intervals[i][0]), std::fabs(Tv - intervals[i][1]));
        if (dist < best)
        {
            best = dist;
            k = i;
        }
    }
    if (outsideBounds)
        *outsideBounds = !inside;

    const auto& p = rows[k];
    const double a1 = p[0], a2 = p[1], a3 = p[2], a4 = p[3], a5 = p[4], a6 = p[5], a7 = p[6], b1 = p[7], b2 = p[8];

    const real T2 = T * T, T3 = T * T2, T4 = T * T3, lnT = log(T);
    const double R = R_CONSTANT;

    ThermoPropertiesSubstanceAD out;
    out.heat_capacity_cp = (a1 / T2 + a2 / T + a3 + a4 * T + a5 * T2 + a6 * T3 + a7 * T4) * R;
    out.enthalpy = (-a1 / T2 + a2 * lnT / T + a3 + a4 * T / 2.0 + a5 * T2 / 3.0 + a6 * T3 / 4.0 + a7 * T4 / 5.0 + b1 / T) * R * T;
    out.entropy  = (-a1 / T2 * 0.5 - a2 / T + a3 * lnT + a4 * T + a5 * T2 / 2.0 + a6 * T3 / 3.0 + a7 * T4 / 4.0 + b2) * R;
    out.gibbs_energy = out.enthalpy - T * out.entropy;
    out.volume = real(subst.V);
    out.internal_energy  = out.enthalpy - Pbar * out.volume;
    out.helmholtz_energy = out.internal_energy - T * out.entropy;
    return out;
}

//-------------------------------------------------------------------------------------------------------
// Bilinear interpolation in T and P
//-------------------------------------------------------------------------------------------------------
namespace {

/// index of the interval of the sorted coordinates that contains x (clamped to the first and the last interval)
auto intervalIndex(const std::vector<double>& xs, double x) -> size_t
{
    if (xs.size() < 2)
        return 0;
    auto it = std::upper_bound(xs.begin(), xs.end(), x);
    size_t i = (it == xs.begin()) ? 0 : size_t(it - xs.begin()) - 1;
    return std::min(i, xs.size() - 2);
}

/// bilinear interpolation of table[i + j*nx] (i: temperature, j: pressure), x and y already clamped
auto bilinear(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& table,
              const real& x, const real& y) -> real
{
    const size_t nx = xs.size(), ny = ys.size();
    if (table.size() == 1)
        return real(table[0]);
    const size_t i1 = intervalIndex(xs, x.val()), j1 = intervalIndex(ys, y.val());
    const size_t i2 = nx == 1 ? 0 : i1 + 1, j2 = ny == 1 ? 0 : j1 + 1;
    auto z = [&](size_t i, size_t j) { return table[i + j * nx]; };
    const double x1 = xs[i1], x2 = xs[i2], y1 = ys[j1], y2 = ys[j2];
    if (x1 != x2 && y1 != y2)
        return (z(i1, j1) * (x2 - x) * (y2 - y) - z(i1, j2) * (x2 - x) * (y1 - y) - z(i2, j1) * (x1 - x) * (y2 - y)
                + z(i2, j2) * (x1 - x) * (y1 - y)) / ((x2 - x1) * (y2 - y1));
    if (x1 == x2 && y1 != y2)
        return (z(i1, j1) * (y2 - y) - z(i1, j2) * (y1 - y)) / (y2 - y1);
    if (x1 != x2 && y1 == y2)
        return (z(i1, j1) * (x2 - x) - z(i2, j1) * (x1 - x)) / (x2 - x1);
    return real(z(i1, j1));
}

}

auto thermoPropertiesTPTableInterpolation(real T, real Pbar, const TableInputs& tb, bool* outsideBounds) -> ThermoPropertiesSubstanceAD
{
    if (tb.temperatures.empty() || tb.pressures.empty())
        errorModelParameters("temperatures and pressures of the table", "T,P table interpolation", __LINE__, __FILE__);
    const size_t n = tb.temperatures.size() * tb.pressures.size();
    for (const auto& kv : tb.values)
        if (kv.second.size() != n)
            errorModelParameters("a table of temperatures x pressures", "T,P table interpolation", __LINE__, __FILE__);
    if (!tb.values.count("gibbs_energy"))
        errorModelParameters("the table gibbs_energy", "T,P table interpolation", __LINE__, __FILE__);

    // the table is in Pa and K; clamp to the table
    real Tc = T;
    real Pc = Pbar * bar_to_Pa;
    bool outside = false;
    if (Tc.val() < tb.temperatures.front()) { Tc = real(tb.temperatures.front()); outside = true; }
    if (Tc.val() > tb.temperatures.back())  { Tc = real(tb.temperatures.back()); outside = true; }
    if (Pc.val() < tb.pressures.front())    { Pc = real(tb.pressures.front()); outside = true; }
    if (Pc.val() > tb.pressures.back())     { Pc = real(tb.pressures.back()); outside = true; }
    if (outsideBounds)
        *outsideBounds = outside;

    auto get = [&](const char* key, real& value) -> bool {
        auto it = tb.values.find(key);
        if (it == tb.values.end() || it->second.empty())
            return false;
        value = bilinear(tb.temperatures, tb.pressures, it->second, Tc, Pc);
        return true;
    };

    ThermoPropertiesSubstanceAD out;
    real G(0.0), H(0.0), V(0.0), Cp(0.0);
    get("gibbs_energy", G);
    const bool hasH = get("enthalpy", H);
    const bool hasV = get("volume", V);
    const bool hasCp = get("heat_capacity_cp", Cp);
    if (!hasV)
        V = real(tb.V) / bar_to_Pa; // J/bar -> m3/mol

    // volume in J/bar: the table has m3/mol (volume is multiplied by the reference pressure of the table)
    const double Pref = tb.reference_pressure > 0.0 ? tb.reference_pressure : 1e5;
    out.volume = V * bar_to_Pa; // m3/mol -> J/bar
    out.gibbs_energy = G + V * (Pbar * bar_to_Pa - Pref); // J/mol
    if (hasH) out.enthalpy = H;
    if (hasH) out.entropy = (out.enthalpy - out.gibbs_energy) / Tc; // at the clamped temperature, as G and H
    if (hasCp) out.heat_capacity_cp = Cp;
    if (hasH)
    {
        out.internal_energy  = out.enthalpy - Pbar * out.volume;
        out.helmholtz_energy = out.internal_energy - Tc * out.entropy;
    }
    return out;
}

}
