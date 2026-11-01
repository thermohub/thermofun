// SPDX-License-Identifier: LGPL-2.1-or-later
// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "WaterUtils.hpp"

// Reaktoro includes (modified DM 11.05.2016)
#include <ThermoFun/Common/ThermoProperties.h>
#include <Common/Exception.h>
#include <Methods/Water/Reaktoro/WaterHelmholtzStateHGK.hpp>
#include <Methods/Water/Reaktoro/WaterHelmholtzStateWagnerPruss.hpp>
#include <Methods/Water/Reaktoro/WaterHelmholtzState.hpp>

#include <vector>

namespace ThermoFun {

template<typename HelmholtsModel>
auto waterDensity(real T, real P, int solvent_state, const WaterSaturationConstants& s, const HelmholtsModel& model) -> real
{
    // Auxiliary constants for the Newton's iterations
    const int max_iters = 100;
    const double tolerance = 1.0e-08;

    // Determine the physical state of water, where: 0-liquid, 1-vapour, 2-supercritical
    int state;

    if(T.val() <= s.Tcr)
    {
        auto p = waterSaturatedPressureWagnerPruss(T, s);
        state = (P < p) ? 1 : 0; // changed from P <= DM 29.05.2016
    }
    else
        state = 2;

    // Determine an adequate initial guess for (dimensionless) density based on the physical state of water
    real D;

    if ((solvent_state >=0) && (T.val() <= s.Tcr))
    state = solvent_state;

    switch(state)
    {
    case 0: D = waterSaturatedLiquidDensityWagnerPruss(T, s); break;
    case 1: D = waterSaturatedVapourDensityWagnerPruss(T, s); break;
    case 2: D = s.Dcr * 0.99; break; // some derivatives of the Helmholtz free energy do not exist in the critical point
    }

    // Apply the Newton's method to the pressure-density equation
    for(int i = 1; i <= max_iters; ++i)
    {
        WaterHelmholtzState h = model(T, D);

        const auto f  = (D*D*h.helmholtzD - P)/s.Pcr;
        const auto df = (2*D*h.helmholtzD + D*D*h.helmholtzDD)/s.Pcr;

        D = (D.val() > f.val()/df.val()) ? D - f/df : P/(D*h.helmholtzD);

        if(std::abs(f.val()) < tolerance)
            return D;
    }

    ThermoFun::Exception exception;
    exception.error << "Unable to calculate the density of water.";
    exception.reason << "The calculations did not converge at temperature "
        << T.val() << " K and pressure " << P.val() << "Pa.";
    exception.line = __LINE__;
    RaiseError(exception);

    return {};
}

auto waterDensityHGK(real T, real P, int state, const HGKConstants& constants) -> real
{
    return waterDensity(T, P, state, constants.saturation, [&constants](real t, real d) { return waterHelmholtzStateHGK(t, d, constants); });
}

auto waterDensityWagnerPruss(real T, real P, int state, const WagnerPrussConstants& constants) -> real
{
    return waterDensity(T, P, state, constants.saturation, [&constants](real t, real d) { return waterHelmholtzStateWagnerPruss(t, d, constants); });
}

auto waterSaturatedPressureWagnerPruss(real T, const WaterSaturationConstants& s) -> real
{
    const double a1 = s.a[0];
    const double a2 = s.a[1];
    const double a3 = s.a[2];
    const double a4 = s.a[3];
    const double a5 = s.a[4];
    const double a6 = s.a[5];

    const double Tcr = s.Tcr;
    const double Pcr = s.Pcr;

    const auto t   = 1 - T/Tcr;
    const auto t15 = pow(t, 1.5);
    const auto t30 = t15 * t15;
    const auto t35 = t15 * t * t;
    const auto t40 = t30 * t;
    const auto t75 = t35 * t40;

    return Pcr * exp(Tcr/T * (a1*t + a2*t15 + a3*t30 + a4*t35 + a5*t40 + a6*t75));
}

auto waterSaturatedLiquidDensityWagnerPruss(real T, const WaterSaturationConstants& s) -> real
{
    const double b1 = s.b[0];
    const double b2 = s.b[1];
    const double b3 = s.b[2];
    const double b4 = s.b[3];
    const double b5 = s.b[4];
    const double b6 = s.b[5];

    const double Tcr = s.Tcr;
    const double Dcr = s.Dcr;

    const auto t     = 1 - T/Tcr;
    const auto t13   = pow(t, 1./3);
    const auto t23   = t13 * t13;
    const auto t53   = t13 * t23 * t23;
    const auto t163  = t13 * t53 * t53 * t53;
    const auto t433  = t163 * t163 * t53 * t * t;
    const auto t1103 = t433 * t433 * t163 * t53 * t;

    return Dcr * (1 + b1*t13 + b2*t23 + b3*t53 + b4*t163 + b5*t433 + b6*t1103);
}

auto waterSaturatedVapourDensityWagnerPruss(real T, const WaterSaturationConstants& s) -> real
{
    const double c1 = s.c[0];
    const double c2 = s.c[1];
    const double c3 = s.c[2];
    const double c4 = s.c[3];
    const double c5 = s.c[4];
    const double c6 = s.c[5];

    const double Tcr = s.Tcr;
    const double Dcr = s.Dcr;

    const auto t    = 1 - T/Tcr;
    const auto t16  = pow(t, 1./6);
    const auto t26  = t16 * t16;
    const auto t46  = t26 * t26;
    const auto t86  = t46 * t46;
    const auto t186 = t86 * t86 * t26;
    const auto t376 = t186 * t186 * t16;
    const auto t716 = t376 * t186 * t86 * t86;

    return Dcr * exp(Tcr/T * (c1*t26 + c2*t46 + c3*t86 + c4*t186 + c5*t376 + c6*t716));
}

} // namespace Reaktoro

