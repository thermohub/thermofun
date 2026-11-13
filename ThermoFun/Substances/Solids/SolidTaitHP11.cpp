// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "SolidTaitHP11.h"
#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesMinTaitHP11(real TK, real Pbar, Substance subst, ThermoPropertiesSubstanceAD tps, bool* variableVolume) -> ThermoPropertiesSubstanceAD
{
    const auto Pr = subst.referenceP() / bar_to_Pa; // bar
    const auto Tr = subst.referenceT();              // K
    const auto ref = subst.thermoReferenceProperties();
    const auto Vr = Reaktoro_::constant(ref.volume); // J/bar
    const auto Sr = Reaktoro_::constant(ref.entropy); // J/(mol K)
    const auto alpha0 = subst.thermoParameters().isobaric_expansivity * 1e-05; // 1/K
    const auto k0 = subst.thermoParameters().isothermal_compresibility * 1000.0; // bulk modulus, bar
    const auto& c = subst.thermoParameters().volume_Tait_coeff;

    if (variableVolume)
        *variableVolume = false;

    if (k0 == 0.0 || fabs(Vr.val()) < 1e-09)
    {
        // no bulk modulus: constant volume
        tps.volume           = Vr;
        tps.gibbs_energy    += Vr * (Pbar - Pr);
        tps.enthalpy        += Vr * (Pbar - Pr);
        tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
        tps.helmholtz_energy = tps.internal_energy - TK*tps.entropy;
        return tps;
    }

    if (c.size() < 3)
        errorModelParameters("three coefficients (kappa0', kappa0'', number of atoms)", "Tait Holland and Powell 2011", __LINE__, __FILE__);

    const double kp  = c[0];          // kappa0'
    const double kpp = c[1] / 1000.0; // kappa0'', 1/bar
    const double numatoms = c[2];

    // eq. (3) of Holland and Powell (2011)
    const double ka = (1 + kp) / (1 + kp + k0 * kpp);
    const double kb = kp / k0 - kpp / (1 + kp);
    const double kc = (1 + kp + k0 * kpp) / (kp * (1 + kp) - k0 * kpp);

    // Einstein temperature (Holland and Powell 2011, p. 346)
    const double theta = 10636.0 / (Sr.val() / numatoms + 6.44);

    const real u  = theta / TK;
    const double u0 = theta / Tr;
    const real eu = exp(u);
    const double eu0 = std::exp(u0);
    const real E  = (u / (eu - 1)) * (u / (eu - 1)) * eu;
    const double E0 = (u0 / (eu0 - 1)) * (u0 / (eu0 - 1)) * eu0;

    // thermal pressure and its first two temperature derivatives
    const real Pth   = alpha0 * k0 * (theta / E0) * (1 / (eu - 1) - 1 / (eu0 - 1));
    const real dPth  = alpha0 * k0 * E / E0;
    const real dEdT  = -E * u / TK * (2 / u + 1 - 2 * eu / (eu - 1));
    const real d2Pth = alpha0 * k0 * dEdT / E0;

    const real aux1 = 1 - kb * Pth;

    // I(P) = integral of V dP, J(P) = dI/dT, K(P) = dJ/dT at the pressure P
    auto integrals = [&](const real& P, real& I, real& J, real& K) {
        const real x = 1 + kb * (P - Pth);
        I = Vr * ((1 - ka) * P + ka * (pow(aux1, 1 - kc) - pow(x, 1 - kc)) / (kb * (kc - 1)));
        J = Vr * ka * dPth * (pow(aux1, -kc) - pow(x, -kc));
        K = Vr * ka * (d2Pth * (pow(aux1, -kc) - pow(x, -kc)) + kc * kb * dPth * dPth * (pow(aux1, -kc - 1) - pow(x, -kc - 1)));
    };

    real I, J, K, I0, J0, K0;
    integrals(Pbar, I, J, K);
    integrals(real(Pr), I0, J0, K0);

    const real dg  = I - I0;
    const real ds  = -(J - J0);
    const real dh  = dg + TK * ds;
    const real dcp = -TK * (K - K0);
    const real x   = 1 + kb * (Pbar - Pth);
    const real V   = Vr * (1 - ka * (1 - pow(x, -kc)));

    if (variableVolume)
        *variableVolume = true;

    tps.gibbs_energy     += dg;
    tps.entropy          += ds;
    tps.enthalpy         += dh;
    tps.heat_capacity_cp += dcp;
    tps.volume            = V;
    tps.internal_energy   = tps.enthalpy - Pbar * tps.volume;
    tps.helmholtz_energy  = tps.internal_energy - TK * tps.entropy;

    return tps;
}

}
