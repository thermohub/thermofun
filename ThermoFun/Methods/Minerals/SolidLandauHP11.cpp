// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "SolidLandauHP11.h"
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

auto thermoPropertiesHP11Landau(real TK, real Pbar, const MineralInputs& subst, ThermoPropertiesSubstanceAD tps, bool* subcritical) -> ThermoPropertiesSubstanceAD
{
    const auto& transProp = subst.coefficients;
    if (transProp.size() < 3)
    {
        thfun_logger->error(" transition properties size {} ", transProp.size());
        errorModelParameters("transition properties", "HP11 Landau", __LINE__, __FILE__);
    }
    const real Tc0 = transProp[0] + C_to_K;         // given in centigrade, as for the 1998 model
    const real Smax = transProp[1] > 0.0 ? transProp[1] : real(1.0e-20);   // J/(mol K)
    const real Vmax = transProp[2];                 // J/bar
    const double Tr = subst.Tr;
    const double Pr = subst.Pr / bar_to_Pa;   // bar

    // the order parameter at the reference state: Q0^2
    const real Q02 = Tr < Tc0 ? real(sqrt((Tc0 - Tr) / Tc0)) : real(0.0);
    const real Q06 = Q02 * Q02 * Q02;

    const real Tc = Tc0 + Vmax * (Pbar - Pr) / Smax;
    const bool below = TK.val() < Tc.val();
    real Q2 = 0.0;
    if (below)
        Q2 = sqrt((Tc - TK) / Tc0);
    const real Q6 = Q2 * Q2 * Q2;

    const real G = Tc0 * Smax * (Q02 - Q06 / 3.0) - Smax * (Tc * Q2 - Tc0 * Q6 / 3.0) - TK * Smax * (Q02 - Q2) + (Pbar - Pr) * Vmax * Q02;
    const real S = Smax * (Q02 - Q2);
    const real V = Vmax * (Q02 - Q2);
    const real H = G + TK * S;

    tps.gibbs_energy += G;
    tps.entropy += S;
    tps.enthalpy += H;
    tps.volume += V;
    if (below)
        tps.heat_capacity_cp += TK * Smax / (2.0 * Tc0 * Q2);
    if (subcritical)
        *subcritical = below;

    tps.internal_energy = tps.enthalpy - Pbar * tps.volume;
    tps.helmholtz_energy = tps.internal_energy - TK * tps.entropy;
    return tps;
}

}
